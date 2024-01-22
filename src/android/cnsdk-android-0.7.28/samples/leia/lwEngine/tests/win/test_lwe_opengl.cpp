/*
 * Copyright (c) 2023 Leia Inc.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software
 * and associated documentation files (the "Software"), to deal in the Software without restriction,
 * including without limitation the rights to use, copy, modify, merge, publish, distribute,
 * sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all copies or
 * substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING
 * BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
 * DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */
#include <sstream> // stringbuf
#include <iostream> // stringbuf

#include <Windows.h>

#include "leia/common/assert.h"
#include "leia/lwEngine/LWE_Types.h"
#include "leia/lwEngine/LWE_Core.h"

#include <imgui.h>

namespace leia {
namespace lwe {

using namespace sdk;

/// <summary>
/// Class used to redirect std::cout to the output window for non-console subsystems.
/// </summary>
class CCOutRedirector : public std::stringbuf {
public:
    ~CCOutRedirector()
    {
        sync();
    }

    int sync()
    {
        ::OutputDebugStringA(str().c_str());
        str(std::string()); // Clear the string buffer
        return 0;
    }
};

// Forward declarations
LRESULT CALLBACK WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

bool finished = false; // todo: this is used in LWE_Core to abort when there is an error. Find better mechanism.

// Global Variables
static HWND            ghWnd;
static LWE_Core*       EnginePtr;
static LARGE_INTEGER   gPerformanceCounterFrequency = {0, 0};
static int             mouseX                       = 0;
static int             mouseY                       = 0;
static bool            hasFocus                     = false;
static CCOutRedirector gDebugCOutRedirector;
static bool            gIsFullscreen          = true;
static const bool      preferSecondaryMonitor = true;

void render_custom_gui()
{
    LWE_Core& Engine = *EnginePtr;

    if (ImGui::Button("Cubes"))
        Engine.SetShaderDebugMode(LEIA_SHADER_DEBUG_MODE_NONE);
    ImGui::SameLine();
    if (ImGui::Button("Stereo Image"))
        Engine.SetShaderDebugMode(LEIA_SHADER_DEBUG_MODE_SHOW_BEER_GLASS);
    ImGui::SameLine();
    if (ImGui::Button("Test Pattern"))
        Engine.SetShaderDebugMode(LEIA_SHADER_DEBUG_MODE_SHOW_CALIBRATION_IMAGE);
}

extern "C" int APIENTRY wWinMain(_In_ HINSTANCE hCurrentInst, _In_opt_ HINSTANCE hPreviousInst, _In_ LPWSTR lpszCmdLine, _In_ int nCmdShow)
try
{
    if (AllocConsole())
    {
        AttachConsole(GetCurrentProcessId());
        FILE* dummy;
        if (freopen_s(&dummy, "CON", "w", stdout) != 0 || freopen_s(&dummy, "CON", "w", stderr) != 0)
        {
            return EXIT_FAILURE;
        }
    }

    // Trick to redirect std::cout calls to the output window (for non-console applications)
    std::cout.rdbuf(&gDebugCOutRedirector);

    // Use OpenGL by default but allow selecting graphics api via command-line.
    leia_graphics_api graphicsAPI = LEIA_GRAPHICS_API_OPENGL;
    if (0 == lstrcmpi(lpszCmdLine, L"d3d11"))
        graphicsAPI = LEIA_GRAPHICS_API_D3D11;
    else if (0 == lstrcmpi(lpszCmdLine, L"d3d12"))
        graphicsAPI = LEIA_GRAPHICS_API_D3D12;
    else if (0 == lstrcmpi(lpszCmdLine, L"vulkan"))
        graphicsAPI = LEIA_GRAPHICS_API_VULKAN;

    EnginePtr        = new LWE_Core(graphicsAPI);
    LWE_Core& Engine = *EnginePtr;

    HRESULT hr = CoInitializeEx(nullptr, COINIT_MULTITHREADED);
    if (FAILED(hr))
        return hr;

    CRITICAL_SECTION gMainCriticalSection;
    InitializeCriticalSection(&gMainCriticalSection);

    LWE_CoreInitArgs initArgs;

    initArgs.coreConfig.SetPlatformLogLevel(kLeiaLogLevelInfo);

    initArgs.window.title                  = "Leia CNSDK";
    initArgs.window.x                      = 0;
    initArgs.window.y                      = 0;
    initArgs.window.width                  = Engine.GetWindowWidth();
    initArgs.window.height                 = Engine.GetWindowHeight();
    initArgs.window.fullscreen             = gIsFullscreen;
    initArgs.window.preferSecondaryMonitor = preferSecondaryMonitor;
    initArgs.window.proc                   = WindowProc;
    initArgs.customGui                     = render_custom_gui;

    ghWnd = Engine.Initialize(initArgs);
    if (ghWnd == NULL)
        exit(1);

    HDC hDC = GetDC(ghWnd);

    ShowWindow(ghWnd, nCmdShow);

    if (!QueryPerformanceFrequency(&gPerformanceCounterFrequency))
        printf("QueryPerformanceFrequency failed!\n");

    // RECT r;
    // GetWindowRect(ghWnd, &r);
    // ClipCursor(&r);
    // ShowCursor(false);

    /*int err = glGetError();
    if (err)
    {
            printf("");
    }*/

    LARGE_INTEGER PreviousTime;
    PreviousTime.QuadPart = 0;
    QueryPerformanceCounter(&PreviousTime);

    MSG msg;
    while (!finished)
    {
        // Empty all messages from queue.
        while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
        {
            if (WM_QUIT == msg.message)
            {
                finished = true;
                break;
            }
            else
            {
                TranslateMessage(&msg);
                DispatchMessage(&msg);
            }
        }

        // Perform app logic.
        if (!finished)
        {
            // Compute time elapsed from previous idle state.
            LARGE_INTEGER CurrentTime;
            QueryPerformanceCounter(&CurrentTime);
            const float ElapsedTime = float(CurrentTime.QuadPart - PreviousTime.QuadPart) / float(gPerformanceCounterFrequency.QuadPart);
            PreviousTime            = CurrentTime;

            // Perform idle task.
            EnterCriticalSection(&gMainCriticalSection);
            Engine.Tick(ElapsedTime);
            LeaveCriticalSection(&gMainCriticalSection);

            // Perform idle task.
            EnterCriticalSection(&gMainCriticalSection);
            Engine.Render(hDC);
            LeaveCriticalSection(&gMainCriticalSection);
        }
    }

    Engine.Shutdown();
    delete EnginePtr;

    wglMakeCurrent(NULL, NULL);
    ReleaseDC(ghWnd, hDC);
    DestroyWindow(ghWnd);
    DeleteCriticalSection(&gMainCriticalSection);

    return (int)msg.wParam;
}
catch (std::exception& e)
{
    printf("Unhandled exception: %s", e.what());
    system("pause");
    return -1;
}

LRESULT CALLBACK WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    LWE_Core& Engine = *EnginePtr;

