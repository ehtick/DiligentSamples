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

#pragma once

#include <vector>
#include <string>
#include <memory>

#include "NativeAppBase.hpp"
#include "RefCntAutoPtr.hpp"
#include "EngineFactory.h"
#include "RenderDevice.h"
#include "DeviceContext.h"
#include "SwapChain.h"
#include "SampleBase.hpp"
#include "ScreenCapture.hpp"
#include "Image.h"

namespace Diligent
{

class ImGuiImplDiligent;

class SampleApp : public NativeAppBase
{
public:
    SampleApp();
    ~SampleApp();
    virtual CommandLineStatus ProcessCommandLine(int argc, const char* const* argv) override final;

    virtual const char* GetAppTitle() const override final { return m_AppTitle.c_str(); }
    virtual void        Update(double CurrTime, double ElapsedTime) override;
    virtual void        WindowResize(int width, int height) override;
    virtual void        Render() override;
    virtual void        Present() override;
    virtual void        SelectDeviceType(){};

    virtual void GetDesiredInitialWindowSize(int& width, int& height) override final
    {
        width  = m_InitialWindowWidth;
        height = m_InitialWindowHeight;
    }

    virtual GoldenImageMode GetGoldenImageMode() const override final
    {
        return m_GoldenImgMode;
    }

    virtual int GetExitCode() const override final
    {
        return m_ExitCode;
    }

    virtual bool IsReady() const override final
    {
        return m_pDevice && m_pSwapChain && m_NumImmediateContexts > 0;
    }

    IDeviceContext* GetImmediateContext(size_t Ind = 0)
    {
        VERIFY_EXPR(Ind < m_NumImmediateContexts);
        return m_pDeviceContexts[Ind];
    }

protected:
    void InitializeDiligentEngine(const NativeWindow* pWindow);
    void InitializeSample();
    void UpdateAdaptersDialog();
    void UpdateAppSettings(bool IsInitialization);

    virtual void SetFullscreenMode(const DisplayModeAttribs& DisplayMode)
    {
        m_TheSample->ReleaseSwapChainBuffers();
        m_bFullScreenMode = true;
        m_pSwapChain->SetFullscreenMode(DisplayMode);
    }
    virtual void SetWindowedMode()
    {
        m_TheSample->ReleaseSwapChainBuffers();
        m_bFullScreenMode = false;
        m_pSwapChain->SetWindowedMode();
    }

    void CompareGoldenImage(const std::string& FileName, ScreenCapture::CaptureInfo& Capture);
    void SaveScreenCapture(const std::string& FileName, ScreenCapture::CaptureInfo& Capture);

    RENDER_DEVICE_TYPE                         m_DeviceType = RENDER_DEVICE_TYPE_UNDEFINED;
    RefCntAutoPtr<IEngineFactory>              m_pEngineFactory;
    RefCntAutoPtr<IRenderDevice>               m_pDevice;
    std::vector<RefCntAutoPtr<IDeviceContext>> m_pDeviceContexts;
    Uint32                                     m_NumImmediateContexts = 0;
    RefCntAutoPtr<ISwapChain>                  m_pSwapChain;
    GraphicsAdapterInfo                        m_AdapterAttribs;
    std::vector<DisplayModeAttribs>            m_DisplayModes;

    std::unique_ptr<SampleBase> m_TheSample;

    int          m_InitialWindowWidth  = 0;
    int          m_InitialWindowHeight = 0;
    int          m_ValidationLevel     = -1;
    std::string  m_AppTitle;
    Uint32       m_AdapterId   = DEFAULT_ADAPTER_ID;
    ADAPTER_TYPE m_AdapterType = ADAPTER_TYPE_UNKNOWN;
    std::string  m_AdapterDetailsString;
    int          m_SelectedDisplayMode      = 0;
    bool         m_bVSync                   = false;
    bool         m_bFullScreenMode          = false;
    bool         m_bShowAdaptersDialog      = true;
    bool         m_bShowUI                  = true;
    bool         m_bForceNonSeprblProgs     = false;
    bool         m_bVulkanCompatibilityMode = false;
    bool         m_bBreakOnError            = true;
    double       m_CurrentTime              = 0;
    Uint32       m_MaxFrameLatency          = SwapChainDesc{}.BufferCount;

    // We will need this when we have to recreate the swap chain (on Android)
    SwapChainDesc m_SwapChainInitDesc;

    struct ScreenCaptureInfo
    {
        bool              AllowCapture = false;
        std::string       Directory;
        std::string       FileName        = "frame";
        double            CaptureFPS      = 30;
        double            LastCaptureTime = -1e+10;
        Uint32            FramesToCapture = 0;
        Uint32            CurrentFrame    = 0;
        IMAGE_FILE_FORMAT FileFormat      = IMAGE_FILE_FORMAT_PNG;
        int               JpegQuality     = 95;
        bool              KeepAlpha       = false;

    } m_ScreenCaptureInfo;
    std::unique_ptr<ScreenCapture> m_pScreenCapture;

    std::unique_ptr<ImGuiImplDiligent> m_pImGui;

    GoldenImageMode m_GoldenImgMode           = GoldenImageMode::None;
    int             m_GoldenImgPixelTolerance = 0;
    int             m_ExitCode                = 0;
};

} // namespace Diligent
