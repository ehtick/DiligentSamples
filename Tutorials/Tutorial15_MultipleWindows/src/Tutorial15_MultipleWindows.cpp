/*
 *  Copyright 2019-2025 Diligent Graphics LLC
 *  Copyright 2015-2019 Egor Yusov
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 *
 *  In no event and under no legal theory, whether in tort (including negligence),
 *  contract, or otherwise, unless required by applicable law (such as deliberate
 *  and grossly negligent acts) or agreed to in writing, shall any Contributor be
 *  liable for any damages, including any direct, indirect, special, incidental,
 *  or consequential damages of any character arising as a result of this License or
 *  out of the use or inability to use the software (including but not limited to damages
 *  for loss of goodwill, work stoppage, computer failure or malfunction, or any and
 *  all other commercial damages or losses), even if such Contributor has been advised
 *  of the possibility of such damages.
 */

#include <memory>
#include <iomanip>
#include <iostream>
#include <vector>
#include <array>
#include <sstream>

#ifndef NOMINMAX
#    define NOMINMAX
#endif
#include <Windows.h>
#include <crtdbg.h>

#ifndef PLATFORM_WIN32
#    define PLATFORM_WIN32 1
#endif

#ifndef ENGINE_DLL
#    define ENGINE_DLL 1
#endif

#ifndef D3D11_SUPPORTED
#    define D3D11_SUPPORTED 1
#endif

#ifndef D3D12_SUPPORTED
#    define D3D12_SUPPORTED 1
#endif

#ifndef GL_SUPPORTED
#    define GL_SUPPORTED 1
#endif

#ifndef VULKAN_SUPPORTED
#    define VULKAN_SUPPORTED 1
#endif

#include "Graphics/GraphicsEngineD3D11/interface/EngineFactoryD3D11.h"
#include "Graphics/GraphicsEngineD3D12/interface/EngineFactoryD3D12.h"
#include "Graphics/GraphicsEngineOpenGL/interface/EngineFactoryOpenGL.h"
#include "Graphics/GraphicsEngineVulkan/interface/EngineFactoryVk.h"

#include "Graphics/GraphicsEngine/interface/RenderDevice.h"
#include "Graphics/GraphicsEngine/interface/DeviceContext.h"
#include "Graphics/GraphicsEngine/interface/SwapChain.h"

#include "Common/interface/RefCntAutoPtr.hpp"

using namespace Diligent;

// For this tutorial, we will use simple vertex shader
// that creates a procedural triangle

// Diligent Engine can use HLSL source on all supported platforms.
// It will convert HLSL to GLSL in OpenGL mode, while Vulkan backend will compile it directly to SPIRV.

static const char* VSSource = R"(
struct PSInput 
{ 
    float4 Pos   : SV_POSITION; 
    float3 Color : COLOR; 
};

void main(in  uint    VertId : SV_VertexID,
          out PSInput PSIn) 
{
    float4 Pos[3];
    Pos[0] = float4(-0.5, -0.5, 0.0, 1.0);
    Pos[1] = float4( 0.0, +0.5, 0.0, 1.0);
    Pos[2] = float4(+0.5, -0.5, 0.0, 1.0);

    float3 Col[3];
    Col[0] = float3(1.0, 0.0, 0.0); // red
    Col[1] = float3(0.0, 1.0, 0.0); // green
    Col[2] = float3(0.0, 0.0, 1.0); // blue

    PSIn.Pos   = Pos[VertId];
    PSIn.Color = Col[VertId];
}
)";

// Pixel shader simply outputs interpolated vertex color
static const char* PSSource = R"(
struct PSInput 
{ 
    float4 Pos   : SV_POSITION; 
    float3 Color : COLOR; 
};

struct PSOutput
{ 
    float4 Color : SV_TARGET; 
};

void main(in  PSInput  PSIn,
          out PSOutput PSOut)
{
    PSOut.Color = float4(PSIn.Color.rgb, 1.0);
}
)";


class Tutorial00App
{
public:
    Tutorial00App()
    {
    }

    ~Tutorial00App()
    {
        if (m_pImmediateContext)
            m_pImmediateContext->Flush();
    }

