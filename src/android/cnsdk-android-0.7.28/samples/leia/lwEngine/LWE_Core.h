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
#pragma once

#include "LWE_Camera.h"
#include "LWE_Types.h"

#include "leia/common/platform.h"
#include "leia/common/Renderer.h"

#include "leia/sdk/core.interlacer.hpp"

// TODO: remove glm from the public API
#include <glm/glm.hpp>

#include <mutex>
#include <functional>

#if defined(LEIA_USE_OPENGL)
#    include <GL/CAPI_GLE.h>
#    if defined(__ANDROID__)
#        include <EGL/egl.h>
#        include <GLES2/gl2.h>
#    endif
#    if defined(LEIA_OS_ANDROID)
typedef EGLContext HGLRC;
#    endif
#endif

#if defined(__ANDROID__)
typedef void* HDC;
typedef void* HWND;
typedef void* WNDPROC;
struct ANativeWindow;
#else
typedef void ANativeWindow;
#endif

namespace leia {

// namespace sdk {
// class MLImage;
// }

namespace lwe {

struct LWE_WindowSettings {
    const char* title;
    int         x;
    int         y;
    int         width;
    int         height;
    bool        fullscreen;
    bool        preferSecondaryMonitor;

    WNDPROC        proc          = nullptr;
    ANativeWindow* androidWindow = nullptr;
};

struct LWE_CoreInitArgs {
    leia::sdk::CoreInitConfiguration coreConfig;

    LWE_WindowSettings window;

    HGLRC appGLContext = nullptr;

    std::function<void()> customGui;
};

class LWE_Core {
public:
    LWE_Core(leia::sdk::GraphicsAPI graphicsAPI);
    ~LWE_Core();

    HGLRC          WglContext    = NULL;
    HWND           hWnd          = NULL;
    ANativeWindow* androidWindow = nullptr;

    HWND Initialize(LWE_CoreInitArgs&);

    void OnPause();
    void OnResume();

    void Render(HDC hDC);

#if defined(WIN32)
private:
    HWND CreateGraphicsWindow(LWE_WindowSettings const&);
#endif

public:
    void SetFullscreen(bool fullscreen);
    void Tick(double deltaTime);
    void Shutdown();
    void OnWindowSizeChanged(int width, int height);

    void ToggleUseLeiaSDK()
    {
        useLeiaSDK = !useLeiaSDK;
    }
    void ToggleInterlaceMode();
    void SetShaderDebugMode(sdk::ShaderDebugMode mode);
    void SetInterlaceMode(sdk::InterlaceMode mode);
    void ToggleFaceTracking();
    void BaselineScalingMore();
    void BaselineScalingLess();
    void ToggleShaderDebugMode();
    void InterlaceCenterViewMore();
    void InterlaceCenterViewLess();
    void MoveForward(float amount);
    void MoveBackward(float amount);
    void MoveLeft(float amount);
    void MoveRight(float amount);
    int  GetWindowWidth() const;
    int  GetWindowHeight() const;
    void ToggleAnimation();
    void ToggleSharpening();
    bool IsInitialized() const;
    bool IsUsingLeiaSDK() const;
    void ToggleReconvergence();
    void ReconvergenceLess();
    void ReconvergenceMore();
    void AspectRatioOffsetMore();
    void AspectRatioOffsetLess();

    bool ToggleGui();
#if defined(LEIA_OS_WINDOWS)
    sdk::GuiInputState ProcessGuiInput(leia_interlacer_gui_surface surface, uint32_t msg, uint64_t wparam, int64_t lparam);
#elif defined(LEIA_OS_ANDROID)
    sdk::GuiInputState ProcessGuiInput(AInputEvent const*);
    sdk::GuiInputState ProcessGuiMotionInput(JNIEnv*, jobject motionEvent);
#endif

    void SetBacklight(bool enable);

private:
    bool DoInternalInit();
    void CreateResources();

    void RenderCustomGui();

private:
    std::unique_ptr<sdk::Core>       pLeiaSDK;
    std::unique_ptr<sdk::Interlacer> pLeiaInterlacer;

    int                   windowWidth     = 1600;
    int                   windowHeight    = 900;
    bool                  useLeiaSDK      = true;
    const bool            UseDebugContext = false;
    LWE_Camera            gCamera;
    std::function<void()> customGui;
    int                   sdkViewWidth  = 0;
    int                   sdkViewHeight = 0;
    int                   sdkViewCount  = 0;

    bool animationEnabled = true;
    bool isInitialized    = false;

    /*char                          guiInputTextBuffer[1024];
    std::unique_ptr<sdk::MLImage> mlImage;
    std::mutex                    mlImageMutex;*/

    leia_graphics_api graphicsAPI = LEIA_GRAPHICS_API_OPENGL;

    renderer::IRenderer*                 renderer    = nullptr;
    renderer::IShader*                   modelShader = nullptr;
    std::vector<renderer::IGeometry*>    geometry;
    std::vector<renderer::IRenderState*> geometryRenderState;

    float viewResolution[2];
};

} // namespace lwe
} // namespace leia
