/*
 *  Copyright 2019-2021 Diligent Graphics LLC
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

#ifndef ENGINE_DLL
#    define ENGINE_DLL 1
#endif

#ifndef D3D11_SUPPORTED
#    define D3D11_SUPPORTED 0
#endif

#ifndef D3D12_SUPPORTED
#    define D3D12_SUPPORTED 0
#endif

#ifndef GL_SUPPORTED
#    define GL_SUPPORTED 0
#endif

#ifndef VULKAN_SUPPORTED
#    define VULKAN_SUPPORTED 0
#endif

#if PLATFORM_WIN32
#    define GLFW_EXPOSE_NATIVE_WIN32 1
#endif

#if PLATFORM_LINUX
#    define GLFW_EXPOSE_NATIVE_X11 1
#    define _stricmp               strcasecmp
#endif

#if PLATFORM_MACOS
#    define GLFW_EXPOSE_NATIVE_COCOA 1
#endif

#if D3D11_SUPPORTED
#    include "Graphics/GraphicsEngineD3D11/interface/EngineFactoryD3D11.h"
#endif
#if D3D12_SUPPORTED
#    include "Graphics/GraphicsEngineD3D12/interface/EngineFactoryD3D12.h"
#endif
#if GL_SUPPORTED
#    include "Graphics/GraphicsEngineOpenGL/interface/EngineFactoryOpenGL.h"
#endif
#if VULKAN_SUPPORTED
#    include "Graphics/GraphicsEngineVulkan/interface/EngineFactoryVk.h"
#endif

#if PLATFORM_WIN32
#    undef GetObject
#    undef CreateWindow
#endif

#include "GLFWSample.hpp"

#include "GLFW/glfw3native.h"

namespace Diligent
{

GLFWSample::GLFWSample()
{
}

GLFWSample::~GLFWSample()
{
    if (m_pImmediateContext)
        m_pImmediateContext->Flush();

    m_pSwapChain        = nullptr;
    m_pImmediateContext = nullptr;
    m_pDevice           = nullptr;

    if (m_Window)
    {
        glfwDestroyWindow(m_Window);
        glfwTerminate();
    }
}

bool GLFWSample::CreateWindow(const char* Title, int Width, int Height)
{
    if (glfwInit() != GLFW_TRUE)
        return false;

    // Use GLFW only to create window, skip additional work for graphics API initialization.
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

    m_Window = glfwCreateWindow(Width, Height, Title, nullptr, nullptr);
    if (m_Window == nullptr)
        return false;

    glfwSetWindowUserPointer(m_Window, this);
    glfwSetFramebufferSizeCallback(m_Window, &GLFW_ResizeCallback);
    glfwSetKeyCallback(m_Window, &GLFW_KeyCallback);
    glfwSetMouseButtonCallback(m_Window, &GLFW_MouseButtonCallback);
    glfwSetCursorPosCallback(m_Window, &GLFW_CursorPosCallback);
    glfwSetScrollCallback(m_Window, &GLFW_MouseWheelCallback);

    return true;
}

bool GLFWSample::CreateEngine(RENDER_DEVICE_TYPE DevType)
{
#if PLATFORM_WIN32
    Win32NativeWindow Window{glfwGetWin32Window(m_Window)};
#endif
#if PLATFORM_LINUX
    LinuxNativeWindow Window{glfwGetX11Window(m_Window), glfwGetX11Display()};
#endif
#if PLATFORM_MACOS
    MacOSNativeWindow Window{glfwGetCocoaWindow(m_Window)};
#endif

    SwapChainDesc SCDesc;
    switch (DevType)
    {
#if D3D11_SUPPORTED
        case RENDER_DEVICE_TYPE_D3D11:
        {
            EngineD3D11CreateInfo EngineCI;
#    ifdef DILIGENT_DEBUG
            EngineCI.DebugFlags |=
                D3D11_DEBUG_FLAG_CREATE_DEBUG_DEVICE |
                D3D11_DEBUG_FLAG_VERIFY_COMMITTED_SHADER_RESOURCES;
#    endif
#    if ENGINE_DLL
            // Load the dll and import GetEngineFactoryD3D11() function
            auto GetEngineFactoryD3D11 = LoadGraphicsEngineD3D11();
#    endif
            auto* pFactoryD3D11 = GetEngineFactoryD3D11();
            pFactoryD3D11->CreateDeviceAndContextsD3D11(EngineCI, &m_pDevice, &m_pImmediateContext);
            pFactoryD3D11->CreateSwapChainD3D11(m_pDevice, m_pImmediateContext, SCDesc, FullScreenModeDesc{}, Window, &m_pSwapChain);
        }
        break;
#endif // D3D11_SUPPORTED


#if D3D12_SUPPORTED
        case RENDER_DEVICE_TYPE_D3D12:
        {
#    if ENGINE_DLL
            // Load the dll and import GetEngineFactoryD3D12() function
            auto GetEngineFactoryD3D12 = LoadGraphicsEngineD3D12();
#    endif
            EngineD3D12CreateInfo EngineCI;
#    ifdef DILIGENT_DEBUG
            EngineCI.EnableDebugLayer = true;
#    endif
            auto* pFactoryD3D12 = GetEngineFactoryD3D12();
            pFactoryD3D12->CreateDeviceAndContextsD3D12(EngineCI, &m_pDevice, &m_pImmediateContext);
            pFactoryD3D12->CreateSwapChainD3D12(m_pDevice, m_pImmediateContext, SCDesc, FullScreenModeDesc{}, Window, &m_pSwapChain);
        }
        break;
#endif // D3D12_SUPPORTED


#if GL_SUPPORTED
        case RENDER_DEVICE_TYPE_GL:
        {
#    if EXPLICITLY_LOAD_ENGINE_GL_DLL
            // Load the dll and import GetEngineFactoryOpenGL() function
            auto GetEngineFactoryOpenGL = LoadGraphicsEngineOpenGL();
#    endif
            auto* pFactoryOpenGL = GetEngineFactoryOpenGL();

            EngineGLCreateInfo EngineCI;
            EngineCI.Window = Window;
#    ifdef DILIGENT_DEBUG
            EngineCI.CreateDebugContext = true;
#    endif
            pFactoryOpenGL->CreateDeviceAndSwapChainGL(EngineCI, &m_pDevice, &m_pImmediateContext, SCDesc, &m_pSwapChain);
        }
        break;
#endif // GL_SUPPORTED


#if VULKAN_SUPPORTED
        case RENDER_DEVICE_TYPE_VULKAN:
        {
#    if EXPLICITLY_LOAD_ENGINE_VK_DLL
            // Load the dll and import GetEngineFactoryVk() function
            auto GetEngineFactoryVk = LoadGraphicsEngineVk();
#    endif
            EngineVkCreateInfo EngineCI;
#    ifdef DILIGENT_DEBUG
            EngineCI.EnableValidation = true;
#    endif
            auto* pFactoryVk = GetEngineFactoryVk();
            pFactoryVk->CreateDeviceAndContextsVk(EngineCI, &m_pDevice, &m_pImmediateContext);
            pFactoryVk->CreateSwapChainVk(m_pDevice, m_pImmediateContext, SCDesc, Window, &m_pSwapChain);
        }
        break;
#endif // VULKAN_SUPPORTED


        default:
            std::cerr << "Unknown/unsupported device type";
            return false;
            break;
    }

    if (m_pDevice == nullptr || m_pImmediateContext == nullptr || m_pSwapChain == nullptr)
        return false;

    return true;
}

void GLFWSample::GLFW_ResizeCallback(GLFWwindow* wnd, int w, int h)
{
    auto* Self = static_cast<GLFWSample*>(glfwGetWindowUserPointer(wnd));
    if (Self->m_pSwapChain != nullptr)
        Self->m_pSwapChain->Resize(static_cast<Uint32>(w), static_cast<Uint32>(h));
}

void GLFWSample::GLFW_KeyCallback(GLFWwindow* wnd, int key, int, int state, int)
{
    auto* Self = static_cast<GLFWSample*>(glfwGetWindowUserPointer(wnd));
    Self->OnKeyEvent(static_cast<Key>(key), static_cast<KeyState>(state));
}

void GLFWSample::GLFW_MouseButtonCallback(GLFWwindow* wnd, int button, int state, int)
{
    auto* Self = static_cast<GLFWSample*>(glfwGetWindowUserPointer(wnd));
    Self->OnKeyEvent(static_cast<Key>(button), static_cast<KeyState>(state));
}

void GLFWSample::GLFW_CursorPosCallback(GLFWwindow* wnd, double xpos, double ypos)
{
    auto* Self = static_cast<GLFWSample*>(glfwGetWindowUserPointer(wnd));
    Self->MouseEvent(float2(static_cast<float>(xpos), static_cast<float>(ypos)));
}

void GLFWSample::GLFW_MouseWheelCallback(GLFWwindow* wnd, double dx, double dy)
{
}

void GLFWSample::Loop()
{
    m_LastUpdate = TClock::now();
    for (;;)
    {
        if (glfwWindowShouldClose(m_Window))
            return;

        glfwPollEvents();

        for (auto KeyIter = m_ActiveKeys.begin(); KeyIter != m_ActiveKeys.end();)
        {
            KeyEvent(KeyIter->key, KeyIter->state);

            // GLFW does not send 'Repeat' state again, we have to keep these keys until the 'Release' is received.
            switch (KeyIter->state)
            {
                // clang-format off
                case KeyState::Release: KeyIter = m_ActiveKeys.erase(KeyIter); break;
                case KeyState::Press:   KeyIter->state = KeyState::Repeat;     break;
                case KeyState::Repeat:  ++KeyIter;                             break;
                    // clang-format on
            }
        }

        const auto time = TClock::now();
        const auto dt   = std::chrono::duration_cast<TSeconds>(time - m_LastUpdate).count();
        m_LastUpdate    = time;

        Update(dt);
    }
}

void GLFWSample::OnKeyEvent(Key key, KeyState newState)
{
    for (auto& active : m_ActiveKeys)
    {
        if (active.key == key)
        {
            if (newState == KeyState::Release)
                active.state = newState;

            return;
        }
    }

    m_ActiveKeys.push_back({key, newState});
}

void GLFWSample::Quit()
{
    VERIFY_EXPR(m_Window != nullptr);
    glfwSetWindowShouldClose(m_Window, GLFW_TRUE);
}

bool GLFWSample::ProcessCommandLine(const char* CmdLine, RENDER_DEVICE_TYPE& DevType)
{
    const auto* Key = "-mode ";
    const auto* pos = strstr(CmdLine, Key);
    if (pos != nullptr)
    {
        pos += strlen(Key);
        if (_stricmp(pos, "D3D11") == 0)
        {
#if D3D11_SUPPORTED
            DevType = RENDER_DEVICE_TYPE_D3D11;
#else
            std::cerr << "Direct3D11 is not supported. Please select another device type";
            return false;
#endif
        }
        else if (_stricmp(pos, "D3D12") == 0)
        {
#if D3D12_SUPPORTED
            DevType = RENDER_DEVICE_TYPE_D3D12;
#else
            std::cerr << "Direct3D12 is not supported. Please select another device type";
            return false;
#endif
        }
        else if (_stricmp(pos, "GL") == 0)
        {
#if GL_SUPPORTED
            DevType = RENDER_DEVICE_TYPE_GL;
#else
            std::cerr << "OpenGL is not supported. Please select another device type";
            return false;
#endif
        }
        else if (_stricmp(pos, "VK") == 0)
        {
#if VULKAN_SUPPORTED
            DevType = RENDER_DEVICE_TYPE_VULKAN;
#else
            std::cerr << "Vulkan is not supported. Please select another device type";
            return false;
#endif
        }
        else
        {
            std::cerr << "Unknown device type. Only the following types are supported: D3D11, D3D12, GL, VK";
            return false;
        }
    }
    else
    {
#if VULKAN_SUPPORTED
        DevType = RENDER_DEVICE_TYPE_VULKAN;
#elif D3D12_SUPPORTED
        DevType = RENDER_DEVICE_TYPE_D3D12;
#elif D3D11_SUPPORTED
        DevType = RENDER_DEVICE_TYPE_D3D11;
#elif GL_SUPPORTED
        DevType = RENDER_DEVICE_TYPE_GL;
#endif

        DevType = RENDER_DEVICE_TYPE_VULKAN;
    }
    return true;
}

int GLFWSampleMain(const char* cmdLine)
{
    std::unique_ptr<GLFWSample> Samp{CreateSample()};

    RENDER_DEVICE_TYPE DevType = RENDER_DEVICE_TYPE_UNDEFINED;
    if (!Samp->ProcessCommandLine(cmdLine, DevType))
        return -1;

    String Title("GLFW Demo");
    switch (DevType)
    {
        case RENDER_DEVICE_TYPE_D3D11: Title.append(" (D3D11)"); break;
        case RENDER_DEVICE_TYPE_D3D12: Title.append(" (D3D12)"); break;
        case RENDER_DEVICE_TYPE_GL: Title.append(" (GL)"); break;
        case RENDER_DEVICE_TYPE_VULKAN: Title.append(" (VK)"); break;
    }

    if (!Samp->CreateWindow(Title.c_str(), 1024, 768))
        return -1;

    if (!Samp->CreateEngine(DevType))
        return -1;

    if (!Samp->Initialize())
        return -1;

    Samp->Loop();

    return 0;
}
} // namespace Diligent


int main(int argc, const char** argv)
{
    return Diligent::GLFWSampleMain(argc >= 2 ? argv[1] : "");
}