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
#include <vector>

#include "LWE_Types.h"
#include "LWE_Core.h"

// #include "leia/sdk/ml.hpp"
// #include "leia/sdk/image.h"
// #include "leia/sdk/debugMenu.hpp"
#if defined(LEIA_USE_OPENGL)
#    include "leia/sdk/core.interlacer.opengl.hpp"
#endif
#if defined(LEIA_USE_VULKAN)
#    include "leia/sdk/core.interlacer.vulkan.hpp"
#endif
#if defined(LEIA_USE_DIRECTX)
#    include "leia/sdk/core.interlacer.d3d11.hpp"
#endif
#if defined(LEIA_USE_DIRECTX12)
#    include "leia/sdk/core.interlacer.d3d12.hpp"
#endif
#include "leia/common/assetManager.hpp"

#include <glm/ext/matrix_transform.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#ifndef _LWE_STANDALONE
// #    include "leia/common/filesystem.hpp"

#    include <Tracy.hpp>
#endif

#include <imgui.h>
#include <iostream>

#if defined(LEIA_USE_DIRECTX)

// D3D11 headers.
#    include <d3d11_1.h>
#    include <d3dcompiler.h>
#    include <dxgidebug.h>

// D3D11 libraries
#    pragma comment(lib, "d3d11.lib")
#    pragma comment(lib, "d3dcompiler.lib")

#endif

#if defined(LEIA_USE_DIRECTX12)

// D3D12 headers.
#    include <d3d12.h>
#    include <d3d12sdklayers.h>
#    include "leia/common/d3dx12.h"
#    include <dxgi1_6.h>

// D3D12 libraries
#    pragma comment(lib, "d3d12.lib")
#    pragma comment(lib, "dxgi.lib")

#endif

#if defined(LEIA_USE_VULKAN)

#    include <optional>
#    include <set>

// Vulkan headers.
#    include <vulkan/vulkan.hpp>
#    ifdef LEIA_OS_WINDOWS
#        include <vulkan/vulkan_win32.h>
#    else
#        include <vulkan/vulkan_android.h>
#    endif

#endif

using namespace glm;

namespace leia {
namespace lwe {

LEIA_FLOAT_SLICE(3) ToSlice(glm::vec3& v)
{
    return {glm::value_ptr(v), 3};
}
LEIA_CONST_FLOAT_SLICE(3) ToConstSlice(glm::vec3 const& v)
{
    return {glm::value_ptr(v), 3};
}
LEIA_FLOAT_SLICE(16) ToSlice(glm::mat4& v)
{
    return {glm::value_ptr(v), 16};
}
LEIA_CONST_FLOAT_SLICE(16) ToConstSlice(glm::mat4 const& v)
{
    return {glm::value_ptr(v), 16};
}

using namespace sdk;

// settings
const float FOV                        = 90.0f;
const float initialConvergenceDistance = 500.0f;

// used for ui rendering
static const float vertices[] = {
    // positions          // colors           // texture coords
    1.0f,  0.0f, 1.0f,  0.0f, 0.0f, 0.0f, 1.0f, 1.0f, // top right
    1.0f,  0.0f, -1.0f, 1.0f, 1.0f, 0.0f, 1.0f, 0.0f, // bottom right
    -1.0f, 0.0f, -1.0f, 2.0f, 0.0f, 1.0f, 0.0f, 0.0f, // bottom left
    -1.0f, 0.0f, 1.0f,  3.0f, 1.0f, 0.0f, 0.0f, 1.0f // top left
};

static const unsigned int indices[] = {
    0,
    1,
    3, // first triangle
    1,
    2,
    3 // second triangle
};

static const int useTextureAtlas = true;

#if defined(LEIA_USE_DIRECTX)

// D3D11 globals.
struct {
    D3D_DRIVER_TYPE         driverType             = D3D_DRIVER_TYPE_NULL;
    D3D_FEATURE_LEVEL       featureLevel           = D3D_FEATURE_LEVEL_11_0;
    DXGI_FORMAT             renderTargetViewFormat = DXGI_FORMAT_R8G8B8A8_UNORM;
    ID3D11Device*           device                 = nullptr;
    ID3D11Device1*          device1                = nullptr;
    ID3D11DeviceContext*    immediateContext       = nullptr;
    ID3D11DeviceContext1*   immediateContext1      = nullptr;
    IDXGISwapChain*         swapChain              = nullptr;
    IDXGISwapChain1*        swapChain1             = nullptr;
    ID3D11RenderTargetView* renderTargetView       = nullptr;
    ID3D11Texture2D*        depthStencilTexture    = nullptr;
    ID3D11DepthStencilView* depthStencilView       = nullptr;
    renderer::ITexture*     backBuffer             = nullptr;
} g_d3d11;
#endif

#if defined(LEIA_USE_DIRECTX12)
const int g_frameCount = 2;
struct {
    ID3D12Device*           device           = nullptr;
    ID3D12CommandQueue*     commandQueue     = nullptr;
    ID3D12CommandAllocator* commandAllocator = nullptr;

    IDXGISwapChain3*    swapChain                = nullptr;
    int                 frameIndex               = 0;
    DXGI_FORMAT         swapChainFormat          = DXGI_FORMAT_R8G8B8A8_UNORM;
    renderer::ITexture* backBuffer[g_frameCount] = {};

    ID3D12DescriptorHeap*       srvHeap              = nullptr;
    D3D12_CPU_DESCRIPTOR_HANDLE srvFontCpuDescHandle = {};
    D3D12_GPU_DESCRIPTOR_HANDLE srvFontGpuDescHandle = {};
    ID3D12GraphicsCommandList*  guiCommandList       = nullptr;

    ID3D12Resource*               renderTargets[g_frameCount]     = {};
    ID3D12DescriptorHeap*         rtvHeap                         = nullptr;
    UINT                          rtvDescriptorSize               = 0;
    UINT                          rtvHeapUsed                     = 0;
    CD3DX12_CPU_DESCRIPTOR_HANDLE renderTargetViews[g_frameCount] = {};

    ID3D12Resource*               depthStencil[g_frameCount]      = {};
    ID3D12DescriptorHeap*         dsvHeap                         = nullptr;
    UINT                          dsvDescriptorSize               = 0;
    UINT                          dsvHeapUsed                     = 0;
    CD3DX12_CPU_DESCRIPTOR_HANDLE depthStencilViews[g_frameCount] = {};

    ID3D12Fence* fence      = nullptr;
    UINT64       fenceValue = 0;
    HANDLE       fenceEvent = NULL;
} g_d3d12;

#endif

#if defined(LEIA_USE_VULKAN)

struct {
    VkInstance       instance               = nullptr;
    VkPhysicalDevice physicalDevice         = nullptr;
    VkDevice         device                 = nullptr;
    VkFormat         textureFormat          = VK_FORMAT_UNDEFINED;
    VkFormat         renderTargetFormat     = VK_FORMAT_UNDEFINED;
    VkFormat         depthStencilFormat     = VK_FORMAT_UNDEFINED;
    bool             enableValidationLayers = false;
#    if defined(LEIA_OS_WINDOWS)
    VkDebugUtilsMessengerEXT debugMessenger = nullptr;
#    elif defined(LEIA_OS_ANDROID)
    VkDebugReportCallbackEXT debugCallback = nullptr;
#    endif
    VkSurfaceKHR                 surface            = nullptr;
    VkQueue                      graphicsQueue      = nullptr;
    int                          graphicsQueueIndex = -1;
    VkQueue                      presentQueue       = nullptr;
    int                          presentQueueIndex  = -1;
    std::vector<VkImageLayout>   renderTargetStates;
    std::vector<VkImageLayout>   depthStencilStates;
    VkSwapchainKHR               swapChain = nullptr;
    std::vector<VkImage>         swapChainImages;
    VkExtent2D                   swapChainExtent = {};
    std::vector<VkImageView>     swapChainImageViews;
    std::vector<VkFramebuffer>   swapChainFramebuffers;
    VkRenderPass                 renderPass       = nullptr;
    VkCommandPool                commandPool      = nullptr;
    VkImage                      depthImage       = nullptr;
    VkDeviceMemory               depthImageMemory = nullptr;
    VkImageView                  depthImageView   = nullptr;
    std::vector<VkCommandBuffer> commandBuffers;
    std::vector<VkSemaphore>     imageAvailableSemaphores;
    std::vector<VkSemaphore>     renderFinishedSemaphores;
    std::vector<VkSemaphore>     readyToPresentSemaphores;
    std::vector<VkFence>         inFlightFences;
    int                          currentFrame      = 0;
    VkCommandBuffer              guiCommandBuffer  = nullptr;
    VkDescriptorPool             guiDescriptorPool = nullptr;
} g_vulkan;

const int MAX_FRAMES_IN_FLIGHT = 2;

const std::vector<const char*> g_validationLayers = {
#    if defined(LEIA_OS_WINDOWS)
    "VK_LAYER_KHRONOS_validation"
#    endif
};

const std::vector<const char*> g_deviceExtensions = {
    VK_KHR_SWAPCHAIN_EXTENSION_NAME,
    VK_KHR_MAINTENANCE1_EXTENSION_NAME,
};

const std::vector<const char*> g_requiredExtensions = {VK_KHR_SURFACE_EXTENSION_NAME,
#    if defined(LEIA_OS_WINDOWS)
                                                       VK_KHR_WIN32_SURFACE_EXTENSION_NAME,
#    endif
#    if defined(LEIA_OS_ANDROID)
                                                       VK_KHR_ANDROID_SURFACE_EXTENSION_NAME
#    endif
};

#endif

#if defined(LEIA_USE_DIRECTX)
HRESULT ResizeBuffersD3D11(renderer::IRenderer* renderer, int width, int height)
{
    if (g_d3d11.immediateContext == nullptr)
        return S_OK;

    if (g_d3d11.backBuffer != nullptr)
    {
        g_d3d11.backBuffer->Destroy();
        g_d3d11.backBuffer = nullptr;
    }

    if (g_d3d11.depthStencilView != nullptr)
    {
        g_d3d11.depthStencilView->Release();
        g_d3d11.depthStencilView = nullptr;
    }

    if (g_d3d11.depthStencilTexture != nullptr)
    {
        g_d3d11.depthStencilTexture->Release();
        g_d3d11.depthStencilTexture = nullptr;
    }

    if (g_d3d11.renderTargetView != nullptr)
    {
        g_d3d11.renderTargetView->Release();
        g_d3d11.renderTargetView = nullptr;
    }

    // Resize swapchain.
    HRESULT hr = g_d3d11.swapChain->ResizeBuffers(1, width /*g_windowWidth*/, height /*g_windowHeight*/, g_d3d11.renderTargetViewFormat, 0);
    if (FAILED(hr))
    {
        // LogPrint(L"Error while resizing swapchain (%s).\n", GetHRESULTString(hr));
        return hr;
    }

    // Get swapchain buffer.
    ID3D11Texture2D* pBackBuffer = nullptr;
    hr                           = g_d3d11.swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(&pBackBuffer));
    if (FAILED(hr))
    {
        // LogPrint(L"Error while getting swapchain buffer (%s).\n", GetHRESULTString(hr));
        return hr;
    }

    // Create a render target view.
    hr = g_d3d11.device->CreateRenderTargetView(pBackBuffer, nullptr, &g_d3d11.renderTargetView);
    pBackBuffer->Release();
    if (FAILED(hr))
    {
        // LogPrint(L"Error while creating rendertarget view (%s).\n", GetHRESULTString(hr));
        return hr;
    }

    D3D11_TEXTURE2D_DESC depthStencilDesc;
    depthStencilDesc.Width              = width;
    depthStencilDesc.Height             = height;
    depthStencilDesc.MipLevels          = 1;
    depthStencilDesc.ArraySize          = 1;
    depthStencilDesc.Format             = DXGI_FORMAT_D24_UNORM_S8_UINT;
    depthStencilDesc.SampleDesc.Count   = 1;
    depthStencilDesc.SampleDesc.Quality = 0;
    depthStencilDesc.Usage              = D3D11_USAGE_DEFAULT;
    depthStencilDesc.BindFlags          = D3D11_BIND_DEPTH_STENCIL;
    depthStencilDesc.CPUAccessFlags     = 0;
    depthStencilDesc.MiscFlags          = 0;

    // Create the Depth/Stencil View
    g_d3d11.device->CreateTexture2D(&depthStencilDesc, NULL, &g_d3d11.depthStencilTexture);
    g_d3d11.device->CreateDepthStencilView(g_d3d11.depthStencilTexture, NULL, &g_d3d11.depthStencilView);

    // Set render target and depth stencil.
    g_d3d11.immediateContext->OMSetRenderTargets(1, &g_d3d11.renderTargetView, g_d3d11.depthStencilView);

    // Here we create the "backbuffer" which can contain the rendertarget and depthbuffer.
    g_d3d11.backBuffer = renderer::AsD3D11(renderer)->CreateTextureAlias(
        pBackBuffer,
        nullptr,
        g_d3d11.renderTargetView,
        g_d3d11.depthStencilTexture,
        g_d3d11.depthStencilView,
        0,
        0,
        1,
        renderer::ePixelFormat::RGBA,
        renderer::eResourceUsageFlags::ShaderInput | renderer::eResourceUsageFlags::RenderTarget | renderer::eResourceUsageFlags::DepthStencil);