    bool InitializeDiligentEngine(HWND hWnd[], size_t NumWindows)
    {
        m_Windows.resize(NumWindows);
        for (size_t i = 0; i < NumWindows; ++i)
            m_Windows[i].hWnd = hWnd[i];

        SwapChainDesc SCDesc;
        switch (m_DeviceType)
        {
#if D3D11_SUPPORTED
            case RENDER_DEVICE_TYPE_D3D11:
            {
                EngineD3D11CreateInfo EngineCI;

#    if ENGINE_DLL
                // Load the dll and import GetEngineFactoryD3D11() function
                GetEngineFactoryD3D11Type GetEngineFactoryD3D11 = LoadGraphicsEngineD3D11();
#    endif
                IEngineFactoryD3D11* pFactoryD3D11 = GetEngineFactoryD3D11();
                pFactoryD3D11->CreateDeviceAndContextsD3D11(EngineCI, &m_pDevice, &m_pImmediateContext);
                for (WindowInfo& WndInfo : m_Windows)
                {
                    Win32NativeWindow Window{WndInfo.hWnd};
                    pFactoryD3D11->CreateSwapChainD3D11(m_pDevice, m_pImmediateContext, SCDesc, FullScreenModeDesc{}, Window, &WndInfo.pSwapChain);
                    SCDesc.IsPrimary = false;
                }
            }
            break;
#endif


#if D3D12_SUPPORTED
            case RENDER_DEVICE_TYPE_D3D12:
            {
#    if ENGINE_DLL
                // Load the dll and import GetEngineFactoryD3D12() function
                GetEngineFactoryD3D12Type GetEngineFactoryD3D12 = LoadGraphicsEngineD3D12();
#    endif
                EngineD3D12CreateInfo EngineCI;

                IEngineFactoryD3D12* pFactoryD3D12 = GetEngineFactoryD3D12();
                pFactoryD3D12->CreateDeviceAndContextsD3D12(EngineCI, &m_pDevice, &m_pImmediateContext);
                for (WindowInfo& WndInfo : m_Windows)
                {
                    Win32NativeWindow Window{WndInfo.hWnd};
                    pFactoryD3D12->CreateSwapChainD3D12(m_pDevice, m_pImmediateContext, SCDesc, FullScreenModeDesc{}, Window, &WndInfo.pSwapChain);
                    SCDesc.IsPrimary = false;
                }
            }
            break;
#endif


#if GL_SUPPORTED
            case RENDER_DEVICE_TYPE_GL:
            {

#    if EXPLICITLY_LOAD_ENGINE_GL_DLL
                // Load the dll and import GetEngineFactoryOpenGL() function
                GetEngineFactoryOpenGLType GetEngineFactoryOpenGL = LoadGraphicsEngineOpenGL();
#    endif
                MessageBox(NULL, L"OpenGL backend does not currently support multiple swap chains", L"Error", MB_OK | MB_ICONWARNING);
                IEngineFactoryOpenGL* pFactoryOpenGL = GetEngineFactoryOpenGL();

                EngineGLCreateInfo EngineCI;

                WindowInfo& WndInfo = m_Windows[0];

                EngineCI.Window.hWnd = WndInfo.hWnd;
                pFactoryOpenGL->CreateDeviceAndSwapChainGL(EngineCI, &m_pDevice, &m_pImmediateContext, SCDesc, &WndInfo.pSwapChain);
            }
            break;
#endif


#if VULKAN_SUPPORTED
            case RENDER_DEVICE_TYPE_VULKAN:
            {
#    if EXPLICITLY_LOAD_ENGINE_VK_DLL
                // Load the dll and import GetEngineFactoryVk() function
                GetEngineFactoryVkType GetEngineFactoryVk = LoadGraphicsEngineVk();
#    endif
                EngineVkCreateInfo EngineCI;

                IEngineFactoryVk* pFactoryVk = GetEngineFactoryVk();
                pFactoryVk->CreateDeviceAndContextsVk(EngineCI, &m_pDevice, &m_pImmediateContext);
                for (WindowInfo& WndInfo : m_Windows)
                {
                    Win32NativeWindow Window{WndInfo.hWnd};
                    pFactoryVk->CreateSwapChainVk(m_pDevice, m_pImmediateContext, SCDesc, Window, &WndInfo.pSwapChain);
                    SCDesc.IsPrimary = false;
                }
            }
            break;
#endif


            default:
                std::cerr << "Unknown/unsupported device type";
                return false;
                break;
        }

        return true;
    }