    leia_interlacer_gui_input_state io;
    if (Engine.IsInitialized())
    {
        io = Engine.ProcessGuiInput(hWnd, uMsg, wParam, lParam);
        if (io.wantCaptureInput)
        {
            return 0;
        }
    }

    static PAINTSTRUCT ps;

    switch (uMsg)
    {
        case WM_PAINT:
            BeginPaint(hWnd, &ps);
            EndPaint(hWnd, &ps);
            return 0;

        case WM_SIZE:
            Engine.OnWindowSizeChanged(LOWORD(lParam), HIWORD(lParam));
            PostMessage(hWnd, WM_PAINT, 0, 0);
            return 0;

        case WM_KEYDOWN:
            if (!io.wantCaptureKeyboard)
            {
                switch (wParam)
                {
                    case VK_PRIOR:
                        Engine.BaselineScalingMore();
                        break; // PgUp
                    case VK_NEXT:
                        Engine.BaselineScalingLess();
                        break; // PgDn
                    case VK_HOME:
                        Engine.InterlaceCenterViewMore();
                        break;
                    case VK_END:
                        Engine.InterlaceCenterViewLess();
                        break;
                    case VK_UP:
                        Engine.MoveForward(100.0f);
                        break;
                    case VK_DOWN:
                        Engine.MoveBackward(100.0f);
                        break;
                    case VK_LEFT:
                        Engine.MoveLeft(100.0f);
                        break;
                    case VK_RIGHT:
                        Engine.MoveRight(100.0f);
                        break;
                    case VK_OEM_MINUS:
                        Engine.ReconvergenceLess();
                        break;
                    case VK_OEM_PLUS:
                        Engine.ReconvergenceMore();
                        break;
                    case '7':
                        Engine.AspectRatioOffsetLess();
                        break;
                    case '8':
                        Engine.AspectRatioOffsetMore();
                        break;
                }
            }
            break;

        case WM_CHAR:
            if (!io.wantCaptureKeyboard)
            {
                switch (wParam)
                {
                    case VK_ESCAPE:
                        PostQuitMessage(0);
                        finished = true;
                        break;
                    case '1':
                        Engine.SetShaderDebugMode(LEIA_SHADER_DEBUG_MODE_NONE);
                        break;
                    case '2':
                        Engine.SetShaderDebugMode(LEIA_SHADER_DEBUG_MODE_SHOW_TEXTURE_COORDINATES);
                        break;
                    case '3':
                        Engine.SetShaderDebugMode(LEIA_SHADER_DEBUG_MODE_SHOW_ALL_VIEWS);
                        break;
                    case '4':
                        Engine.SetShaderDebugMode(LEIA_SHADER_DEBUG_MODE_SHOW_FIRST_VIEW);
                        break;
                    case '5':
                        Engine.SetShaderDebugMode(LEIA_SHADER_DEBUG_MODE_SHOW_CALIBRATION_IMAGE);
                        break;
                    case '6':
                        Engine.SetShaderDebugMode(LEIA_SHADER_DEBUG_MODE_SHOW_BEER_GLASS);
                        break;
                    case 'a':
                        Engine.ToggleAnimation();
                        break;
                    case 's':
                        Engine.ToggleSharpening();
                        break;
                    case 'x':
                        Engine.ToggleUseLeiaSDK();
                        break;
                    case 'f':
                        Engine.ToggleFaceTracking();
                        break;
                    case 'w':
                        Engine.SetFullscreen(gIsFullscreen = !gIsFullscreen);
                        break;
                    case 'r':
                        Engine.ToggleReconvergence();
                        break;
                    case 'd':
                        Engine.ToggleGui();
                        break;
                }
            }
            return 0;

        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;

        case WM_LBUTTONDOWN:
            return 0;

        case WM_LBUTTONUP:
            return 0;

        case WM_RBUTTONDOWN:
            return 0;

        case WM_RBUTTONUP:
            return 0;

        case WM_SETFOCUS:
            hasFocus = true;
            return 0;

        case WM_KILLFOCUS:
            hasFocus = false;
            return 0;

        case WM_MOUSEMOVE:
            mouseX = LOWORD(lParam);
            mouseY = HIWORD(lParam);
            return 0;
    }

    return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

} // namespace lwe
} // namespace leia