    return S_OK;
}

HRESULT InitializeD3D11(renderer::IRenderer* renderer, HWND hWnd)
{
    HRESULT hr = S_OK;

    // Get window size.
    RECT rc;
    GetClientRect(hWnd, &rc);
    const UINT width  = rc.right - rc.left;
    const UINT height = rc.bottom - rc.top;

    UINT createDeviceFlags = 0;
#    ifdef _DEBUG
    createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#    endif

    const D3D_DRIVER_TYPE driverTypes[] = {
        D3D_DRIVER_TYPE_HARDWARE,
        D3D_DRIVER_TYPE_WARP,
        D3D_DRIVER_TYPE_REFERENCE,
    };

    const int driverTypeCount = ARRAYSIZE(driverTypes);

    const D3D_FEATURE_LEVEL featureLevels[] = {
        D3D_FEATURE_LEVEL_11_1, D3D_FEATURE_LEVEL_11_0,
        // D3D_FEATURE_LEVEL_10_1,
        // D3D_FEATURE_LEVEL_10_0,
    };

    const int featureLevelCount = ARRAYSIZE(featureLevels);

    for (int i = 0; i < driverTypeCount; i++)
    {
        g_d3d11.driverType = driverTypes[i];

        // Create device.
        hr = D3D11CreateDevice(nullptr,
                               g_d3d11.driverType,
                               nullptr,
                               createDeviceFlags,
                               featureLevels,
                               featureLevelCount,
                               D3D11_SDK_VERSION,
                               &g_d3d11.device,
                               &g_d3d11.featureLevel,
                               &g_d3d11.immediateContext);

        // DirectX 11.0 platforms will not recognize D3D_FEATURE_LEVEL_11_1 so we need to retry without it
        if (hr == E_INVALIDARG)
        {
            hr = D3D11CreateDevice(nullptr,
                                   g_d3d11.driverType,
                                   nullptr,
                                   createDeviceFlags,
                                   &featureLevels[1],
                                   featureLevelCount - 1,
                                   D3D11_SDK_VERSION,
                                   &g_d3d11.device,
                                   &g_d3d11.featureLevel,
                                   &g_d3d11.immediateContext);
        }

        if (SUCCEEDED(hr))
            break;
    }

    if (FAILED(hr))
    {
        // LogPrint(L"Could not find a Direct3D11 device.\n");
        return hr;
    }

    // Obtain DXGI factory from device (since we used nullptr for pAdapter above)
    IDXGIFactory1* dxgiFactory = nullptr;
    {
        IDXGIDevice* dxgiDevice = nullptr;
        hr                      = g_d3d11.device->QueryInterface(__uuidof(IDXGIDevice), reinterpret_cast<void**>(&dxgiDevice));
        if (SUCCEEDED(hr))
        {
            IDXGIAdapter* adapter = nullptr;
            hr                    = dxgiDevice->GetAdapter(&adapter);
            if (SUCCEEDED(hr))
            {
                hr = adapter->GetParent(__uuidof(IDXGIFactory1), reinterpret_cast<void**>(&dxgiFactory));
                // if (FAILED(hr))
                // LogPrint(L"Error while getting DXGIFactory from IDXGIAdapter (%s).\n", GetHRESULTString(hr));
                adapter->Release();
            }
            else
            {
                // LogPrint(L"Error while getting IDXGIAdapter from IDXGIDevice (%s).\n", GetHRESULTString(hr));
            }
            dxgiDevice->Release();
        }
        else
        {
            // LogPrint(L"Error getting DXGIDevice from ID3D11Device (%s).\n", GetHRESULTString(hr));
        }

        if (FAILED(hr))
            return hr;
    }

    // Create swap chain
    {
        IDXGIFactory2* dxgiFactory2 = nullptr;
        hr                          = dxgiFactory->QueryInterface(__uuidof(IDXGIFactory2), reinterpret_cast<void**>(&dxgiFactory2));
        if (dxgiFactory2)
        {
            // DirectX 11.1 or later
            hr = g_d3d11.device->QueryInterface(__uuidof(ID3D11Device1), reinterpret_cast<void**>(&g_d3d11.device1));
            if (SUCCEEDED(hr))
            {
                g_d3d11.immediateContext->QueryInterface(__uuidof(ID3D11DeviceContext1), reinterpret_cast<void**>(&g_d3d11.immediateContext1));
            }
            else
            {
                // LogPrint(L"Error getting ID3D11DeviceContext1 from ID3D11DeviceContext (%s).\n", GetHRESULTString(hr));
            }

            DXGI_SWAP_CHAIN_DESC1 sd = {};
            sd.Width                 = width;
            sd.Height                = height;
            sd.Format                = g_d3d11.renderTargetViewFormat;
            sd.SampleDesc.Count      = 1;
            sd.SampleDesc.Quality    = 0;
            sd.BufferUsage           = DXGI_USAGE_RENDER_TARGET_OUTPUT;
            sd.BufferCount           = 1;

            hr = dxgiFactory2->CreateSwapChainForHwnd(g_d3d11.device, hWnd, &sd, nullptr, nullptr, &g_d3d11.swapChain1);
            if (SUCCEEDED(hr))
            {
                hr = g_d3d11.swapChain1->QueryInterface(__uuidof(IDXGISwapChain), reinterpret_cast<void**>(&g_d3d11.swapChain));
                // if (FAILED(hr))
                // LogPrint(L"Error getting IDXGISwapChain from IDXGISwapChain1 (%s).\n", GetHRESULTString(hr));
            }
            else
            {
                // LogPrint(L"Error creating IDXGISwapChain1 (%s).\n", GetHRESULTString(hr));
            }

            dxgiFactory2->Release();
        }
        else
        {
            // DirectX 11.0 systems
            DXGI_SWAP_CHAIN_DESC sd               = {};
            sd.BufferCount                        = 1;
            sd.BufferDesc.Width                   = width;
            sd.BufferDesc.Height                  = height;
            sd.BufferDesc.Format                  = g_d3d11.renderTargetViewFormat;
            sd.BufferDesc.RefreshRate.Numerator   = 60;
            sd.BufferDesc.RefreshRate.Denominator = 1;
            sd.BufferUsage                        = DXGI_USAGE_RENDER_TARGET_OUTPUT;
            sd.OutputWindow                       = hWnd;
            sd.SampleDesc.Count                   = 1;
            sd.SampleDesc.Quality                 = 0;
            sd.Windowed                           = TRUE;

            hr = dxgiFactory->CreateSwapChain(g_d3d11.device, &sd, &g_d3d11.swapChain);
            // if (FAILED(hr))
            // LogPrint(L"Error creating IDXGISwapChain (%s).\n", GetHRESULTString(hr));
        }

        // Note this tutorial doesn't handle full-screen swapchains so we block the ALT+ENTER shortcut
        dxgiFactory->MakeWindowAssociation(hWnd, 0); // DXGI_MWA_NO_ALT_ENTER);

        dxgiFactory->Release();

        if (FAILED(hr))
            return hr;
    }

    // Update render-target view.
    hr = ResizeBuffersD3D11(renderer, width, height);
    if (FAILED(hr))
        return hr;

    return S_OK;
}

#endif

#if defined(LEIA_USE_DIRECTX12)

// Helper function for acquiring the first available hardware adapter that supports Direct3D 12.
// If no such adapter can be found, *ppAdapter will be set to nullptr.
void GetHardwareAdapter(IDXGIFactory1* pFactory, IDXGIAdapter1** ppAdapter, bool requestHighPerformanceAdapter = false)
{
    *ppAdapter = nullptr;

    IDXGIAdapter1* adapter = nullptr;

    IDXGIFactory6* factory6 = nullptr;
    if (SUCCEEDED(pFactory->QueryInterface(_uuidof(IDXGIAdapter1), (void**)&adapter)))
    {
        for (UINT adapterIndex = 0; SUCCEEDED(factory6->EnumAdapterByGpuPreference(adapterIndex,
                                                                                   requestHighPerformanceAdapter == true ? DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE
                                                                                                                         : DXGI_GPU_PREFERENCE_UNSPECIFIED,
                                                                                   IID_PPV_ARGS(&adapter)));
             ++adapterIndex)
        {
            DXGI_ADAPTER_DESC1 desc;
            adapter->GetDesc1(&desc);

            if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
            {
                // Don't select the Basic Render Driver adapter.
                // If you want a software adapter, pass in "/warp" on the command line.
                continue;
            }

            // Check to see whether the adapter supports Direct3D 12, but don't create the
            // actual device yet.
            if (SUCCEEDED(D3D12CreateDevice(adapter, D3D_FEATURE_LEVEL_11_0, _uuidof(ID3D12Device), nullptr)))
            {
                break;
            }
        }
    }

    if (adapter == nullptr)
    {
        for (UINT adapterIndex = 0; SUCCEEDED(pFactory->EnumAdapters1(adapterIndex, &adapter)); ++adapterIndex)
        {
            DXGI_ADAPTER_DESC1 desc;
            adapter->GetDesc1(&desc);

            if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
            {
                // Don't select the Basic Render Driver adapter.
                // If you want a software adapter, pass in "/warp" on the command line.
                continue;
            }

            // Check to see whether the adapter supports Direct3D 12, but don't create the
            // actual device yet.
            if (SUCCEEDED(D3D12CreateDevice(adapter, D3D_FEATURE_LEVEL_11_0, _uuidof(ID3D12Device), nullptr)))
            {
                break;
            }
        }
    }

    *ppAdapter = adapter;
}

void WaitForPreviousFrame()
{
    // WAITING FOR THE FRAME TO COMPLETE BEFORE CONTINUING IS NOT BEST PRACTICE.
    // This is code implemented as such for simplicity. The D3D12HelloFrameBuffering
    // sample illustrates how to use fences for efficient resource usage and to
    // maximize GPU utilization.

    // Signal and increment the fence value.
    const UINT64 fenceVal = g_d3d12.fenceValue;
    HRESULT      hr       = g_d3d12.commandQueue->Signal(g_d3d12.fence, fenceVal);
    g_d3d12.fenceValue++;

    // Wait until the previous frame is finished.
    if (g_d3d12.fence->GetCompletedValue() < fenceVal)
    {
        hr = g_d3d12.fence->SetEventOnCompletion(fenceVal, g_d3d12.fenceEvent);
        WaitForSingleObject(g_d3d12.fenceEvent, INFINITE);
    }

    g_d3d12.frameIndex = g_d3d12.swapChain->GetCurrentBackBufferIndex();
}

HRESULT ResizeBuffersD3D12(renderer::IRenderer* renderer, int width, int height)
{
    if (g_d3d12.device == nullptr)
        return S_OK;

    for (int i = 0; i < g_frameCount; i++)
    {
        if (g_d3d12.backBuffer[i] != nullptr)
        {
            g_d3d12.backBuffer[i]->Destroy();
            g_d3d12.backBuffer[i] = nullptr;
        }
    }

    for (int i = 0; i < g_frameCount; i++)
    {
        if (g_d3d12.renderTargets[i] != nullptr)
        {
            g_d3d12.renderTargets[i]->Release();
            g_d3d12.renderTargets[i] = nullptr;
        }

        if (g_d3d12.depthStencil[i] != nullptr)
        {
            g_d3d12.depthStencil[i]->Release();
            g_d3d12.depthStencil[i] = nullptr;
        }
    }

    g_d3d12.rtvHeapUsed = 0;
    g_d3d12.dsvHeapUsed = 0;

    memset(g_d3d12.renderTargetViews, 0, sizeof(g_d3d12.renderTargetViews));
    memset(g_d3d12.depthStencilViews, 0, sizeof(g_d3d12.depthStencilViews));

    // Resize swapchain.
    HRESULT hr = g_d3d12.swapChain->ResizeBuffers(g_frameCount, width, height, g_d3d12.swapChainFormat, 0);
    if (FAILED(hr))
    {
        // LogPrint(L"Error while resizing swapchain (%s).\n", GetHRESULTString(hr));
        return hr;
    }

    // Create frame resources.
    {
        // Create a RTV for each frame.
        for (int n = 0; n < g_frameCount; n++)
        {
            g_d3d12.renderTargetViews[n] = CD3DX12_CPU_DESCRIPTOR_HANDLE(g_d3d12.rtvHeap->GetCPUDescriptorHandleForHeapStart());
            g_d3d12.renderTargetViews[n].Offset(g_d3d12.rtvHeapUsed); // 1, g_d3d12.rtvHeapUsed);

            hr = g_d3d12.swapChain->GetBuffer(n, _uuidof(ID3D12Resource), (void**)&g_d3d12.renderTargets[n]);
            g_d3d12.device->CreateRenderTargetView(g_d3d12.renderTargets[n], nullptr, g_d3d12.renderTargetViews[n]);

            g_d3d12.rtvHeapUsed += g_d3d12.rtvDescriptorSize;
        }

        // Create a depth stencil buffer and a DSV for each frame.
        {
            D3D12_DEPTH_STENCIL_VIEW_DESC depthStencilDesc = {};
            depthStencilDesc.Format                        = DXGI_FORMAT_D32_FLOAT;
            depthStencilDesc.ViewDimension                 = D3D12_DSV_DIMENSION_TEXTURE2D;
            depthStencilDesc.Flags                         = D3D12_DSV_FLAG_NONE;

            D3D12_CLEAR_VALUE depthOptimizedClearValue    = {};
            depthOptimizedClearValue.Format               = DXGI_FORMAT_D32_FLOAT;
            depthOptimizedClearValue.DepthStencil.Depth   = 1.0f;
            depthOptimizedClearValue.DepthStencil.Stencil = 0;

            for (int n = 0; n < g_frameCount; n++)
            {
                CD3DX12_HEAP_PROPERTIES heapProps(D3D12_HEAP_TYPE_DEFAULT);
                CD3DX12_RESOURCE_DESC   resourceDesc =
                    CD3DX12_RESOURCE_DESC::Tex2D(DXGI_FORMAT_D32_FLOAT, width, height, 1, 0, 1, 0, D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL);

                hr = g_d3d12.device->CreateCommittedResource(&heapProps,
                                                             D3D12_HEAP_FLAG_NONE,
                                                             &resourceDesc,
                                                             D3D12_RESOURCE_STATE_DEPTH_WRITE,
                                                             &depthOptimizedClearValue,
                                                             _uuidof(ID3D12Resource),
                                                             (void**)&g_d3d12.depthStencil[n]);

                g_d3d12.depthStencilViews[n] = CD3DX12_CPU_DESCRIPTOR_HANDLE(g_d3d12.dsvHeap->GetCPUDescriptorHandleForHeapStart());
                g_d3d12.depthStencilViews[n].Offset(g_d3d12.dsvHeapUsed); // 1, g_d3d12.dsvHeapUsed);

                g_d3d12.device->CreateDepthStencilView(g_d3d12.depthStencil[n], &depthStencilDesc, g_d3d12.depthStencilViews[n]);

                g_d3d12.dsvHeapUsed += g_d3d12.dsvDescriptorSize;
            }
        }
    }

    return S_OK;
}

HRESULT InitializeD3D12(renderer::IRenderer* renderer, HWND hWnd)
{
    HRESULT hr = S_OK;

    UINT dxgiFactoryFlags = 0;

#    if defined(_DEBUG)
    // Enable the debug layer (requires the Graphics Tools "optional feature").
    // NOTE: Enabling the debug layer after device creation will invalidate the active device.
    {
        ID3D12Debug* debugController = nullptr;
        if (SUCCEEDED(D3D12GetDebugInterface(_uuidof(ID3D12Debug), (void**)&debugController)))
        {
            debugController->EnableDebugLayer();

            // Enable additional debug layers.
            dxgiFactoryFlags |= DXGI_CREATE_FACTORY_DEBUG;
        }
    }
#    endif

    IDXGIFactory4* factory = nullptr;
    hr                     = CreateDXGIFactory2(dxgiFactoryFlags, _uuidof(IDXGIFactory4), (void**)&factory);
    if (FAILED(hr))
        return hr;

    IDXGIAdapter1* hardwareAdapter = nullptr;
    GetHardwareAdapter(factory, &hardwareAdapter);

    hr = D3D12CreateDevice(hardwareAdapter, D3D_FEATURE_LEVEL_11_0, _uuidof(ID3D12Device), (void**)&g_d3d12.device);
    if (FAILED(hr))
        return hr;

    // Get window size.
    RECT rc;
    GetClientRect(hWnd, &rc);
    const UINT width  = rc.right - rc.left;
    const UINT height = rc.bottom - rc.top;

    // Create the command queue.
    D3D12_COMMAND_QUEUE_DESC queueDesc = {};
    queueDesc.Flags                    = D3D12_COMMAND_QUEUE_FLAG_NONE;
    queueDesc.Type                     = D3D12_COMMAND_LIST_TYPE_DIRECT;
    hr                                 = g_d3d12.device->CreateCommandQueue(&queueDesc, _uuidof(ID3D12CommandQueue), (void**)&g_d3d12.commandQueue);
    if (FAILED(hr))
        return false;

    // Create the swap chain.
    DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
    swapChainDesc.BufferCount           = g_frameCount;
    swapChainDesc.Width                 = width;
    swapChainDesc.Height                = height;
    swapChainDesc.Format                = g_d3d12.swapChainFormat;
    swapChainDesc.BufferUsage           = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    swapChainDesc.SwapEffect            = DXGI_SWAP_EFFECT_FLIP_DISCARD;
    swapChainDesc.SampleDesc.Count      = 1;
    IDXGISwapChain1* swapChain          = nullptr;
    hr                                  = factory->CreateSwapChainForHwnd(g_d3d12.commandQueue, hWnd, &swapChainDesc, nullptr, nullptr, &swapChain);
    if (FAILED(hr))
        return false;
    swapChain->QueryInterface(_uuidof(IDXGISwapChain3), (void**)&g_d3d12.swapChain); // g_d3d12.swapChain = dynamic_cast<IDXGISwapChain3*>(swapChain);
    if (g_d3d12.swapChain == nullptr)
        return false;

    //
    hr = factory->MakeWindowAssociation(hWnd, 0); // DXGI_MWA_NO_ALT_ENTER
    if (FAILED(hr))
        return false;

    g_d3d12.frameIndex = g_d3d12.swapChain->GetCurrentBackBufferIndex();

    // Create descriptor heaps.
    {
        // Describe and create a render target view (RTV) descriptor heap.
        D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = {};
        rtvHeapDesc.NumDescriptors             = g_frameCount;
        rtvHeapDesc.Type                       = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
        rtvHeapDesc.Flags                      = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
        hr                                     = g_d3d12.device->CreateDescriptorHeap(&rtvHeapDesc, _uuidof(ID3D12DescriptorHeap), (void**)&g_d3d12.rtvHeap);
        g_d3d12.rtvDescriptorSize              = g_d3d12.device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

        // Describe and create a depth stencil view (DSV) descriptor heap.
        D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc = {};
        dsvHeapDesc.NumDescriptors             = g_frameCount;
        dsvHeapDesc.Type                       = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
        dsvHeapDesc.Flags                      = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
        hr                                     = g_d3d12.device->CreateDescriptorHeap(&dsvHeapDesc, _uuidof(ID3D12DescriptorHeap), (void**)&g_d3d12.dsvHeap);
        g_d3d12.dsvDescriptorSize              = g_d3d12.device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);

        // Describe and create a shader resource view (SRV) descriptor heap.
        D3D12_DESCRIPTOR_HEAP_DESC srvHeapDesc = {};
        srvHeapDesc.NumDescriptors             = g_frameCount;
        srvHeapDesc.Type                       = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
        srvHeapDesc.Flags                      = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
        hr                                     = g_d3d12.device->CreateDescriptorHeap(&srvHeapDesc, _uuidof(ID3D12DescriptorHeap), (void**)&g_d3d12.srvHeap);
    }