    bool ProcessCommandLine(const char* CmdLine)
    {
        const char* mode = nullptr;

        const char* Keys[] = {"--mode ", "--mode=", "-m "};
        for (size_t i = 0; i < _countof(Keys); ++i)
        {
            const char* Key = Keys[i];
            if ((mode = strstr(CmdLine, Key)) != nullptr)
            {
                mode += strlen(Key);
                break;
            }
        }

        if (mode != nullptr)
        {
            while (*mode == ' ')
                ++mode;

            if (_stricmp(mode, "D3D11") == 0)
            {
#if D3D11_SUPPORTED
                m_DeviceType = RENDER_DEVICE_TYPE_D3D11;
#else
                std::cerr << "Direct3D11 is not supported. Please select another device type";
                return false;
#endif
            }
            else if (_stricmp(mode, "D3D12") == 0)
            {
#if D3D12_SUPPORTED
                m_DeviceType = RENDER_DEVICE_TYPE_D3D12;
#else
                std::cerr << "Direct3D12 is not supported. Please select another device type";
                return false;
#endif
            }
            else if (_stricmp(mode, "GL") == 0)
            {
#if GL_SUPPORTED
                m_DeviceType = RENDER_DEVICE_TYPE_GL;
#else
                std::cerr << "OpenGL is not supported. Please select another device type";
                return false;
#endif
            }
            else if (_stricmp(mode, "VK") == 0)
            {
#if VULKAN_SUPPORTED
                m_DeviceType = RENDER_DEVICE_TYPE_VULKAN;
#else
                std::cerr << "Vulkan is not supported. Please select another device type";
                return false;
#endif
            }
            else
            {
                std::cerr << mode << " is not a valid device type. Only the following types are supported: D3D11, D3D12, GL, VK";
                return false;
            }
        }
        else
        {
#if D3D12_SUPPORTED
            m_DeviceType = RENDER_DEVICE_TYPE_D3D12;
#elif VULKAN_SUPPORTED
            m_DeviceType = RENDER_DEVICE_TYPE_VULKAN;
#elif D3D11_SUPPORTED
            m_DeviceType = RENDER_DEVICE_TYPE_D3D11;
#elif GL_SUPPORTED
            m_DeviceType = RENDER_DEVICE_TYPE_GL;
#endif
        }
        return true;
    }


