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

#include <atomic>
#include <vector>
#include <thread>
#include <mutex>
#include "SampleBase.hpp"
#include "BasicMath.hpp"
#include "ThreadSignal.hpp"

namespace Diligent
{

class Tutorial09_Quads final : public SampleBase
{
public:
    ~Tutorial09_Quads() override;

    virtual CommandLineStatus ProcessCommandLine(int argc, const char* const* argv) override final;

    virtual void ModifyEngineInitInfo(const ModifyEngineInitInfoAttribs& Attribs) override final;

    virtual void Initialize(const SampleInitInfo& InitInfo) override final;

    virtual void Render() override final;
    virtual void Update(double CurrTime, double ElapsedTime, bool DoUpdateUI) override final;

    virtual const Char* GetSampleName() const override final { return "Tutorial09: Quads"; }

protected:
    virtual void UpdateUI() override final;

private:
    void CreatePipelineStates(std::vector<StateTransitionDesc>& Barriers);
    void LoadTextures(std::vector<StateTransitionDesc>& Barriers);

    void InitializeQuads();
    void CreateInstanceBuffer();
    void UpdateQuads(float elapsedTime);
    void StartWorkerThreads(size_t NumThreads);
    void StopWorkerThreads();
    template <bool UseBatch>
    void RenderSubset(IDeviceContext* pCtx, Uint32 Subset);

    static void WorkerThreadFunc(Tutorial09_Quads* pThis, Uint32 ThreadNum);

    Threading::Signal m_RenderSubsetSignal;
    Threading::Signal m_ExecuteCommandListsSignal;
    Threading::Signal m_GotoNextFrameSignal;

    std::atomic_int m_NumThreadsCompleted;
    std::atomic_int m_NumThreadsReady;

    std::vector<std::thread>                 m_WorkerThreads;
    std::vector<RefCntAutoPtr<ICommandList>> m_CmdLists;
    std::vector<ICommandList*>               m_CmdListPtrs;

    static constexpr int          NumStates = 5;
    RefCntAutoPtr<IPipelineState> m_pPSO[2][NumStates];
    RefCntAutoPtr<IBuffer>        m_QuadAttribsCB;
    RefCntAutoPtr<IBuffer>        m_BatchDataBuffer;

    static constexpr int                  NumTextures = 4;
    RefCntAutoPtr<IShaderResourceBinding> m_SRB[NumTextures];
    RefCntAutoPtr<IShaderResourceBinding> m_BatchSRB;
    RefCntAutoPtr<ITextureView>           m_TextureSRV[NumTextures];
    RefCntAutoPtr<ITextureView>           m_TexArraySRV;

    static constexpr int MaxQuads     = 100000;
    static constexpr int MaxBatchSize = 100;

    int m_NumQuads  = 1000;
    int m_BatchSize = 5;

    int m_MaxThreads       = 8;
    int m_NumWorkerThreads = 4;

    struct QuadData
    {
        float2 Pos;
        float2 MoveDir;
        float  Size       = 0;
        float  Angle      = 0;
        float  RotSpeed   = 0;
        int    TextureInd = 0;
        int    StateInd   = 0;
    };
    std::vector<QuadData> m_Quads;

    struct InstanceData
    {
        float4 QuadRotationAndScale;
        float2 QuadCenter;
        float  TexArrInd;
    };
};

} // namespace Diligent