    hr = g_d3d12.device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, _uuidof(ID3D12CommandAllocator), (void**)&g_d3d12.commandAllocator);
    if (FAILED(hr))
    {
        LNK_ASSERT(false);
        return hr;
    }

    {
        // Create SRV for GUI font.
        g_d3d12.srvFontCpuDescHandle = g_d3d12.srvHeap->GetCPUDescriptorHandleForHeapStart();
        g_d3d12.srvFontGpuDescHandle = g_d3d12.srvHeap->GetGPUDescriptorHandleForHeapStart();

        // Create command list.
        hr = g_d3d12.device->CreateCommandList(
            0, D3D12_COMMAND_LIST_TYPE_DIRECT, g_d3d12.commandAllocator, nullptr, _uuidof(ID3D12GraphicsCommandList), (void**)&g_d3d12.guiCommandList);
        if (FAILED(hr))
        {
            LNK_ASSERT(false);
        }

        hr = g_d3d12.guiCommandList->Close();
        if (FAILED(hr))
        {
            LNK_ASSERT(false);
        }
    }

    // Create frame resources.
    {
        // Create a RTV for each frame.
        for (int n = 0; n < g_frameCount; n++)
        {
            g_d3d12.renderTargetViews[n] = CD3DX12_CPU_DESCRIPTOR_HANDLE(g_d3d12.rtvHeap->GetCPUDescriptorHandleForHeapStart());
            g_d3d12.renderTargetViews[n].Offset(g_d3d12.rtvHeapUsed); // 1, g_d3d12.rtvHeapUsed);

            hr = g_d3d12.swapChain->GetBuffer(n, _uuidof(ID3D12Resource), (void**)&g_d3d12.renderTargets[n]);
            g_d3d12.device->CreateRenderTargetView(g_d3d12.renderTargets[n], nullptr, g_d3d12.renderTargetViews[n]);

            g_d3d12.rtvHeapUsed += g_d3d12.rtvDescriptorSize;
        }

        // Create a depth stencil buffer and a DSV for each frame.
        {
            D3D12_DEPTH_STENCIL_VIEW_DESC depthStencilDesc = {};
            depthStencilDesc.Format                        = DXGI_FORMAT_D32_FLOAT;
            depthStencilDesc.ViewDimension                 = D3D12_DSV_DIMENSION_TEXTURE2D;
            depthStencilDesc.Flags                         = D3D12_DSV_FLAG_NONE;

            D3D12_CLEAR_VALUE depthOptimizedClearValue    = {};
            depthOptimizedClearValue.Format               = DXGI_FORMAT_D32_FLOAT;
            depthOptimizedClearValue.DepthStencil.Depth   = 1.0f;
            depthOptimizedClearValue.DepthStencil.Stencil = 0;

            for (int n = 0; n < g_frameCount; n++)
            {
                CD3DX12_HEAP_PROPERTIES heapProps(D3D12_HEAP_TYPE_DEFAULT);
                CD3DX12_RESOURCE_DESC   resourceDesc =
                    CD3DX12_RESOURCE_DESC::Tex2D(DXGI_FORMAT_D32_FLOAT, width, height, 1, 0, 1, 0, D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL);

                hr = g_d3d12.device->CreateCommittedResource(&heapProps,
                                                             D3D12_HEAP_FLAG_NONE,
                                                             &resourceDesc,
                                                             D3D12_RESOURCE_STATE_DEPTH_WRITE,
                                                             &depthOptimizedClearValue,
                                                             _uuidof(ID3D12Resource),
                                                             (void**)&g_d3d12.depthStencil[n]);

                g_d3d12.depthStencilViews[n] = CD3DX12_CPU_DESCRIPTOR_HANDLE(g_d3d12.dsvHeap->GetCPUDescriptorHandleForHeapStart());
                g_d3d12.depthStencilViews[n].Offset(g_d3d12.dsvHeapUsed); // 1, g_d3d12.dsvHeapUsed);

                g_d3d12.device->CreateDepthStencilView(g_d3d12.depthStencil[n], &depthStencilDesc, g_d3d12.depthStencilViews[n]);

                g_d3d12.dsvHeapUsed += g_d3d12.dsvDescriptorSize;
            }
        }
    }

    // Create synchronization objects and wait until assets have been uploaded to the GPU.
    {
        hr = g_d3d12.device->CreateFence(0, D3D12_FENCE_FLAG_NONE, _uuidof(ID3D12Fence), (void**)&g_d3d12.fence);
        if (FAILED(hr))
            return false;
        g_d3d12.fenceValue = 1;

        // Create an event handle to use for frame synchronization.
        g_d3d12.fenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
        if (g_d3d12.fenceEvent == nullptr)
        {
            hr = HRESULT_FROM_WIN32(GetLastError());
            if (FAILED(hr))
                return false;
        }

        // Wait for the command list to execute; we are reusing the same command
        // list in our main loop but for now, we just want to wait for setup to
        // complete before continuing.
        WaitForPreviousFrame();
    }

    // Update render-target view.
    // hr = ResizeBuffersD3D12(renderer, width, height);
    // if (FAILED(hr))
    //  return hr;

    return S_OK;
}

#endif

#if defined(LEIA_USE_VULKAN)

bool checkValidationLayerSupport()
{
    uint32_t layerCount = 0;
    vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

    std::vector<VkLayerProperties> availableLayers(layerCount);
    vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

    for (const char* layerName : g_validationLayers)
    {
        bool layerFound = false;
        for (const auto& layerProperties : availableLayers)
        {
            if (strcmp(layerName, layerProperties.layerName) == 0)
            {
                layerFound = true;
                break;
            }
        }

        if (!layerFound)
            return false;
    }

    return true;
}

#    if defined(LEIA_OS_WINDOWS)
static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT      messageSeverity,
                                                    VkDebugUtilsMessageTypeFlagsEXT             messageType,
                                                    const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
                                                    void*                                       pUserData)
{
    if (/*messageSeverity == VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT || */ messageSeverity == VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT)
    {
        OutputDebugStringA(pCallbackData->pMessage);
        OutputDebugStringA("\n");
    }
    return VK_FALSE;
}
#    elif defined(LEIA_OS_ANDROID)
static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugReportFlagsEXT      flags,
                                                    VkDebugReportObjectTypeEXT type,
                                                    uint64_t                   object,
                                                    size_t                     location,
                                                    int32_t                    message_code,
                                                    const char*                layer_prefix,
                                                    const char*                message,
                                                    void*                      user_data)
{
    if (flags & VK_DEBUG_REPORT_ERROR_BIT_EXT)
    {
        std::cout << "\"Validation Layer: Error:" << layer_prefix << message << std::endl;
    }
    else if (flags & VK_DEBUG_REPORT_WARNING_BIT_EXT)
    {
        std::cout << "\"Validation Layer: Warning:" << layer_prefix << message << std::endl;
    }
    else if (flags & VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT)
    {
        std::cout << "\"Validation Layer: Performance warning:" << layer_prefix << message << std::endl;
    }
    else
    {
        std::cout << "\"Validation Layer: Information:" << layer_prefix << message << std::endl;
    }
    return VK_FALSE;
}
#    endif

VkResult CreateDebugUtilsMessengerEXT(VkInstance                                instance,
                                      const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
                                      const VkAllocationCallbacks*              pAllocator,
                                      VkDebugUtilsMessengerEXT*                 pDebugMessenger)
{
    auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");

    if (func != nullptr)
        return func(instance, pCreateInfo, pAllocator, pDebugMessenger);

    return VK_ERROR_EXTENSION_NOT_PRESENT;
}

VkResult CreateDebugReportCallbackEXT(VkInstance                                instance,
                                      const VkDebugReportCallbackCreateInfoEXT* pCreateInfo,
                                      const VkAllocationCallbacks*              pAllocator,
                                      VkDebugReportCallbackEXT*                 pDebugCallback)
{
    auto func = (PFN_vkCreateDebugReportCallbackEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugReportCallbackEXT");

    if (func != nullptr)
        return func(instance, pCreateInfo, pAllocator, pDebugCallback);

    return VK_ERROR_EXTENSION_NOT_PRESENT;
}

struct QueueFamilyIndices {
    std::optional<uint32_t> graphicsFamily;
    std::optional<uint32_t> presentFamily;

    bool isComplete()
    {
        return graphicsFamily.has_value() && presentFamily.has_value();
    }
};

QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device)
{
    QueueFamilyIndices indices = {};

    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

    std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

    int i = 0;
    for (const auto& queueFamily : queueFamilies)
    {
        if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT)
            indices.graphicsFamily = i;

        VkBool32 presentSupport = false;
        vkGetPhysicalDeviceSurfaceSupportKHR(device, i, g_vulkan.surface, &presentSupport);

        if (presentSupport)
            indices.presentFamily = i;

        if (indices.isComplete())
            break;

        i++;
    }

    return indices;
}

bool checkDeviceExtensionSupport(VkPhysicalDevice device)
{
    uint32_t extensionCount = 0;
    vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

    std::vector<VkExtensionProperties> availableExtensions(extensionCount);
    vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());

    std::set<std::string> requiredExtensions(g_deviceExtensions.begin(), g_deviceExtensions.end());

    for (const auto& extension : availableExtensions)
        requiredExtensions.erase(extension.extensionName);

    return requiredExtensions.empty();
}

struct SwapChainSupportDetails {
    VkSurfaceCapabilitiesKHR        capabilities;
    std::vector<VkSurfaceFormatKHR> formats;
    std::vector<VkPresentModeKHR>   presentModes;
};

SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device)
{
    SwapChainSupportDetails details = {};
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, g_vulkan.surface, &details.capabilities);

    uint32_t formatCount = 0;
    vkGetPhysicalDeviceSurfaceFormatsKHR(device, g_vulkan.surface, &formatCount, nullptr);

    if (formatCount != 0)
    {
        details.formats.resize(formatCount);
        vkGetPhysicalDeviceSurfaceFormatsKHR(device, g_vulkan.surface, &formatCount, details.formats.data());
    }

    uint32_t presentModeCount = 0;
    vkGetPhysicalDeviceSurfacePresentModesKHR(device, g_vulkan.surface, &presentModeCount, nullptr);

    if (presentModeCount != 0)
    {
        details.presentModes.resize(presentModeCount);
        vkGetPhysicalDeviceSurfacePresentModesKHR(device, g_vulkan.surface, &presentModeCount, details.presentModes.data());
    }

    return details;
}

VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats)
{
    const int          acceptableFormatListCount                       = 2;
    VkSurfaceFormatKHR acceptableFormatList[acceptableFormatListCount] = {
        {VK_FORMAT_R8G8B8A8_UNORM, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR},
        {VK_FORMAT_B8G8R8A8_UNORM, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR},
    };

    for (int i = 0; i < acceptableFormatListCount; i++)
        for (const auto& availableFormat : availableFormats)
            if (availableFormat.format == acceptableFormatList[i].format)
                if (availableFormat.colorSpace == acceptableFormatList[i].colorSpace)
                    return availableFormat;

    return availableFormats[0];
}

VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes)
{
    bool useVSync = true;

    VkPresentModeKHR desiredPresentMode = useVSync ? VK_PRESENT_MODE_FIFO_KHR : VK_PRESENT_MODE_MAILBOX_KHR;

    for (const auto& availablePresentMode : availablePresentModes)
        if (availablePresentMode == desiredPresentMode)
            return availablePresentMode;

    return (availablePresentModes.size() > 0) ? availablePresentModes[0] : VK_PRESENT_MODE_IMMEDIATE_KHR;
}

uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties)
{
    VkPhysicalDeviceMemoryProperties memProperties = {};
    vkGetPhysicalDeviceMemoryProperties(g_vulkan.physicalDevice, &memProperties);

    for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++)
        if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties)
            return i;

    // Failed to find suitable memory type
    LNK_ASSERT(false);
    return 0;
}

VkFormat findSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features)
{
    for (VkFormat format : candidates)
    {
        VkFormatProperties props = {};
        vkGetPhysicalDeviceFormatProperties(g_vulkan.physicalDevice, format, &props);

        if (tiling == VK_IMAGE_TILING_LINEAR && (props.linearTilingFeatures & features) == features)
            return format;

        if (tiling == VK_IMAGE_TILING_OPTIMAL && (props.optimalTilingFeatures & features) == features)
            return format;
    }

    // Failed to find supported format.
    LNK_ASSERT(false);
    return VkFormat::VK_FORMAT_UNDEFINED;
}

VkFormat findDepthFormat()
{
    return findSupportedFormat(
        {VK_FORMAT_D32_SFLOAT, VK_FORMAT_D24_UNORM_S8_UINT, VK_FORMAT_D32_SFLOAT_S8_UINT}, // note with stencil usage, different aspect bits need to be set
        VK_IMAGE_TILING_OPTIMAL,
        VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);
}

bool isDeviceSuitable(VkPhysicalDevice device)
{
    QueueFamilyIndices indices = findQueueFamilies(device);

    bool extensionsSupported = checkDeviceExtensionSupport(device);

    bool swapChainAdequate = false;
    if (extensionsSupported)
    {
        SwapChainSupportDetails swapChainSupport = querySwapChainSupport(device);
        swapChainAdequate                        = !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();
    }

    VkPhysicalDeviceFeatures supportedFeatures;
    vkGetPhysicalDeviceFeatures(device, &supportedFeatures);

    return indices.isComplete() && extensionsSupported && swapChainAdequate && supportedFeatures.samplerAnisotropy;
}

void createImage(uint32_t              width,
                 uint32_t              height,
                 VkFormat              format,
                 VkImageTiling         tiling,
                 VkImageUsageFlags     usage,
                 VkMemoryPropertyFlags properties,
                 VkImage&              image,
                 VkDeviceMemory&       imageMemory)
{
    VkImageCreateInfo imageInfo = {};
    imageInfo.sType             = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageInfo.imageType         = VK_IMAGE_TYPE_2D;
    imageInfo.extent.width      = width;
    imageInfo.extent.height     = height;
    imageInfo.extent.depth      = 1;
    imageInfo.mipLevels         = 1;
    imageInfo.arrayLayers       = 1;
    imageInfo.format            = format;
    imageInfo.tiling            = tiling;
    imageInfo.initialLayout     = VK_IMAGE_LAYOUT_UNDEFINED;
    imageInfo.usage             = usage;
    imageInfo.samples           = VK_SAMPLE_COUNT_1_BIT;
    imageInfo.sharingMode       = VK_SHARING_MODE_EXCLUSIVE;

    if (vkCreateImage(g_vulkan.device, &imageInfo, nullptr, &image) != VK_SUCCESS)
    {
        // Failed to create image
        LNK_ASSERT(false);
    }

    VkMemoryRequirements memRequirements = {};
    vkGetImageMemoryRequirements(g_vulkan.device, image, &memRequirements);

    VkMemoryAllocateInfo allocInfo = {};
    allocInfo.sType                = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize       = memRequirements.size;
    allocInfo.memoryTypeIndex      = findMemoryType(memRequirements.memoryTypeBits, properties);

    if (vkAllocateMemory(g_vulkan.device, &allocInfo, nullptr, &imageMemory) != VK_SUCCESS)
    {
        // Failed to allocate image memory
        LNK_ASSERT(false);
    }

    vkBindImageMemory(g_vulkan.device, image, imageMemory, 0);
}

VkImageView createImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags)
{
    VkImageViewCreateInfo viewInfo           = {};
    viewInfo.sType                           = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    viewInfo.image                           = image;
    viewInfo.viewType                        = VK_IMAGE_VIEW_TYPE_2D;
    viewInfo.format                          = format;
    viewInfo.subresourceRange.aspectMask     = aspectFlags;
    viewInfo.subresourceRange.baseMipLevel   = 0;
    viewInfo.subresourceRange.levelCount     = 1;
    viewInfo.subresourceRange.baseArrayLayer = 0;
    viewInfo.subresourceRange.layerCount     = 1;

    VkImageView imageView = nullptr;
    if (vkCreateImageView(g_vulkan.device, &viewInfo, nullptr, &imageView) != VK_SUCCESS)
    {
        // Failed to create texture image.
        LNK_ASSERT(false);
    }

    return imageView;
}