    void CreateResources()
    {
        // Pipeline state object encompasses configuration of all GPU stages

        GraphicsPipelineStateCreateInfo PSOCreateInfo;

        // Pipeline state name is used by the engine to report issues.
        // It is always a good idea to give objects descriptive names.
        PSOCreateInfo.PSODesc.Name = "Simple triangle PSO";

        // This is a graphics pipeline
        PSOCreateInfo.PSODesc.PipelineType = PIPELINE_TYPE_GRAPHICS;

        // clang-format off
        // This tutorial will render to a single render target
        PSOCreateInfo.GraphicsPipeline.NumRenderTargets             = 1;
        // Set render target format which is the format of the swap chain's color buffer
        ISwapChain* pSwapChain = m_Windows[0].pSwapChain;
        PSOCreateInfo.GraphicsPipeline.RTVFormats[0]                = pSwapChain->GetDesc().ColorBufferFormat;
        // Use the depth buffer format from the swap chain
        PSOCreateInfo.GraphicsPipeline.DSVFormat                    = pSwapChain->GetDesc().DepthBufferFormat;
        // Primitive topology defines what kind of primitives will be rendered by this pipeline state
        PSOCreateInfo.GraphicsPipeline.PrimitiveTopology            = PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
        // No back face culling for this tutorial
        PSOCreateInfo.GraphicsPipeline.RasterizerDesc.CullMode      = CULL_MODE_NONE;
        // Disable depth testing
        PSOCreateInfo.GraphicsPipeline.DepthStencilDesc.DepthEnable = False;
        // clang-format on

        ShaderCreateInfo ShaderCI;
        // Tell the system that the shader source code is in HLSL.
        // For OpenGL, the engine will convert this into GLSL under the hood
        ShaderCI.SourceLanguage = SHADER_SOURCE_LANGUAGE_HLSL;
        // OpenGL backend requires emulated combined HLSL texture samplers (g_Texture + g_Texture_sampler combination)
        ShaderCI.Desc.UseCombinedTextureSamplers = true;
        // Create a vertex shader
        RefCntAutoPtr<IShader> pVS;
        {
            ShaderCI.Desc.ShaderType = SHADER_TYPE_VERTEX;
            ShaderCI.EntryPoint      = "main";
            ShaderCI.Desc.Name       = "Triangle vertex shader";
            ShaderCI.Source          = VSSource;
            m_pDevice->CreateShader(ShaderCI, &pVS);
        }

        // Create a pixel shader
        RefCntAutoPtr<IShader> pPS;
        {
            ShaderCI.Desc.ShaderType = SHADER_TYPE_PIXEL;
            ShaderCI.EntryPoint      = "main";
            ShaderCI.Desc.Name       = "Triangle pixel shader";
            ShaderCI.Source          = PSSource;
            m_pDevice->CreateShader(ShaderCI, &pPS);
        }

        // Finally, create the pipeline state
        PSOCreateInfo.pVS = pVS;
        PSOCreateInfo.pPS = pPS;
        m_pDevice->CreateGraphicsPipelineState(PSOCreateInfo, &m_pPSO);
    }

    void Render()
    {
        for (size_t i = 0; i < m_Windows.size(); ++i)
        {
            WindowInfo& WndInfo = m_Windows[i];
            if (!WndInfo.pSwapChain)
                continue;

            ITextureView* pRTV = WndInfo.pSwapChain->GetCurrentBackBufferRTV();
            ITextureView* pDSV = WndInfo.pSwapChain->GetDepthBufferDSV();
            m_pImmediateContext->SetRenderTargets(1, &pRTV, pDSV, RESOURCE_STATE_TRANSITION_MODE_TRANSITION);


            // Clear the back buffer
            const float ClearColor[] = {0.350f, 0.350f, 0.350f, 1.0f};
            // Let the engine perform required state transitions
            m_pImmediateContext->ClearRenderTarget(pRTV, ClearColor, RESOURCE_STATE_TRANSITION_MODE_TRANSITION);
            m_pImmediateContext->ClearDepthStencil(pDSV, CLEAR_DEPTH_FLAG, 1.f, 0, RESOURCE_STATE_TRANSITION_MODE_TRANSITION);

            // Set the pipeline state in the immediate context
            m_pImmediateContext->SetPipelineState(m_pPSO);

            DrawAttribs drawAttrs;
            drawAttrs.NumVertices = 3; // Render 3 vertices
            m_pImmediateContext->Draw(drawAttrs);
        }
    }

    void Present()
    {
        for (WindowInfo& WndInfo : m_Windows)
        {
            if (WndInfo.pSwapChain)
                WndInfo.pSwapChain->Present();
        }
    }

    void WindowResize(HWND hWnd, Uint32 Width, Uint32 Height)
    {
        for (WindowInfo& WndInfo : m_Windows)
        {
            if (WndInfo.hWnd == hWnd)
            {
                if (WndInfo.pSwapChain)
                    WndInfo.pSwapChain->Resize(Width, Height);
                break;
            }
        }
    }

    RENDER_DEVICE_TYPE GetDeviceType() const { return m_DeviceType; }

private:
    RefCntAutoPtr<IRenderDevice>  m_pDevice;
    RefCntAutoPtr<IDeviceContext> m_pImmediateContext;
    RefCntAutoPtr<IPipelineState> m_pPSO;
    RENDER_DEVICE_TYPE            m_DeviceType = RENDER_DEVICE_TYPE_D3D11;
    struct WindowInfo
    {
        RefCntAutoPtr<ISwapChain> pSwapChain;
        HWND                      hWnd = NULL;
    };
    std::vector<WindowInfo> m_Windows;
};