void InitializeVulkan_CreateFrameBuffers()
{
    g_vulkan.swapChainFramebuffers.resize(g_vulkan.swapChainImageViews.size());

    for (size_t i = 0; i < g_vulkan.swapChainImageViews.size(); i++)
    {
        VkImageView attachments[2] = {g_vulkan.swapChainImageViews[i], g_vulkan.depthImageView};

        VkFramebufferCreateInfo framebufferInfo{};
        framebufferInfo.sType           = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebufferInfo.renderPass      = g_vulkan.renderPass;
        framebufferInfo.attachmentCount = 2;
        framebufferInfo.pAttachments    = attachments;
        framebufferInfo.width           = g_vulkan.swapChainExtent.width;
        framebufferInfo.height          = g_vulkan.swapChainExtent.height;
        framebufferInfo.layers          = 1;

        if (vkCreateFramebuffer(g_vulkan.device, &framebufferInfo, nullptr, &g_vulkan.swapChainFramebuffers[i]) != VK_SUCCESS)
        {
            // Failed to create framebuffer.
            LNK_ASSERT(false);
        }
    }
}

void InitializeVulkan_CreateDepthResources()
{
    VkFormat depthFormat = findDepthFormat();

    createImage(g_vulkan.swapChainExtent.width,
                g_vulkan.swapChainExtent.height,
                depthFormat,
                VK_IMAGE_TILING_OPTIMAL,
                VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT,
                VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                g_vulkan.depthImage,
                g_vulkan.depthImageMemory);

    g_vulkan.depthImageView = createImageView(g_vulkan.depthImage, depthFormat, VK_IMAGE_ASPECT_DEPTH_BIT);
}

void InitializeVulkan_CreateImageViews()
{
    g_vulkan.swapChainImageViews.resize(g_vulkan.swapChainImages.size());
    g_vulkan.renderTargetStates.resize(g_vulkan.swapChainImages.size());
    g_vulkan.depthStencilStates.resize(g_vulkan.swapChainImages.size());

    for (uint32_t i = 0; i < g_vulkan.swapChainImages.size(); i++)
    {
        g_vulkan.swapChainImageViews[i] = createImageView(g_vulkan.swapChainImages[i], g_vulkan.renderTargetFormat, VK_IMAGE_ASPECT_COLOR_BIT);
        g_vulkan.renderTargetStates[i]  = VK_IMAGE_LAYOUT_UNDEFINED;
        g_vulkan.depthStencilStates[i]  = VK_IMAGE_LAYOUT_UNDEFINED;
    }
}

#    if defined(LEIA_OS_WINDOWS)
void InitializeVulkan_CreateSwapChain(HWND window)
#    else
void InitializeVulkan_CreateSwapChain(ANativeWindow* window)
#    endif
{
    QueueFamilyIndices indices              = findQueueFamilies(g_vulkan.physicalDevice);
    uint32_t           queueFamilyIndices[] = {indices.graphicsFamily.value(), indices.presentFamily.value()};

    SwapChainSupportDetails swapChainSupport = querySwapChainSupport(g_vulkan.physicalDevice);

    VkSurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(swapChainSupport.formats);
    VkPresentModeKHR   presentMode   = chooseSwapPresentMode(swapChainSupport.presentModes);
    VkExtent2D         extent        = swapChainSupport.capabilities.currentExtent;

    //
    uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;
    if ((swapChainSupport.capabilities.maxImageCount > 0) && (imageCount > swapChainSupport.capabilities.maxImageCount))
        imageCount = swapChainSupport.capabilities.maxImageCount;

    //
    VkSwapchainCreateInfoKHR createInfo = {};
    createInfo.sType                    = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    createInfo.surface                  = g_vulkan.surface;
    createInfo.minImageCount            = imageCount;
    createInfo.imageFormat              = surfaceFormat.format;
    createInfo.imageColorSpace          = surfaceFormat.colorSpace;
    createInfo.imageExtent              = extent;
    createInfo.imageArrayLayers         = 1;
    createInfo.imageUsage               = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    createInfo.preTransform             = swapChainSupport.capabilities.currentTransform;
    createInfo.compositeAlpha           = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    createInfo.presentMode              = presentMode;
    createInfo.clipped                  = VK_TRUE;

    if (indices.graphicsFamily != indices.presentFamily)
    {
        createInfo.imageSharingMode      = VK_SHARING_MODE_CONCURRENT;
        createInfo.queueFamilyIndexCount = 2;
        createInfo.pQueueFamilyIndices   = queueFamilyIndices;
    }
    else
    {
        createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    }

    if (vkCreateSwapchainKHR(g_vulkan.device, &createInfo, nullptr, &g_vulkan.swapChain) != VK_SUCCESS)
    {
        // Failed to create swap chain.
        LNK_ASSERT(false);
    }

    // Get swap chain images.
    vkGetSwapchainImagesKHR(g_vulkan.device, g_vulkan.swapChain, &imageCount, nullptr);
    g_vulkan.swapChainImages.resize(imageCount);
    vkGetSwapchainImagesKHR(g_vulkan.device, g_vulkan.swapChain, &imageCount, g_vulkan.swapChainImages.data());

    //
    g_vulkan.renderTargetFormat = surfaceFormat.format;
    g_vulkan.depthStencilFormat = findDepthFormat();
    g_vulkan.textureFormat      = VK_FORMAT_R8G8B8A8_UNORM;
    g_vulkan.swapChainExtent    = extent;
}

#    if defined(LEIA_OS_WINDOWS)
bool ResizeBuffersVulkan(renderer::IRenderer* renderer, int width, int height, HWND window)
#    else
bool ResizeBuffersVulkan(renderer::IRenderer* renderer, int width, int height, ANativeWindow* window)
#    endif
{
    if (g_vulkan.device == nullptr)
        return false;

    // Wait
    vkDeviceWaitIdle(g_vulkan.device);

    // Destroy
    {
        vkDestroyImageView(g_vulkan.device, g_vulkan.depthImageView, nullptr);
        vkDestroyImage(g_vulkan.device, g_vulkan.depthImage, nullptr);
        vkFreeMemory(g_vulkan.device, g_vulkan.depthImageMemory, nullptr);

        for (auto framebuffer : g_vulkan.swapChainFramebuffers)
            vkDestroyFramebuffer(g_vulkan.device, framebuffer, nullptr);

        for (auto imageView : g_vulkan.swapChainImageViews)
            vkDestroyImageView(g_vulkan.device, imageView, nullptr);

        vkDestroySwapchainKHR(g_vulkan.device, g_vulkan.swapChain, nullptr);
    }

    // Create
    InitializeVulkan_CreateSwapChain(window);
    InitializeVulkan_CreateImageViews();
    InitializeVulkan_CreateDepthResources();
    InitializeVulkan_CreateFrameBuffers();

    return true;
}

#    ifdef LEIA_OS_WINDOWS
bool InitializeVulkan(renderer::IRenderer* renderer, HWND window)
#    else
bool InitializeVulkan(renderer::IRenderer* renderer, ANativeWindow* window)
#    endif
{
    if (g_vulkan.enableValidationLayers && !checkValidationLayerSupport())
    {
        // Validation layers requested, but not available.
        LNK_ASSERT(false);
        return false;
    }

    // Set application info.
    VkApplicationInfo appInfo  = {};
    appInfo.sType              = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName   = "LWE_Core";
    appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.pEngineName        = "LWE_Core";
    appInfo.engineVersion      = VK_MAKE_VERSION(1, 0, 0);
    appInfo.apiVersion         = VK_MAKE_VERSION(1, 1, 0);

    // Create instance.
    {
        // Get instance extensions.
        auto extensions = g_requiredExtensions;
        if (g_vulkan.enableValidationLayers)
        {
#    if defined(LEIA_OS_WINDOWS)
            extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
#    elif defined(LEIA_OS_ANDROID)
            extensions.push_back(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);
#    endif
        }

        VkInstanceCreateInfo createInfo    = {};
        createInfo.sType                   = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        createInfo.pApplicationInfo        = &appInfo;
        createInfo.enabledExtensionCount   = static_cast<uint32_t>(extensions.size());
        createInfo.ppEnabledExtensionNames = extensions.data();

#    if defined(LEIA_OS_WINDOWS)
        VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo = {};
        if (g_vulkan.enableValidationLayers)
        {
            createInfo.enabledLayerCount   = static_cast<uint32_t>(g_validationLayers.size());
            createInfo.ppEnabledLayerNames = g_validationLayers.data();

            debugCreateInfo.sType           = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
            debugCreateInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
                VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
            debugCreateInfo.messageType =
                VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
            debugCreateInfo.pfnUserCallback = debugCallback;

            createInfo.pNext = &debugCreateInfo;
        }
#    elif defined(LEIA_OS_ANDROID)
        VkDebugReportCallbackCreateInfoEXT debugCreateInfo = {};
        if (g_vulkan.enableValidationLayers)
        {
            createInfo.enabledLayerCount   = static_cast<uint32_t>(g_validationLayers.size());
            createInfo.ppEnabledLayerNames = g_validationLayers.data();

            debugCreateInfo.sType       = VK_STRUCTURE_TYPE_DEBUG_REPORT_CREATE_INFO_EXT;
            debugCreateInfo.flags       = VK_DEBUG_REPORT_ERROR_BIT_EXT | VK_DEBUG_REPORT_WARNING_BIT_EXT;
            debugCreateInfo.pfnCallback = debugCallback;

            createInfo.pNext = &debugCreateInfo;
        }
#    endif

        if (vkCreateInstance(&createInfo, nullptr, &g_vulkan.instance) != VK_SUCCESS)
        {
            // Failed to create instance.
            LNK_ASSERT(false);
            return false;
        }

        if (g_vulkan.enableValidationLayers)
        {
#    if defined(LEIA_OS_WINDOWS)

            if (CreateDebugUtilsMessengerEXT(g_vulkan.instance, &debugCreateInfo, nullptr, &g_vulkan.debugMessenger) != VK_SUCCESS)
            {
                // Failed to setup debug messenger.
                LNK_ASSERT(false);
                return false;
            }
#    elif defined(LEIA_OS_ANDROID)

            if (CreateDebugReportCallbackEXT(g_vulkan.instance, &debugCreateInfo, nullptr, &g_vulkan.debugCallback) != VK_SUCCESS)
            {
                // Failed to setup debug callback.
                LNK_ASSERT(false);
                return false;
            }
#    endif
        }
    }

    // Create surface
    {
#    if defined(LEIA_OS_WINDOWS)
        VkWin32SurfaceCreateInfoKHR sci = {};
        sci.sType                       = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
        sci.hinstance                   = GetModuleHandle(NULL);
        sci.hwnd                        = window;

        VkResult ret = vkCreateWin32SurfaceKHR(g_vulkan.instance, &sci, nullptr, &g_vulkan.surface);
        if (ret != 0)
        {
            // Failed to create surface.
            LNK_ASSERT(false);
            return false;
        }
#    elif defined(LEIA_OS_ANDROID)
        VkAndroidSurfaceCreateInfoKHR sci = {};
        sci.sType = VK_STRUCTURE_TYPE_ANDROID_SURFACE_CREATE_INFO_KHR, sci.window = window;

        VkResult ret = vkCreateAndroidSurfaceKHR(g_vulkan.instance, &sci, nullptr, &g_vulkan.surface);
        if (ret != 0)
        {
            // Failed to create surface.
            LNK_ASSERT(false);
            return false;
        }
#    else
        LNK_ASSERT(false);
#    endif
    }

    // Pick physical device.
    {
        // Check physical devices count.
        uint32_t deviceCount = 0;
        vkEnumeratePhysicalDevices(g_vulkan.instance, &deviceCount, nullptr);
        if (deviceCount == 0)
        {
            // No GPUs have Vulkan support.
            LNK_ASSERT(false);
            return false;
        }

        // Get all physical devices.
        std::vector<VkPhysicalDevice> devices(deviceCount);
        vkEnumeratePhysicalDevices(g_vulkan.instance, &deviceCount, devices.data());

        // Select the first "suitable" physical device.
        for (const auto& device : devices)
        {
            if (isDeviceSuitable(device))
            {
                g_vulkan.physicalDevice = device;
                break;
            }
        }

        // Ensure we found something.
        if (g_vulkan.physicalDevice == VK_NULL_HANDLE)
        {
            // Failed to find suitable GPU.
            LNK_ASSERT(false);
            return false;
        }
    }

    // Get queue indices
    {
        QueueFamilyIndices indices  = findQueueFamilies(g_vulkan.physicalDevice);
        g_vulkan.graphicsQueueIndex = indices.graphicsFamily.value();
        g_vulkan.presentQueueIndex  = indices.presentFamily.value();
    }

    // Create logical device
    {
        std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
        std::set<int>                        uniqueQueueFamilies = {g_vulkan.graphicsQueueIndex, g_vulkan.presentQueueIndex};

        float queuePriority = 1.0f;
        for (int queueFamily : uniqueQueueFamilies)
        {
            VkDeviceQueueCreateInfo queueCreateInfo = {};
            queueCreateInfo.sType                   = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
            queueCreateInfo.queueFamilyIndex        = queueFamily;
            queueCreateInfo.queueCount              = 1;
            queueCreateInfo.pQueuePriorities        = &queuePriority;
            queueCreateInfos.push_back(queueCreateInfo);
        }

        VkPhysicalDeviceFeatures deviceFeatures = {};
        deviceFeatures.samplerAnisotropy        = VK_TRUE;

        VkDeviceCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;

        createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
        createInfo.pQueueCreateInfos    = queueCreateInfos.data();

        createInfo.pEnabledFeatures = &deviceFeatures;

        createInfo.enabledExtensionCount   = static_cast<uint32_t>(g_deviceExtensions.size());
        createInfo.ppEnabledExtensionNames = g_deviceExtensions.data();

        if (g_vulkan.enableValidationLayers)
        {
            createInfo.enabledLayerCount   = static_cast<uint32_t>(g_validationLayers.size());
            createInfo.ppEnabledLayerNames = g_validationLayers.data();
        }
        else
        {
            createInfo.enabledLayerCount = 0;
        }

        if (vkCreateDevice(g_vulkan.physicalDevice, &createInfo, nullptr, &g_vulkan.device) != VK_SUCCESS)
        {
            // Failed to create logical device.
            LNK_ASSERT(false);
            return false;
        }
    }

    // Get device queues.
    {
        vkGetDeviceQueue(g_vulkan.device, g_vulkan.graphicsQueueIndex, 0, &g_vulkan.graphicsQueue);
        vkGetDeviceQueue(g_vulkan.device, g_vulkan.presentQueueIndex, 0, &g_vulkan.presentQueue);
    }

    InitializeVulkan_CreateSwapChain(window);
    InitializeVulkan_CreateImageViews();

    // Create render pass
    {
        VkAttachmentDescription colorAttachment = {};
        colorAttachment.format                  = g_vulkan.renderTargetFormat;
        colorAttachment.samples                 = VK_SAMPLE_COUNT_1_BIT;
        colorAttachment.loadOp                  = VK_ATTACHMENT_LOAD_OP_LOAD;
        colorAttachment.storeOp                 = VK_ATTACHMENT_STORE_OP_STORE;
        colorAttachment.stencilLoadOp           = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        colorAttachment.stencilStoreOp          = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        colorAttachment.initialLayout           = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        colorAttachment.finalLayout             = colorAttachment.initialLayout;

        VkAttachmentDescription depthAttachment = {};
        depthAttachment.format                  = g_vulkan.depthStencilFormat;
        depthAttachment.samples                 = VK_SAMPLE_COUNT_1_BIT;
        depthAttachment.loadOp                  = VK_ATTACHMENT_LOAD_OP_LOAD;
        depthAttachment.storeOp                 = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        depthAttachment.stencilLoadOp           = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        depthAttachment.stencilStoreOp          = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        depthAttachment.initialLayout           = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
        depthAttachment.finalLayout             = depthAttachment.initialLayout;

        VkAttachmentReference colorAttachmentRef = {};
        colorAttachmentRef.attachment            = 0;
        colorAttachmentRef.layout                = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

        VkAttachmentReference depthAttachmentRef = {};
        depthAttachmentRef.attachment            = 1;
        depthAttachmentRef.layout                = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

        VkSubpassDescription subpass    = {};
        subpass.pipelineBindPoint       = VK_PIPELINE_BIND_POINT_GRAPHICS;
        subpass.colorAttachmentCount    = 1;
        subpass.pColorAttachments       = &colorAttachmentRef;
        subpass.pDepthStencilAttachment = &depthAttachmentRef;

        VkSubpassDependency dependency = {};
        dependency.srcSubpass          = VK_SUBPASS_EXTERNAL;
        dependency.dstSubpass          = 0;
        dependency.srcStageMask        = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
        dependency.srcAccessMask       = 0;
        dependency.dstStageMask        = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
        dependency.dstAccessMask       = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

        VkAttachmentDescription attachments[2] = {colorAttachment, depthAttachment};

        VkRenderPassCreateInfo renderPassInfo = {};
        renderPassInfo.sType                  = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
        renderPassInfo.attachmentCount        = 2;
        renderPassInfo.pAttachments           = attachments;
        renderPassInfo.subpassCount           = 1;
        renderPassInfo.pSubpasses             = &subpass;
        renderPassInfo.dependencyCount        = 1;
        renderPassInfo.pDependencies          = &dependency;

        if (vkCreateRenderPass(g_vulkan.device, &renderPassInfo, nullptr, &g_vulkan.renderPass) != VK_SUCCESS)
        {
            // Failed to create render pass.
            LNK_ASSERT(false);
            return false;
        }
    }

    // Create command pool
    {
        QueueFamilyIndices queueFamilyIndices = findQueueFamilies(g_vulkan.physicalDevice);

        VkCommandPoolCreateInfo poolInfo{};
        poolInfo.sType            = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        poolInfo.flags            = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
        poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily.value();

        if (vkCreateCommandPool(g_vulkan.device, &poolInfo, nullptr, &g_vulkan.commandPool) != VK_SUCCESS)
        {
            // Failed to create command pool.
            LNK_ASSERT(false);
            return false;
        }
    }

    // Create command buffers
    {
        g_vulkan.commandBuffers.resize(MAX_FRAMES_IN_FLIGHT);

        VkCommandBufferAllocateInfo allocInfo{};
        allocInfo.sType              = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.commandPool        = g_vulkan.commandPool;
        allocInfo.level              = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocInfo.commandBufferCount = (uint32_t)g_vulkan.commandBuffers.size();

        if (vkAllocateCommandBuffers(g_vulkan.device, &allocInfo, g_vulkan.commandBuffers.data()) != VK_SUCCESS)
        {
            // Failed to allocate command buffer.
            LNK_ASSERT(false);
            return false;
        }
    }

    // Create GUI command buffer
    {
        VkCommandBufferAllocateInfo allocInfo = {};
        allocInfo.sType                       = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.commandPool                 = g_vulkan.commandPool;
        allocInfo.level                       = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocInfo.commandBufferCount          = 1;

        if (vkAllocateCommandBuffers(g_vulkan.device, &allocInfo, &g_vulkan.guiCommandBuffer) != VK_SUCCESS)
        {
            // Failed to allocate command buffer.
            LNK_ASSERT(false);
            return false;
        }
    }

    // Create GUI descriptor pool.
    {
        VkDescriptorPoolSize poolSizes[] = {{VK_DESCRIPTOR_TYPE_SAMPLER, 16},
                                            {VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 16},
                                            {VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 16},
                                            {VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 16},
                                            {VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 16},
                                            {VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 16},
                                            {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 16},
                                            {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 16},
                                            {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 16},
                                            {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 16},
                                            {VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 16}};

        VkDescriptorPoolCreateInfo poolInfo = {};
        poolInfo.sType                      = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        poolInfo.flags                      = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
        poolInfo.maxSets                    = 16;
        poolInfo.poolSizeCount              = (uint32_t)std::size(poolSizes);
        poolInfo.pPoolSizes                 = poolSizes;

        if (vkCreateDescriptorPool(g_vulkan.device, &poolInfo, nullptr, &g_vulkan.guiDescriptorPool) != VK_SUCCESS)
        {
            // Failed to create descriptor pool
            LNK_ASSERT(false);
            return false;
        }
    }

    InitializeVulkan_CreateDepthResources();

    InitializeVulkan_CreateFrameBuffers();

    // Create synchronization objects.
    {
        g_vulkan.imageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
        g_vulkan.renderFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
        g_vulkan.readyToPresentSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
        g_vulkan.inFlightFences.resize(MAX_FRAMES_IN_FLIGHT);

        VkSemaphoreCreateInfo semaphoreInfo{};
        semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

        VkFenceCreateInfo fenceInfo{};
        fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

        for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
        {
            if (vkCreateSemaphore(g_vulkan.device, &semaphoreInfo, nullptr, &g_vulkan.imageAvailableSemaphores[i]) != VK_SUCCESS)
            {
                // Failed to create semaphore
                LNK_ASSERT(false);
                return false;
            }

            if (vkCreateSemaphore(g_vulkan.device, &semaphoreInfo, nullptr, &g_vulkan.renderFinishedSemaphores[i]) != VK_SUCCESS)
            {
                // Failed to create semaphore
                LNK_ASSERT(false);
                return false;
            }

            if (vkCreateSemaphore(g_vulkan.device, &semaphoreInfo, nullptr, &g_vulkan.readyToPresentSemaphores[i]) != VK_SUCCESS)
            {
                // Failed to create semaphore
                LNK_ASSERT(false);
                return false;
            }

            if (vkCreateFence(g_vulkan.device, &fenceInfo, nullptr, &g_vulkan.inFlightFences[i]) != VK_SUCCESS)
            {
                // Failed to create fence
                LNK_ASSERT(false);
                return false;
            }
        }
    }

    return true;
}

#endif

LWE_Core::LWE_Core(GraphicsAPI graphicsAPI)
{
    this->graphicsAPI = graphicsAPI;

    leia_platform_on_library_load();
}

LWE_Core::~LWE_Core()
{
    leia_platform_on_library_unload();
}

bool LWE_Core::IsInitialized() const
{
    return isInitialized;
}

bool LWE_Core::IsUsingLeiaSDK() const
{
    return useLeiaSDK;
}

HWND LWE_Core::Initialize(LWE_CoreInitArgs& initArgs)
{
    WglContext = initArgs.appGLContext;

    sdk::CoreInitConfiguration& config = initArgs.coreConfig;

    config.SetFaceTrackingServerLogLevel(kLeiaLogLevelTrace);
    config.SetFaceTrackingEnable(true);

    pLeiaSDK = std::make_unique<sdk::Core>(config);

    customGui     = initArgs.customGui;
    androidWindow = initArgs.window.androidWindow;

#if defined(LEIA_OS_WINDOWS)
    hWnd = CreateGraphicsWindow(initArgs.window);
#endif

    if (initArgs.window.fullscreen)
        SetFullscreen(true);

    return hWnd;
}

void LWE_Core::RenderCustomGui()
{
    // XXX: disabled until we figure out the correct way of protecting data access using mutex.
    // Right now, changing shader debug mode here would cause deadlock due to double lock of the same mutex.
    /*if (ImGui::Button("Shader Debug Mode")) {
            ImGui::OpenPopup("shaderMode");
    }
    if (ImGui::BeginPopup("shaderMode")) {
            for (int i = 0; i < ToIntegral(eLeiaShaderDebugMode::COUNT); ++i) {
                    auto shaderDebugMode = FromIntegral<eLeiaShaderDebugMode>(i);
                    if (ImGui::Button(ToUiStr(shaderDebugMode))) {
                            pLeiaInterlacer->SetShaderDebugMode(shaderDebugMode);
                            ImGui::CloseCurrentPopup();
                    }
            }
            ImGui::EndPopup();
    }*/

    /*if (leia::sdk::ML* ml = pLeiaSDK->GetML())
    {
        guiInputTextBuffer[0] = '\0';
        ImGui::Spacing();
        if (ImGui::CollapsingHeader("ML"))
        {
            // TODO: Implement image picker
#if defined(LEIA_OS_ANDROID)
#    ifndef _LWE_STANDALONE
            fs::path testImagePath{"/data/user/0/com.leia.lwe.sample/cache/beerglass.jpg"};
            if (fs::exists(testImagePath))
            {
                if (ImGui::Button("Test"))
                {
                    ml->ConvertAsync(testImagePath, {2}, [this](std::unique_ptr<sdk::MLImage> albedo, std::unique_ptr<sdk::MLImage>) {
                        std::lock_guard lock(mlImageMutex);
                        std::swap(mlImage, albedo);
                    });
                }
            }
#    endif
#else
            if (ImGui::InputText("Path", guiInputTextBuffer, sizeof(guiInputTextBuffer), ImGuiInputTextFlags_EnterReturnsTrue))
            {
                ml->ConvertAsync(guiInputTextBuffer, {2}, [this](std::unique_ptr<sdk::MLImage> albedo, std::unique_ptr<sdk::MLImage>) {
                    std::lock_guard<std::mutex> lock(mlImageMutex);
                    std::swap(mlImage, albedo);
                });
            }
#endif
        }
    }*/
    if (customGui)
    {
        ImGui::Spacing();
        customGui();
    }
}

bool LWE_Core::DoInternalInit()
{
    if (isInitialized)
    {
        return true;
    }

    if (!pLeiaSDK || !pLeiaSDK->IsInitialized())
    {
        return false;
    }

    {
        device::Config* config = pLeiaSDK->GetDeviceConfig();
        viewResolution[0]      = config->viewResolution[0];
        viewResolution[1]      = config->viewResolution[1];
        pLeiaSDK->ReleaseDeviceConfig(config);
    }

#ifndef _LWE_STANDALONE
    ZoneScoped;
#endif

    InterlacerInitConfiguration interlacerInitArgs;
    interlacerInitArgs.SetUseAtlasForViews(useTextureAtlas);

    if (graphicsAPI == LEIA_GRAPHICS_API_OPENGL)
        renderer = renderer::CreateRenderer(renderer::eGraphicsAPI::OpenGL);
    else if (graphicsAPI == LEIA_GRAPHICS_API_D3D11)
        renderer = renderer::CreateRenderer(renderer::eGraphicsAPI::D3D11);
    else if (graphicsAPI == LEIA_GRAPHICS_API_D3D12)
        renderer = renderer::CreateRenderer(renderer::eGraphicsAPI::D3D12);
    else if (graphicsAPI == LEIA_GRAPHICS_API_VULKAN)
        renderer = renderer::CreateRenderer(renderer::eGraphicsAPI::Vulkan);
    else
    {
        LNK_ASSERT(false);
    }

    if (graphicsAPI == LEIA_GRAPHICS_API_D3D11)
    {
#if defined(LEIA_USE_DIRECTX)
        HRESULT hr = InitializeD3D11(renderer, hWnd);
        if (FAILED(hr))
        {
            LNK_ASSERT(false);
            return NULL;
        }
#endif
    }
    else if (graphicsAPI == LEIA_GRAPHICS_API_D3D12)
    {
#if defined(LEIA_USE_DIRECTX12)
        HRESULT hr = InitializeD3D12(renderer, hWnd);
        if (FAILED(hr))
        {
            LNK_ASSERT(false);
            return NULL;
        }
#endif
    }
    else if (graphicsAPI == LEIA_GRAPHICS_API_VULKAN)
    {
#if defined(LEIA_USE_VULKAN)
#    if defined(LEIA_OS_WINDOWS)
        bool initOk = InitializeVulkan(renderer, hWnd);
#    else
        bool initOk = InitializeVulkan(renderer, androidWindow);
#    endif
        if (!initOk)
        {
            LNK_ASSERT(false);
            return NULL;
        }
#endif
    }

    pLeiaSDK->SetBacklight(true);

    InterlacerDebugMenuConfiguration debugMenu = {};

    debugMenu.userData  = this;
    debugMenu.customGui = [](void* userData) {
        static_cast<LWE_Core*>(userData)->RenderCustomGui();
    };
#if defined(LEIA_OS_WINDOWS)
    debugMenu.gui.surface = hWnd;
#endif

    debugMenu.gui.graphicsAPI = graphicsAPI;

    if (graphicsAPI == LEIA_GRAPHICS_API_D3D11)
    {
#if defined(LEIA_USE_DIRECTX)
        debugMenu.gui.d3d11.device        = g_d3d11.device;
        debugMenu.gui.d3d11.deviceContext = g_d3d11.immediateContext;
#endif
    }
    else if (graphicsAPI == LEIA_GRAPHICS_API_D3D12)
    {
#if defined(LEIA_USE_DIRECTX12)
        debugMenu.gui.d3d12.device               = g_d3d12.device;
        debugMenu.gui.d3d12.deviceCbvSrvHeap     = g_d3d12.srvHeap;
        debugMenu.gui.d3d12.fontSrvCpuDescHandle = *((uint64_t*)&g_d3d12.srvFontCpuDescHandle);
        debugMenu.gui.d3d12.fontSrvGpuDescHandle = *((uint64_t*)&g_d3d12.srvFontGpuDescHandle);
        debugMenu.gui.d3d12.numFramesInFlight    = g_frameCount;
        debugMenu.gui.d3d12.rtvFormat            = g_d3d12.swapChainFormat;
        debugMenu.gui.d3d12.commandList          = g_d3d12.guiCommandList;
#endif
    }
    else if (graphicsAPI == LEIA_GRAPHICS_API_VULKAN)
    {
#if defined(LEIA_USE_VULKAN)
        debugMenu.gui.vulkan.commandBuffer  = g_vulkan.guiCommandBuffer;
        debugMenu.gui.vulkan.descriptorPool = g_vulkan.guiDescriptorPool;
        debugMenu.gui.vulkan.commandPool    = g_vulkan.commandPool;
        debugMenu.gui.vulkan.device         = g_vulkan.device;
        debugMenu.gui.vulkan.imageCount     = (uint32_t)g_vulkan.swapChainImages.size();
        debugMenu.gui.vulkan.instance       = g_vulkan.instance;
        debugMenu.gui.vulkan.minImageCount  = 2;
        debugMenu.gui.vulkan.mSAASamples    = 1;
        debugMenu.gui.vulkan.physicalDevice = g_vulkan.physicalDevice;
        debugMenu.gui.vulkan.pipelineCache  = nullptr;
        debugMenu.gui.vulkan.queue          = g_vulkan.graphicsQueue;
        debugMenu.gui.vulkan.queueFamily    = g_vulkan.graphicsQueueIndex;
        debugMenu.gui.vulkan.renderPass     = g_vulkan.renderPass;
        debugMenu.gui.vulkan.subpass        = 0;
#endif
    }

    if (graphicsAPI == LEIA_GRAPHICS_API_OPENGL)
    {
        pLeiaInterlacer = std::make_unique<InterlacerOpenGL>(*pLeiaSDK, interlacerInitArgs, WglContext);
        renderer::AsOpenGL(renderer)->Initialize(WglContext);
    }
    else if (graphicsAPI == LEIA_GRAPHICS_API_D3D11)
    {
#if defined(LEIA_USE_DIRECTX)
        pLeiaInterlacer = std::make_unique<InterlacerD3D11>(*pLeiaSDK, interlacerInitArgs, g_d3d11.immediateContext);
        renderer::AsD3D11(renderer)->Initialize(g_d3d11.immediateContext);
#endif
    }
    else if (graphicsAPI == LEIA_GRAPHICS_API_D3D12)
    {
#if defined(LEIA_USE_DIRECTX12)
        pLeiaInterlacer = std::make_unique<InterlacerD3D12>(*pLeiaSDK, interlacerInitArgs, g_d3d12.device, g_d3d12.commandQueue);
        renderer::AsD3D12(renderer)->Initialize(g_d3d12.device, g_d3d12.commandQueue);
#endif
    }
    else if (graphicsAPI == LEIA_GRAPHICS_API_VULKAN)
    {
#if defined(LEIA_USE_VULKAN)
        pLeiaInterlacer = std::make_unique<InterlacerVulkan>(*pLeiaSDK,
                                                             interlacerInitArgs,
                                                             g_vulkan.device,
                                                             g_vulkan.physicalDevice,
                                                             g_vulkan.textureFormat,
                                                             g_vulkan.renderTargetFormat,
                                                             g_vulkan.depthStencilFormat,
                                                             MAX_FRAMES_IN_FLIGHT);
        renderer::AsVulkan(renderer)->Initialize(
            g_vulkan.device, g_vulkan.physicalDevice, g_vulkan.textureFormat, g_vulkan.renderTargetFormat, g_vulkan.depthStencilFormat, MAX_FRAMES_IN_FLIGHT);
#endif
    }
    else
    {
        LNK_ASSERT(false);
    }

    pLeiaInterlacer->SetConvergenceDistance(initialConvergenceDistance);
    pLeiaInterlacer->InitializeGui(&debugMenu);

    CreateResources();
    OnWindowSizeChanged(windowWidth, windowHeight);

    isInitialized = true;
    return true;
}