std::unique_ptr<Tutorial00App> g_pTheApp;

LRESULT CALLBACK MessageProc(HWND, UINT, WPARAM, LPARAM);
// Main
int WINAPI WinMain(_In_ HINSTANCE     hInstance,
                   _In_opt_ HINSTANCE hPrevInstance,
                   _In_ LPSTR         lpCmdLine,
                   _In_ int           nShowCmd)
{
#if defined(_DEBUG) || defined(DEBUG)
    _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif

    g_pTheApp.reset(new Tutorial00App);

    LPSTR cmdLine = GetCommandLineA();
    if (!g_pTheApp->ProcessCommandLine(cmdLine))
        return -1;

    // Register our window class
    WNDCLASSEX wcex = {sizeof(WNDCLASSEX), CS_HREDRAW | CS_VREDRAW, MessageProc,
                       0L, 0L, hInstance, NULL, NULL, NULL, NULL, L"SampleApp", NULL};
    RegisterClassEx(&wcex);

    constexpr size_t NumWindows = 3;

    std::array<HWND, NumWindows> hWnds;
    // clang-format off
    std::array<RECT, NumWindows> WndRects =
    {
        RECT{0, 0, 1024, 768},
        RECT{0, 0, 640, 480},
        RECT{0, 0, 480, 320}
    };
    // clang-format on

    for (size_t i = 0; i < NumWindows; ++i)
    {
        std::wstringstream TitleSS;
        TitleSS << L"Tutorial15: Multiple Windows";
        switch (g_pTheApp->GetDeviceType())
        {
            case RENDER_DEVICE_TYPE_D3D11: TitleSS << L" (D3D11)"; break;
            case RENDER_DEVICE_TYPE_D3D12: TitleSS << L" (D3D12)"; break;
            case RENDER_DEVICE_TYPE_GL: TitleSS << L" (GL)"; break;
            case RENDER_DEVICE_TYPE_VULKAN: TitleSS << L" (VK)"; break;
        }
        TitleSS << " - Window " << i;
        std::wstring Title = TitleSS.str();

        RECT& rc = WndRects[i];
        AdjustWindowRect(&rc, WS_OVERLAPPEDWINDOW, FALSE);
        hWnds[i] = CreateWindow(L"SampleApp", Title.c_str(),
                                WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT,
                                rc.right - rc.left, rc.bottom - rc.top, NULL, NULL, hInstance, NULL);
        if (!hWnds[i])
        {
            MessageBox(NULL, L"Cannot create window", L"Error", MB_OK | MB_ICONERROR);
            return -1;
        }
        ShowWindow(hWnds[i], nShowCmd);
        UpdateWindow(hWnds[i]);
    }

    if (!g_pTheApp->InitializeDiligentEngine(hWnds.data(), hWnds.size()))
        return -1;

    g_pTheApp->CreateResources();

    // Main message loop
    MSG msg = {0};
    while (WM_QUIT != msg.message)
    {
        if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
        else
        {
            g_pTheApp->Render();
            g_pTheApp->Present();
        }
    }

    g_pTheApp.reset();

    return (int)msg.wParam;
}

// Called every time the NativeNativeAppBase receives a message
LRESULT CALLBACK MessageProc(HWND wnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
        case WM_PAINT:
        {
            PAINTSTRUCT ps;
            BeginPaint(wnd, &ps);
            EndPaint(wnd, &ps);
            return 0;
        }
        case WM_SIZE: // Window size has been changed
            if (g_pTheApp)
            {
                g_pTheApp->WindowResize(wnd, LOWORD(lParam), HIWORD(lParam));
            }
            return 0;

        case WM_CHAR:
            if (wParam == VK_ESCAPE)
                PostQuitMessage(0);
            return 0;

        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;

        case WM_GETMINMAXINFO:
        {
            LPMINMAXINFO lpMMI      = (LPMINMAXINFO)lParam;
            lpMMI->ptMinTrackSize.x = 320;
            lpMMI->ptMinTrackSize.y = 240;
            return 0;
        }

        default:
            return DefWindowProc(wnd, message, wParam, lParam);
    }
}