void LWE_Core::Tick(double deltaTime)
{
    if (!DoInternalInit())
    {
        return;
    }

    if (animationEnabled)
    {
        // For now we're just rotating every object
        float yRot = 0.2f * (float)deltaTime;
        float zRot = 0.3f * (float)deltaTime;
        float xRot = 0.4f * (float)deltaTime;

        // Rotate geometry.
        for (size_t i = 0; i < geometry.size(); i++)
            geometry[i]->Rotate(xRot, yRot, zRot);
    }

#if 0
	static double backlightTimer = 0.0;
	static int prevBacklightStep = 0;
	backlightTimer += deltaTime;
	int backlightStep = int(backlightTimer / 2.0);
	if (prevBacklightStep != backlightStep) {
		prevBacklightStep = backlightStep;
		pLeiaSDK->SetBacklight(prevBacklightStep % 2 == 0);
	}
#endif
}

void LWE_Core::OnWindowSizeChanged(int width, int height)
{
    bool changed = (windowWidth != width) || (windowHeight != height);
    if (!changed)
        return;

    windowWidth  = width;
    windowHeight = height;

    if (!pLeiaInterlacer)
    {
        return;
    }

    if (renderer != nullptr)
        renderer->OnWindowSizeChanged(width, height);

    if (graphicsAPI == LEIA_GRAPHICS_API_D3D11)
    {
#if defined(LEIA_USE_DIRECTX)
        ResizeBuffersD3D11(renderer, width, height);
#endif
    }
    else if (graphicsAPI == LEIA_GRAPHICS_API_D3D12)
    {
#if defined(LEIA_USE_DIRECTX12)
        ResizeBuffersD3D12(renderer, width, height);
#endif
    }
    else if (graphicsAPI == LEIA_GRAPHICS_API_VULKAN)
    {
#if defined(LEIA_USE_VULKAN)
#    if defined(LEIA_OS_WINDOWS)
        ResizeBuffersVulkan(renderer, width, height, hWnd);
#    elif defined(LEIA_OS_ANDROID)
        ResizeBuffersVulkan(renderer, width, height, androidWindow);
#    else
        LNK_ASSERT(false);
#    endif
#endif
    }
}

void LWE_Core::Shutdown()
{
    renderer->Destroy();
    renderer    = nullptr;
    modelShader = nullptr;
    geometryRenderState.clear();
    geometry.clear();

    pLeiaInterlacer = nullptr;
    pLeiaSDK        = nullptr;

    isInitialized = false;
}

void LWE_Core::OnPause()
{
    pLeiaSDK->OnPause();
}

void LWE_Core::OnResume()
{
    pLeiaSDK->OnResume();
}

void LWE_Core::Render(HDC hDC)
{
    if (!isInitialized)
    {
        return;
    }

    // Don't render when window doesn't have a size (which may indicate a minimized window).
    if ((windowWidth <= 0) || (windowHeight <= 0))
        return;

#ifndef _LWE_STANDALONE
    FrameMark;
#endif

#if defined(LEIA_USE_VULKAN)
    renderer->BeginFrame(g_vulkan.currentFrame);
#endif // LEIA_USE_VULKAN

    // Get backbuffer.
    renderer::ITexture* pBackBuffer      = nullptr;
    uint32_t            vulkanImageIndex = 0;
    if (graphicsAPI == LEIA_GRAPHICS_API_OPENGL)
    {
        // Use null backbuffer in GL
    }
    else if (graphicsAPI == LEIA_GRAPHICS_API_D3D11)
    {
#if defined(LEIA_USE_DIRECTX)
        pBackBuffer = renderer::AsD3D11(renderer)->CreateTextureAlias(
            nullptr, nullptr, g_d3d11.renderTargetView, nullptr, nullptr, 0, 0, 1, renderer::ePixelFormat::RGBA, renderer::eResourceUsageFlags::RenderTarget);
#endif
    }
    else if (graphicsAPI == LEIA_GRAPHICS_API_D3D12)
    {
#if defined(LEIA_USE_DIRECTX12)
        g_d3d12.frameIndex = g_d3d12.swapChain->GetCurrentBackBufferIndex();

        // Lazy create backbuffer alias.
        if (g_d3d12.backBuffer[g_d3d12.frameIndex] == nullptr)
        {
            D3D12_RESOURCE_STATES renderTargetState = D3D12_RESOURCE_STATE_COMMON;
            D3D12_RESOURCE_STATES depthStencilState = D3D12_RESOURCE_STATE_DEPTH_WRITE;

            g_d3d12.backBuffer[g_d3d12.frameIndex] = renderer::AsD3D12(renderer)->CreateTextureAlias(
                g_d3d12.renderTargets[g_d3d12.frameIndex],
                &renderTargetState,
                g_d3d12.depthStencil[g_d3d12.frameIndex],
                &depthStencilState,
                0,
                0,
                1,
                renderer::ePixelFormat::RGBA,
                renderer::eResourceUsageFlags::ShaderInput | renderer::eResourceUsageFlags::RenderTarget | renderer::eResourceUsageFlags::DepthStencil);
        }

        pBackBuffer = g_d3d12.backBuffer[g_d3d12.frameIndex];
#endif
    }
    else if (graphicsAPI == LEIA_GRAPHICS_API_VULKAN)
    {
#if defined(LEIA_USE_VULKAN)
        vkWaitForFences(g_vulkan.device, 1, &g_vulkan.inFlightFences[g_vulkan.currentFrame], VK_TRUE, UINT64_MAX);

        VkResult result = vkAcquireNextImageKHR(
            g_vulkan.device, g_vulkan.swapChain, UINT64_MAX, g_vulkan.imageAvailableSemaphores[g_vulkan.currentFrame], VK_NULL_HANDLE, &vulkanImageIndex);

        if (result == VK_ERROR_OUT_OF_DATE_KHR)
        {
            LNK_ASSERT(false);
            // recreateSwapChain();
            // return;
        }
        else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
        {
            LNK_ASSERT(false);
            // throw std::runtime_error("failed to acquire swap chain image!");
        }

        // updateUniformBuffer(currentFrame);

        vkResetFences(g_vulkan.device, 1, &g_vulkan.inFlightFences[g_vulkan.currentFrame]);

        vkResetCommandBuffer(g_vulkan.commandBuffers[g_vulkan.currentFrame], /*VkCommandBufferResetFlagBits*/ 0);

        // Here we create the "backbuffer" which can contain the rendertarget and depthbuffer.
        pBackBuffer = renderer::AsVulkan(renderer)->CreateTextureAlias(
            g_vulkan.swapChainFramebuffers[vulkanImageIndex],
            g_vulkan.swapChainImages[vulkanImageIndex],
            g_vulkan.renderTargetStates[vulkanImageIndex],
            g_vulkan.renderTargetFormat,
            g_vulkan.depthImage,
            g_vulkan.depthStencilStates[vulkanImageIndex],
            0,
            0,
            1,
            renderer::ePixelFormat::RGBA,
            renderer::eResourceUsageFlags::ShaderInput | renderer::eResourceUsageFlags::RenderTarget | renderer::eResourceUsageFlags::DepthStencil);

        if (g_vulkan.renderTargetStates[vulkanImageIndex] == VK_IMAGE_LAYOUT_UNDEFINED)
            g_vulkan.renderTargetStates[vulkanImageIndex] = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        if (g_vulkan.depthStencilStates[vulkanImageIndex] == VK_IMAGE_LAYOUT_UNDEFINED)
            g_vulkan.depthStencilStates[vulkanImageIndex] = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
#endif
    }
    else
    {
        LNK_ASSERT(false);
    }

    // Clear backbuffer to black.

#if defined(LEIA_USE_VULKAN)
    if (graphicsAPI == LEIA_GRAPHICS_API_VULKAN)
    {
        renderer::AsVulkan(renderer)->ClearScene(pBackBuffer, 0.0f, 0.0f, 0.0f, 1.0f, g_vulkan.imageAvailableSemaphores[g_vulkan.currentFrame], nullptr);
    }
    else
#endif // LEIA_USE_VULKAN
    {
        renderer->ClearScene(pBackBuffer, 0.0f, 0.0f, 0.0f, 1.0f);
    }

    renderer::ITexture* mlTexture = nullptr;

    /*if (mlImage)
    {
        std::unique_ptr<MLImage> localMlImage;
        {
            std::lock_guard<std::mutex> lock(mlImageMutex);
            localMlImage = std::move(mlImage);
        }

        if (localMlImage)
        {
            if (mlTexture != nullptr)
            {
                mlTexture->Destroy();
                mlTexture = nullptr;
            }

            leia_sdk_image_desc imageDesc = localMlImage->GetDesc();
            if (imageDesc.data)
            {
                if (imageDesc.format == leia_sdk_image_format::kLeiaSdkImageFormatRGB)
                {
                    mlTexture = renderer->CreateTexture(
                        imageDesc.width, imageDesc.height, 1, renderer::ePixelFormat::RGB, renderer::eResourceUsageFlags::ShaderInput, imageDesc.data);
                }
                else if (imageDesc.format == leia_sdk_image_format::kLeiaSdkImageFormatRGBA)
                {
                    mlTexture = renderer->CreateTexture(
                        imageDesc.width, imageDesc.height, 1, renderer::ePixelFormat::RGBA, renderer::eResourceUsageFlags::ShaderInput, imageDesc.data);
                }
                else
                {
                    LNK_ASSERT(false);
                }
            }
        }
    }*/

    if (!useLeiaSDK)
    {
        // Setup camera to render into backbuffer.
        renderer->SetCameraPosition(renderer::vec3f(gCamera.pos.x, gCamera.pos.y, gCamera.pos.z));
        renderer->SetCameraDirection(renderer::vec3f(gCamera.dir.x, gCamera.dir.y, gCamera.dir.z));
        renderer->SetCameraProjection(glm::radians(FOV), (float)windowWidth / windowHeight, 0.01f, 10000.0f);

        // Render scene into backbuffer.
        for (int i = 0; i < geometryRenderState.size(); i++)
            geometryRenderState[i]->SetViewport(0, 0, windowWidth, windowHeight);
        renderer->RenderScene(pBackBuffer);
    }
    else if (mlTexture != nullptr)
    {
#if defined(LEIA_USE_OPENGL)
        if (InterlacerOpenGL* interlacer = AsOpenGL(pLeiaInterlacer.get()))
        {
            GLuint texId = renderer::AsOpenGL(mlTexture)->GetTexture();
            interlacer->DoPostProcessPicture(windowWidth, windowHeight, texId);
        }
#endif
#if defined(LEIA_USE_DIRECTX)
        if (InterlacerD3D11* interlacer = AsD3D11(pLeiaInterlacer.get()))
        {
            interlacer->DoPostProcessPicture(windowWidth, windowHeight, renderer::AsD3D11(mlTexture)->GetShaderResourceView(), g_d3d11.renderTargetView);
        }
#endif
    }
    else
    {
        renderer::ITexture* renderTargetPtr = nullptr;

        int numViews = pLeiaInterlacer->GetNumViews();
        for (int i = 0; i < numViews; ++i)
        {
            if (!useTextureAtlas || i == 0)
            {
#if defined(LEIA_USE_OPENGL)
                if (auto interlacer = AsOpenGL(pLeiaInterlacer.get()))
                {
                    GLuint renderTarget = interlacer->GetRenderTargetForView(useTextureAtlas ? 0 : i);
                    if (renderTarget != 0)
                        renderTargetPtr = renderer::AsOpenGL(renderer)->CreateTextureAlias(renderTarget,
                                                                                           0,
                                                                                           0,
                                                                                           viewResolution[0],
                                                                                           viewResolution[1],
                                                                                           1,
                                                                                           renderer::ePixelFormat::RGBA,
                                                                                           renderer::eResourceUsageFlags::RenderTarget);
                }
#endif
#if defined(LEIA_USE_DIRECTX)
                if (InterlacerD3D11* interlacer = AsD3D11(pLeiaInterlacer.get()))
                {
                    ID3D11RenderTargetView* renderTargetView = interlacer->GetRenderTargetView(useTextureAtlas ? 0 : i);
                    ID3D11DepthStencilView* depthStencilView = interlacer->GetDepthStencilView(useTextureAtlas ? 0 : i);
                    if (renderTargetView != nullptr)
                        renderTargetPtr = renderer::AsD3D11(renderer)->CreateTextureAlias(nullptr,
                                                                                          nullptr,
                                                                                          renderTargetView,
                                                                                          nullptr,
                                                                                          depthStencilView,
                                                                                          viewResolution[0],
                                                                                          viewResolution[1],
                                                                                          1,
                                                                                          renderer::ePixelFormat::RGBA,
                                                                                          renderer::eResourceUsageFlags::RenderTarget |
                                                                                              renderer::eResourceUsageFlags::DepthStencil);
                }
#endif
#if defined(LEIA_USE_DIRECTX12)
                if (InterlacerD3D12* interlacer = AsD3D12(pLeiaInterlacer.get()))
                {
                    D3D12_RESOURCE_STATES textureResourceState      = {};
                    ID3D12Resource*       textureResource           = interlacer->GetRenderTargetResource(useTextureAtlas ? 0 : i, &textureResourceState);
                    D3D12_RESOURCE_STATES depthTextureResourceState = {};
                    ID3D12Resource*       depthTextureResource      = interlacer->GetDepthStencilResource(useTextureAtlas ? 0 : i, &depthTextureResourceState);

                    renderTargetPtr = renderer::AsD3D12(renderer)->CreateTextureAlias(textureResource,
                                                                                      &textureResourceState,
                                                                                      depthTextureResource,
                                                                                      &depthTextureResourceState,
                                                                                      viewResolution[0],
                                                                                      viewResolution[1],
                                                                                      1,
                                                                                      renderer::ePixelFormat::RGBA,
                                                                                      renderer::eResourceUsageFlags::RenderTarget |
                                                                                          renderer::eResourceUsageFlags::DepthStencil);
                }
#endif
#if defined(LEIA_USE_VULKAN)
                if (InterlacerVulkan* interlacer = AsVulkan(pLeiaInterlacer.get()))
                {

                    VkImageLayoutInt imageBufferLayout      = VK_IMAGE_LAYOUT_UNDEFINED;
                    VkImageLayoutInt depthImageBufferLayout = VK_IMAGE_LAYOUT_UNDEFINED;
                    VkFramebuffer    frameBuffer            = interlacer->GetFramebuffer(useTextureAtlas ? 0 : i);
                    VkImage          imageBuffer            = interlacer->GetRenderTargetImage(useTextureAtlas ? 0 : i, &imageBufferLayout);
                    int              imageBufferWidth       = interlacer->GetRenderTargetImageWidth();
                    int              imageBufferHeight      = interlacer->GetRenderTargetImageHeight();
                    VkImage          depthImageBuffer       = interlacer->GetDepthStencilImage(useTextureAtlas ? 0 : i, &depthImageBufferLayout);

                    renderTargetPtr = renderer::AsVulkan(renderer)->CreateTextureAlias(frameBuffer,
                                                                                       imageBuffer,
                                                                                       imageBufferLayout,
                                                                                       g_vulkan.renderTargetFormat,
                                                                                       depthImageBuffer,
                                                                                       depthImageBufferLayout,
                                                                                       imageBufferWidth,
                                                                                       imageBufferHeight,
                                                                                       1,
                                                                                       renderer::ePixelFormat::RGBA,
                                                                                       renderer::eResourceUsageFlags::RenderTarget |
                                                                                           renderer::eResourceUsageFlags::DepthStencil);
                }
#endif
            }

            if (!useTextureAtlas || i == 0)
                renderer->ClearScene(renderTargetPtr, 0.0f, 0.0f, 0.5f, 1.0f);

            const int       viewWidth  = viewResolution[0];
            const int       viewHeight = viewResolution[1];
            const glm::vec3 camForward = gCamera.getForward();

            // Get camera position and shear.
            glm::vec3 viewCamPos = glm::vec3(0);
            float     shearX     = 0;
            float     shearY     = 0;
            pLeiaInterlacer->GetConvergedPerspectiveViewInfo(i,
                                                             ToConstSlice(gCamera.pos),
                                                             ToConstSlice(camForward),
                                                             ToConstSlice(gCamera.getUp()),
                                                             glm::radians(FOV),
                                                             (float)viewWidth / (float)viewHeight,
                                                             0.01f,
                                                             10000.0f,
                                                             ToSlice(viewCamPos),
                                                             {},
                                                             nullptr,
                                                             &shearX,
                                                             &shearY);

            // Setup camera to render into part of atlas.
            renderer->SetCameraPosition(renderer::vec3f(viewCamPos.x, viewCamPos.y, viewCamPos.z));
            renderer->SetCameraDirection(renderer::vec3f(camForward.x, camForward.y, camForward.z));
            renderer->SetCameraProjection(glm::radians(FOV), (float)viewWidth / (float)viewHeight, 0.01f, 10000.0f, shearX, shearY);

            // Render scene into part of atlas.
            int viewportX = useTextureAtlas ? viewResolution[0] * i : 0;
            for (int j = 0; j < geometryRenderState.size(); j++)
                geometryRenderState[j]->SetViewport(viewportX, 0, viewWidth, viewHeight);
            renderer->RenderScene(renderTargetPtr);

            // Delete render-target when we're finished with it.
            if (!useTextureAtlas || i == (numViews - 1))
                if (renderTargetPtr != nullptr)
                    renderTargetPtr->Destroy();
        }

        if (false)
        {
        }
#if defined(LEIA_USE_OPENGL)
        else if (InterlacerOpenGL* interlacer = AsOpenGL(pLeiaInterlacer.get()))
        {
            interlacer->DoPostProcess(windowWidth, windowHeight, false, (uint32_t)0);
        }
#endif
#if defined(LEIA_USE_DIRECTX)
        else if (InterlacerD3D11* interlacer = AsD3D11(pLeiaInterlacer.get()))
        {
            interlacer->DoPostProcess(windowWidth, windowHeight, false, g_d3d11.renderTargetView);
        }
#endif
#if defined(LEIA_USE_DIRECTX12)
        else if (InterlacerD3D12* interlacer = AsD3D12(pLeiaInterlacer.get()))
        {
            // Prepare GUI command-list.
            {
                // Reset the list.
                g_d3d12.guiCommandList->Reset(g_d3d12.commandAllocator, nullptr);

                // set constant buffer descriptor heap
                ID3D12DescriptorHeap* descriptorHeaps[] = {g_d3d12.srvHeap};
                g_d3d12.guiCommandList->SetDescriptorHeaps(_countof(descriptorHeaps), descriptorHeaps);

                renderer::AsD3D12(pBackBuffer)->TransitionRenderTarget(g_d3d12.guiCommandList);

                // set render-target for GUI (required by imGui)
                g_d3d12.guiCommandList->OMSetRenderTargets(1, &g_d3d12.renderTargetViews[g_d3d12.frameIndex], FALSE, nullptr);
            }

            interlacer->DoPostProcess(windowWidth, windowHeight, false, g_d3d12.renderTargets[g_d3d12.frameIndex]);
        }
#endif
#if defined(LEIA_USE_VULKAN)
        else if (InterlacerVulkan* interlacer = AsVulkan(pLeiaInterlacer.get()))
        {

            // The colorImage parameter of DoPostProcess below must be in VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL already.
            renderer::AsVulkan(renderer)->TransitionRenderTarget(nullptr, g_vulkan.swapChainImages[vulkanImageIndex]);

            interlacer->DoPostProcess(windowWidth,
                                      windowHeight,
                                      false,
                                      g_vulkan.swapChainFramebuffers[vulkanImageIndex],
                                      g_vulkan.swapChainImages[vulkanImageIndex],
                                      g_vulkan.depthImage,
                                      renderer::AsVulkan(renderer)->GetPreviousPassSignalSemaphore(),
                                      g_vulkan.renderFinishedSemaphores[g_vulkan.currentFrame],
                                      g_vulkan.currentFrame);
        }
#endif
        else
        {
            LNK_ASSERT(false);
        }
    }

    renderer->EndFrame();

    if (graphicsAPI == LEIA_GRAPHICS_API_OPENGL)
    {
#if defined(LEIA_USE_OPENGL)
#    if defined(_WIN32)
        renderer::AsOpenGL(renderer)->Present(hDC);
#    elif defined(LEIA_OS_ANDROID)
        renderer::AsOpenGL(renderer)->Present();
#    endif
#endif
    }
    else if (graphicsAPI == LEIA_GRAPHICS_API_D3D11)
    {
#if defined(LEIA_USE_DIRECTX)
        renderer::AsD3D11(renderer)->Present(g_d3d11.swapChain);
#endif
    }
    else if (graphicsAPI == LEIA_GRAPHICS_API_D3D12)
    {
#if defined(LEIA_USE_DIRECTX12)
        // Render GUI.
        {
            renderer::AsD3D12(pBackBuffer)->TransitionPresent(g_d3d12.guiCommandList);

            // Close gui commandlist.
            g_d3d12.guiCommandList->Close();

            // Execute the command list.
            ID3D12CommandList* ppCommandLists[] = {g_d3d12.guiCommandList};
            g_d3d12.commandQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);
        }

        renderer::AsD3D12(renderer)->Present(g_d3d12.swapChain);
#endif
    }
    else if (graphicsAPI == LEIA_GRAPHICS_API_VULKAN)
    {

#if defined(LEIA_USE_VULKAN)

        // Transition swapchain target to be presentable.
        {
            VkCommandBufferBeginInfo beginInfo = {};
            beginInfo.sType                    = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
            beginInfo.flags                    = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
            vkBeginCommandBuffer(g_vulkan.commandBuffers[g_vulkan.currentFrame], &beginInfo);

            renderer::AsVulkan(renderer)->TransitionPresent(g_vulkan.commandBuffers[g_vulkan.currentFrame], g_vulkan.swapChainImages[vulkanImageIndex]);
            g_vulkan.renderTargetStates[vulkanImageIndex] = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

            vkEndCommandBuffer(g_vulkan.commandBuffers[g_vulkan.currentFrame]);

            VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};

            VkSubmitInfo submitInfo         = {};
            submitInfo.sType                = VK_STRUCTURE_TYPE_SUBMIT_INFO;
            submitInfo.waitSemaphoreCount   = 1;
            submitInfo.pWaitSemaphores      = &g_vulkan.renderFinishedSemaphores[g_vulkan.currentFrame];
            submitInfo.pWaitDstStageMask    = waitStages;
            submitInfo.commandBufferCount   = 1;
            submitInfo.pCommandBuffers      = &g_vulkan.commandBuffers[g_vulkan.currentFrame];
            submitInfo.signalSemaphoreCount = 1;
            submitInfo.pSignalSemaphores    = &g_vulkan.readyToPresentSemaphores[g_vulkan.currentFrame];

            if (vkQueueSubmit(g_vulkan.graphicsQueue, 1, &submitInfo, g_vulkan.inFlightFences[g_vulkan.currentFrame]) != VK_SUCCESS)
            {
                // std::cout << "Failed to submit draw command buffer" << std::endl;
                LNK_ASSERT(false);
            }
        }

        // Present frame.
        {
            VkSwapchainKHR swapChains[] = {g_vulkan.swapChain};

            VkPresentInfoKHR presentInfo   = {};
            presentInfo.sType              = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
            presentInfo.waitSemaphoreCount = 1;
            presentInfo.pWaitSemaphores    = &g_vulkan.readyToPresentSemaphores[g_vulkan.currentFrame];
            presentInfo.swapchainCount     = 1;
            presentInfo.pSwapchains        = swapChains;
            presentInfo.pImageIndices      = &vulkanImageIndex;

            VkResult result = vkQueuePresentKHR(g_vulkan.presentQueue, &presentInfo);

            if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR)
            {
                // Re-create swapchain.
#    if defined(LEIA_OS_WINDOWS)
                ResizeBuffersVulkan(renderer, 0, 0, hWnd);
#    elif defined(LEIA_OS_ANDROID)
                ResizeBuffersVulkan(renderer, 0, 0, androidWindow);
#    endif
            }
            else if (result != VK_SUCCESS)
            {
                // Failed to present swap chain image
                LNK_ASSERT(false);
            }

            g_vulkan.currentFrame = (g_vulkan.currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
        }
#endif
    }
    else
    {
        LNK_ASSERT(false);
    }

    if (graphicsAPI == LEIA_GRAPHICS_API_D3D11)
    {
        if (pBackBuffer != nullptr)
        {
            pBackBuffer->Destroy();
            pBackBuffer = nullptr;
        }
    }
}

void LWE_Core::CreateResources()
{
    AssetManager assetManager = pLeiaSDK->GetAssetManager();

    std::string modelShaderVertexCode;
    std::string modelShaderFragmentCode;
    std::string vertexEntryPt;
    std::string fragmentEntryPt;

    AssetManager::ResolveContext assetResolveContext = {};
#if defined(LEIA_OS_WINDOWS)
    // Resolve assets relative to the lwe.exe
    static int localSymbol            = 0;
    assetResolveContext.moduleAddress = &localSymbol;
#endif // LEIA_OS_WINDOWS

    if (graphicsAPI == LEIA_GRAPHICS_API_OPENGL)
    {
        modelShaderVertexCode   = assetManager.ReadString("shaders/modelShader.vs", &assetResolveContext);
        modelShaderFragmentCode = assetManager.ReadString("shaders/modelShader.fs", &assetResolveContext);
        vertexEntryPt           = "main";
        fragmentEntryPt         = "main";
    }
    else if (graphicsAPI == LEIA_GRAPHICS_API_VULKAN)
    {
        modelShaderVertexCode   = assetManager.ReadString("shaders/vulkan_modelShader.vs", &assetResolveContext);
        modelShaderFragmentCode = assetManager.ReadString("shaders/vulkan_modelShader.fs", &assetResolveContext);
        vertexEntryPt           = "main";
        fragmentEntryPt         = "main";
    }
    else if (graphicsAPI == LEIA_GRAPHICS_API_D3D11 || graphicsAPI == LEIA_GRAPHICS_API_D3D12)
    {
        modelShaderVertexCode   = assetManager.ReadString("shaders/modelShader.hlsl", &assetResolveContext);
        modelShaderFragmentCode = modelShaderVertexCode;
        vertexEntryPt           = "VSMain";
        fragmentEntryPt         = "PSMain";
    }
    else
    {
        LNK_ASSERT(false);
    }

    modelShader =
        renderer->CreateShaderFromText(modelShaderVertexCode.c_str(), vertexEntryPt.c_str(), modelShaderFragmentCode.c_str(), fragmentEntryPt.c_str());

    if (graphicsAPI == LEIA_GRAPHICS_API_OPENGL)
    {
#if defined(WIN32)
        modelShader->SetVertexShaderHeader("#version 330 core\n");
        modelShader->SetPixelShaderHeader("#version 330 core\n");
#elif defined(__ANDROID__)
        modelShader->SetVertexShaderHeader("#version 310 es\n");
        modelShader->SetPixelShaderHeader("#version 310 es\n"
                                          "precision mediump float;\n");
#endif
    }
    else if (graphicsAPI == LEIA_GRAPHICS_API_VULKAN)
    {
#if defined(LEIA_OS_WINDOWS)
        modelShader->SetVertexShaderHeader("#version 450 core\n");
        modelShader->SetPixelShaderHeader("#version 450 core\n");
#elif defined(LEIA_OS_ANDROID)
        modelShader->SetVertexShaderHeader("#version 320 es\n");
        modelShader->SetPixelShaderHeader("#version 320 es\n"
                                          "precision mediump float;\n");
#endif
    }

    // Load image.
    /*Tga tga;
    AssetManager::Buffer buffer = assetManager.ReadData("TILE.TGA");
    tga.ReadFromData(buffer.data.get());
    if (tga.GetPixels().size() > 0)
        tga.SwapRedAndBlue();

    // Create texture.
    renderer->CreateTexture(tga.GetWidth(), tga.GetHeight(), 1, tga.GetBitsPerPixel() == 24 ? renderer::ePixelFormat::RGB :
    renderer::ePixelFormat::RGBA, renderer::eResourceUsageFlags::ShaderInput, tga.GetPixelsBuffer());
    */

    // Create geometry renderstate.
    for (int i = 0; i < 3; i++)
    {
        auto cubeRenderState = renderer->CreateRenderState();
        cubeRenderState->SetShader(modelShader);
        cubeRenderState->AddShaderUniformm("worldViewProj", true); // <<<<<< note true at end specifies the special-case transform matrix uniform name (matrix
                                                                   // itself comes from geometry rendering using this renderstate)
        geometryRenderState.emplace_back(cubeRenderState);

        auto cubeGeometry = renderer->CreateGeometryCube(renderer::vec3f(200.0f), cubeRenderState, true);
        geometry.emplace_back(cubeGeometry);
    }

    // Place cubes at/before/after convergence plane.
    const float convergenceDistance = pLeiaInterlacer->GetConvergenceDistance();
    geometry[0]->SetPosition(renderer::vec3f(0, convergenceDistance, 0));
    geometry[1]->SetPosition(renderer::vec3f(-convergenceDistance * 1.5f, convergenceDistance * 1.5f, -convergenceDistance * 0.8f));
    geometry[2]->SetPosition(renderer::vec3f(convergenceDistance * 0.6f, convergenceDistance * 0.75f, convergenceDistance * 0.35f));
}

#if defined(WIN32)

#    if defined(LEIA_USE_OPENGL)
void GLAPIENTRY DebugGLCallback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* userParam)
{
    UNREFERENCED_PARAMETER(source);
    UNREFERENCED_PARAMETER(type);
    UNREFERENCED_PARAMETER(id);
    UNREFERENCED_PARAMETER(severity);
    UNREFERENCED_PARAMETER(length);
    UNREFERENCED_PARAMETER(message);
    UNREFERENCED_PARAMETER(userParam);
    OVR_DEBUG_LOG(("Message from OpenGL: %s\n", message));

    HWND hWnd = (HWND)userParam;
    MessageBoxA(hWnd, message, "None", MB_OK);
    extern bool finished;
    finished = true;
}

#    endif

static BOOL CALLBACK GetDefaultWindowStartPos_MonitorEnumProc(__in HMONITOR hMonitor, __in HDC hdcMonitor, __in LPRECT lprcMonitor, __in LPARAM dwData)
{
    std::vector<MONITORINFOEX>& infoArray = *reinterpret_cast<std::vector<MONITORINFOEX>*>(dwData);
    MONITORINFOEX               info;
    ZeroMemory(&info, sizeof(info));
    info.cbSize = sizeof(info);
    GetMonitorInfo(hMonitor, &info);
    infoArray.push_back(info);
    return TRUE;
}

bool GetNonPrimaryDisplayTopLeftCoordinate(int& x, int& y)
{
    // Get connected monitor info.
    std::vector<MONITORINFOEX> mInfo;
    mInfo.reserve(::GetSystemMetrics(SM_CMONITORS));
    EnumDisplayMonitors(NULL, NULL, GetDefaultWindowStartPos_MonitorEnumProc, reinterpret_cast<LPARAM>(&mInfo));

    // If we have multiple monitors, select the first non-primary one.
    if (mInfo.size() > 1)
    {
        for (int i = 0; i < mInfo.size(); i++)
        {
            const MONITORINFOEX& mi = mInfo[i];

            if (0 == (mi.dwFlags & MONITORINFOF_PRIMARY))
            {
                x = mi.rcMonitor.left;
                y = mi.rcMonitor.top;
                return true;
            }
        }
    }

    // Didn't find a non-primary, there is only one display connected.
    x = 0;
    y = 0;
    return false;
}

HWND LWE_Core::CreateGraphicsWindow(LWE_WindowSettings const& window)
{
    static HMODULE     hInstance     = NULL;
    static const char* szWindowClass = "LWECoreWindowClass";

    if (hInstance == NULL)
    {
        hInstance = GetModuleHandle(NULL);

        WNDCLASSEXA wcex   = {};
        wcex.cbSize        = sizeof(WNDCLASSEX);
        wcex.style         = CS_HREDRAW | CS_VREDRAW;
        wcex.lpfnWndProc   = window.proc;
        wcex.cbClsExtra    = 0;
        wcex.cbWndExtra    = 0;
        wcex.hInstance     = hInstance;
        wcex.hIcon         = NULL; /// LoadIcon(hInstance, MAKEINTRESOURCE(IDI_DXHELLOWORLD1));
        wcex.hCursor       = LoadCursor(nullptr, IDC_ARROW);
        wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
        wcex.lpszMenuName  = NULL; // MAKEINTRESOURCEW(IDC_DXHELLOWORLD1);
        wcex.lpszClassName = szWindowClass;
        wcex.hIconSm       = NULL; // LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

        RegisterClassExA(&wcex);
    }

    DWORD dwExStyle = WS_EX_APPWINDOW | WS_EX_WINDOWEDGE; // Window Extended Style
    DWORD dwStyle   = WS_OVERLAPPEDWINDOW; // Windows Style

    int defaultX = 0;
    int defaultY = 0;
    if (window.preferSecondaryMonitor)
        GetNonPrimaryDisplayTopLeftCoordinate(defaultX, defaultY);

    RECT WindowRect; // Grabs Rectangle Upper Left / Lower Right Values
    WindowRect.left   = (long)defaultX; // Set Left Value To 0
    WindowRect.right  = (long)(defaultX + window.width); // Set Right Value To Requested Width
    WindowRect.top    = (long)defaultY; // Set Top Value To 0
    WindowRect.bottom = (long)(defaultY + window.height); // Set Bottom Value To Requested Height
    // AdjustWindowRectEx(&WindowRect, dwStyle, FALSE, dwExStyle);		// Adjust Window To True Requested Size

    hWnd = CreateWindowExA(dwExStyle, // Extended Style For The Window
                           szWindowClass, // Class Name
                           window.title, // Window Title
                           dwStyle | // Defined Window Style
                               WS_CLIPSIBLINGS | // Required Window Style
                               WS_CLIPCHILDREN, // Required Window Style
                           WindowRect.left,
                           WindowRect.top, // Window Position
                           WindowRect.right - WindowRect.left, // Calculate Window Width
                           WindowRect.bottom - WindowRect.top, // Calculate Window Height
                           NULL, // No Parent Window
                           NULL, // No Menu
                           hInstance, // Instance
                           NULL // Dont Pass Anything To WM_CREATE
    );

    if (hWnd == NULL)
    {
        MessageBoxA(NULL, "CreateWindow() failed: Cannot create a window.", "Error", MB_OK);
        return NULL;
    }

    if (graphicsAPI == LEIA_GRAPHICS_API_OPENGL)
    {
        HDC hDC = GetDC(hWnd);

        ///* there is no guarantee that the contents of the stack that become
        //   the pfd are zeroed, therefore _make sure_ to clear these bits. */
        // memset(&pfd, 0, sizeof(pfd));
        // pfd.nSize = sizeof(pfd);
        // pfd.nVersion = 1;
        // pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | flags;
        // pfd.iPixelType = type;
        // pfd.cColorBits = 32;

        // pf = ChoosePixelFormat(hDC, &pfd);
        // if (pf == 0) {
        //	MessageBoxA(NULL, "ChoosePixelFormat() failed:  "
        //		"Cannot find a suitable pixel format.", "Error", MB_OK);
        //	return 0;
        // }

        // if (SetPixelFormat(hDC, pf, &pfd) == FALSE) {
        //	MessageBoxA(NULL, "SetPixelFormat() failed:  "
        //		"Cannot set format specified.", "Error", MB_OK);
        //	return 0;
        // }

        // DescribePixelFormat(hDC, pf, sizeof(PIXELFORMATDESCRIPTOR), &pfd);

        PFNWGLCHOOSEPIXELFORMATARBPROC    wglChoosePixelFormatARBFunc    = nullptr;
        PFNWGLCREATECONTEXTATTRIBSARBPROC wglCreateContextAttribsARBFunc = nullptr;
        {
            // First create a context for the purpose of getting access to wglChoosePixelFormatARB / wglCreateContextAttribsARB.
            PIXELFORMATDESCRIPTOR pfd;
            memset(&pfd, 0, sizeof(pfd));
            pfd.nSize      = sizeof(pfd);
            pfd.nVersion   = 1;
            pfd.iPixelType = PFD_TYPE_RGBA;
            pfd.dwFlags    = PFD_SUPPORT_OPENGL | PFD_DRAW_TO_WINDOW | PFD_DOUBLEBUFFER;
            pfd.cColorBits = 32;
            pfd.cDepthBits = 16;
            int pf         = ChoosePixelFormat(hDC, &pfd);
            VALIDATE(pf, "Failed to choose pixel format.");

            VALIDATE(SetPixelFormat(hDC, pf, &pfd), "Failed to set pixel format.");

            HGLRC context = wglCreateContext(hDC);
            VALIDATE(context, "wglCreateContextfailed.");
            VALIDATE(wglMakeCurrent(hDC, context), "wglMakeCurrent failed.");

            wglChoosePixelFormatARBFunc    = (PFNWGLCHOOSEPIXELFORMATARBPROC)wglGetProcAddress("wglChoosePixelFormatARB");
            wglCreateContextAttribsARBFunc = (PFNWGLCREATECONTEXTATTRIBSARBPROC)wglGetProcAddress("wglCreateContextAttribsARB");
            LNK_ASSERT(wglChoosePixelFormatARBFunc && wglCreateContextAttribsARBFunc);

            wglDeleteContext(context);
        }

        // Now create the real context that we will be using.
        int iAttributes[] = {// WGL_DRAW_TO_WINDOW_ARB, GL_TRUE,
                             WGL_SUPPORT_OPENGL_ARB,
                             GL_TRUE,
                             WGL_COLOR_BITS_ARB,
                             32,
                             WGL_DEPTH_BITS_ARB,
                             16,
                             WGL_DOUBLE_BUFFER_ARB,
                             GL_TRUE,
                             WGL_FRAMEBUFFER_SRGB_CAPABLE_ARB,
                             GL_TRUE,
                             0,
                             0};

        float fAttributes[] = {0, 0};
        int   pf            = 0;
        UINT  numFormats    = 0;

        VALIDATE(wglChoosePixelFormatARBFunc(hDC, iAttributes, fAttributes, 1, &pf, &numFormats), "wglChoosePixelFormatARBFunc failed.");

        PIXELFORMATDESCRIPTOR pfd;
        memset(&pfd, 0, sizeof(pfd));
        VALIDATE(SetPixelFormat(hDC, pf, &pfd), "SetPixelFormat failed.");

        GLint attribs[16];
        int   attribCount = 0;
        if (UseDebugContext)
        {
            attribs[attribCount++] = WGL_CONTEXT_FLAGS_ARB;
            attribs[attribCount++] = WGL_CONTEXT_DEBUG_BIT_ARB;
        }

        attribs[attribCount++] = WGL_CONTEXT_MAJOR_VERSION_ARB;
        attribs[attribCount++] = 3;

        attribs[attribCount++] = WGL_CONTEXT_MINOR_VERSION_ARB;
        attribs[attribCount++] = 0;

        attribs[attribCount++] = GL_CONTEXT_PROFILE_MASK;
        attribs[attribCount++] = GL_CONTEXT_CORE_PROFILE_BIT;

        /*attribs[attribCount++] = WGL_CONTEXT_FLAGS_ARB;
        attribs[attribCount++] = WGL_CONTEXT_DEBUG_BIT_ARB;*/

        attribs[attribCount] = 0;

        WglContext = wglCreateContextAttribsARBFunc(hDC, 0, attribs);
        VALIDATE(wglMakeCurrent(hDC, WglContext), "wglMakeCurrent failed.");

        // Have to init this after a glContext is created but before we make any calls to glXYZ
        OVR::GLEContext::GetCurrentContext()->Init();

        std::string ver = (const char*)glGetString(GL_VERSION);

        if (UseDebugContext && GLE_ARB_debug_output)
        {
            glDebugMessageCallbackARB(DebugGLCallback, hWnd);
            if (glGetError())
            {
                OVR_DEBUG_LOG(("glDebugMessageCallbackARB failed."));
            }

            glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS_ARB);

            // Explicitly disable notification severity output.
            glDebugMessageControlARB(GL_DEBUG_SOURCE_API, GL_DONT_CARE, GL_DEBUG_SEVERITY_NOTIFICATION, 0, NULL, GL_FALSE);
        }

        ReleaseDC(hWnd, hDC);
    }

    SetProcessDPIAware();

    // ShowWindow(hWnd, SW_NORMAL);// nCmdShow);
    // UpdateWindow(hWnd);

    return hWnd;
}

#endif

void LWE_Core::SetFullscreen(bool fullscreen)
{
#if defined(WIN32)
    static int windowPrevX      = 0;
    static int windowPrevY      = 0;
    static int windowPrevWidth  = 0;
    static int windowPrevHeight = 0;

    DWORD style = GetWindowLong(hWnd, GWL_STYLE);
    if (fullscreen)
    {
        RECT        rect;
        MONITORINFO mi = {sizeof(mi)};
        GetWindowRect(hWnd, &rect);

        windowPrevX      = rect.left;
        windowPrevY      = rect.top;
        windowPrevWidth  = rect.right - rect.left;
        windowPrevHeight = rect.bottom - rect.top;

        GetMonitorInfo(MonitorFromWindow(hWnd, MONITOR_DEFAULTTOPRIMARY), &mi);
        SetWindowLong(hWnd, GWL_STYLE, style & ~WS_OVERLAPPEDWINDOW);
        SetWindowPos(hWnd,
                     HWND_TOP,
                     mi.rcMonitor.left,
                     mi.rcMonitor.top,
                     mi.rcMonitor.right - mi.rcMonitor.left,
                     mi.rcMonitor.bottom - mi.rcMonitor.top,
                     SWP_NOOWNERZORDER | SWP_FRAMECHANGED | SWP_SHOWWINDOW);
    }
    else
    {
        MONITORINFO mi    = {sizeof(mi)};
        UINT        flags = SWP_NOZORDER | SWP_FRAMECHANGED | SWP_SHOWWINDOW;
        GetMonitorInfo(MonitorFromWindow(hWnd, MONITOR_DEFAULTTOPRIMARY), &mi);
        SetWindowLong(hWnd, GWL_STYLE, style | WS_OVERLAPPEDWINDOW);
        SetWindowPos(hWnd, HWND_NOTOPMOST, windowPrevX, windowPrevY, windowPrevWidth, windowPrevHeight, flags);
    }
#endif
}

void LWE_Core::ToggleInterlaceMode()
{
    SetInterlaceMode((InterlaceMode)(((int)pLeiaInterlacer->GetInterlaceMode() + 1) % LEIA_INTERLACE_MODE_COUNT));
}

void LWE_Core::ToggleFaceTracking()
{
    pLeiaSDK->EnableFaceTracking(!pLeiaSDK->IsFaceTrackingEnabled());
}

void LWE_Core::SetShaderDebugMode(ShaderDebugMode mode)
{
    pLeiaInterlacer->SetShaderDebugMode(mode);

    int tilesX = (mode == LEIA_SHADER_DEBUG_MODE_SHOW_CALIBRATION_IMAGE) ? 3 : 2;
    int tilesY = (mode == LEIA_SHADER_DEBUG_MODE_SHOW_CALIBRATION_IMAGE) ? 4 : 1;
    pLeiaInterlacer->SetNumTiles(tilesX, tilesY);
}

void LWE_Core::SetInterlaceMode(InterlaceMode mode)
{
    pLeiaInterlacer->SetInterlaceMode(mode);
}

void LWE_Core::MoveForward(float amount)
{
    gCamera.pos = gCamera.pos + gCamera.getForward() * amount;
}

void LWE_Core::MoveBackward(float amount)
{
    gCamera.pos = gCamera.pos - gCamera.getForward() * amount;
}

void LWE_Core::MoveLeft(float amount)
{
    gCamera.pos = gCamera.pos - gCamera.getRight() * amount;
}

void LWE_Core::MoveRight(float amount)
{
    gCamera.pos = gCamera.pos + gCamera.getRight() * amount;
}

void LWE_Core::BaselineScalingMore()
{
    pLeiaInterlacer->SetBaselineScaling(pLeiaInterlacer->GetBaselineScaling() + 0.1f);
}

void LWE_Core::BaselineScalingLess()
{
    pLeiaInterlacer->SetBaselineScaling(pLeiaInterlacer->GetBaselineScaling() - 0.1f);
}

void LWE_Core::ToggleShaderDebugMode()
{
    SetShaderDebugMode((ShaderDebugMode)(((int)pLeiaInterlacer->GetShaderDebugMode() + 1) % LEIA_SHADER_DEBUG_MODE_COUNT));
}

void LWE_Core::InterlaceCenterViewMore()
{
    pLeiaSDK->SetCenterView(pLeiaSDK->GetCenterView() + 1.0f);
}

void LWE_Core::InterlaceCenterViewLess()
{
    pLeiaSDK->SetCenterView(pLeiaSDK->GetCenterView() - 1.0f);
}

int LWE_Core::GetWindowWidth() const
{
    return windowWidth;
}

int LWE_Core::GetWindowHeight() const
{
    return windowHeight;
}

void LWE_Core::ToggleAnimation()
{
    animationEnabled = !animationEnabled;
}

void LWE_Core::ToggleSharpening()
{
    pLeiaInterlacer->EnableSharpening(!pLeiaInterlacer->IsSharpeningEnabled());
}

void LWE_Core::ToggleReconvergence()
{
    pLeiaInterlacer->EnableReconvergence(!pLeiaInterlacer->IsReconvergenceEnabled());
}

void LWE_Core::ReconvergenceLess()
{
    pLeiaInterlacer->SetReconvergence(pLeiaInterlacer->GetReconvergence() - 0.01f);
}

void LWE_Core::ReconvergenceMore()
{
    pLeiaInterlacer->SetReconvergence(pLeiaInterlacer->GetReconvergence() + 0.01f);
}

bool LWE_Core::ToggleGui()
{
    bool isVisible = !pLeiaInterlacer->IsGuiVisible();
    pLeiaInterlacer->SetGuiVisibility(isVisible);
    return isVisible;
}

#if defined(LEIA_OS_WINDOWS)
sdk::GuiInputState LWE_Core::ProcessGuiInput(leia_interlacer_gui_surface surface, uint32_t msg, uint64_t wparam, int64_t lparam)
{
    if (!IsInitialized())
    {
        return {};
    }
    return pLeiaInterlacer->ProcessGuiInput(surface, msg, wparam, lparam);
}
#elif defined(LEIA_OS_ANDROID)
sdk::GuiInputState LWE_Core::ProcessGuiInput(AInputEvent const* inputEvent)
{
    if (!IsInitialized())
    {
        return {};
    }
    return pLeiaInterlacer->ProcessGuiInput(inputEvent);
}
sdk::GuiInputState LWE_Core::ProcessGuiMotionInput(JNIEnv* jniEnv, jobject motionEvent)
{
    if (!IsInitialized())
    {
        return {};
    }
    return pLeiaInterlacer->ProcessGuiMotionInput(jniEnv, motionEvent);
}
#endif

void LWE_Core::AspectRatioOffsetMore()
{
    pLeiaInterlacer->SetAspectRatioOffset(pLeiaInterlacer->GetAspectRatioOffset() + 0.05f);
}

void LWE_Core::AspectRatioOffsetLess()
{
    pLeiaInterlacer->SetAspectRatioOffset(pLeiaInterlacer->GetAspectRatioOffset() - 0.05f);
}

void LWE_Core::SetBacklight(bool enable)
{
    pLeiaSDK->SetBacklight(enable);
}

} // namespace lwe
} // namespace leia

#if defined(LEIA_OS_ANDROID)

#    include "leia/common/android/jniLoader.h"

extern "C" {

JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM* vm, void*)
{
    return leia_jni_on_load(vm);
}

JNIEXPORT void JNICALL JNI_OnUnload(JavaVM* vm, void*)
{
    leia_jni_on_unload(vm);
}

} // extern "C"

#endif // LEIA_OS_ANDROID
