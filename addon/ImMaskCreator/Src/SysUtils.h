#pragma once
#include <cstdint>
#include <memory>
#include <mutex>

namespace SysUtils
{
enum class CpuFeature
{
    NONE,
    MMX,
    SSE,
    SSE2,
    SSE3,
    SSSE3,
    SSE4_1,
    SSE4_2,
    POPCNT,
    FP16,
    AVX,
    AVX2,
    FMA3,
    AVX_512F,
    AVX_512BW,
    AVX_512CD,
    AVX_512DQ,
    AVX_512ER,
    AVX_512IFMA,
    AVX_512PF,
    AVX_512VBMI,
    AVX_512VL,
    AVX_512VBMI2,
    AVX_512VNNI,
    AVX_512BITALG,
    AVX_512VPOPCNTDQ,
    AVX_5124VNNIW,
    AVX_5124FMAPS,
    AVX512_COMMON,
    AVX512_KNL,
    AVX512_KNM,
    AVX512_SKX,
    AVX512_CNL,
    AVX512_CLX,
    AVX512_ICL,
    NEON,
    MSA,
    RISCVV,
    VSX,
    VSX3,
    RVV,
};

struct CpuChecker
{
    static bool HasFeature(CpuFeature fea);
};

struct AsyncTask
{
    using Holder = std::shared_ptr<AsyncTask>;

    enum State
    {
        WAITING = 0,
        PROCESSING = 1,
        DONE = 2,
    };

    virtual void operator() () = 0;
    virtual bool SetState(State eState) = 0;
    virtual State GetState() const = 0;
    virtual bool IsWaiting() const = 0;
    virtual bool IsProcessing() const = 0;
    virtual bool IsDone() const = 0;
    virtual bool IsCancelled() const = 0;
    virtual void Cancel() = 0;
    virtual void WaitDone() = 0;
};

class BaseAsyncTask : public AsyncTask
{
public:
    bool SetState(State eState) override
    {
        std::lock_guard<std::mutex> lk(m_mtxLock);
        if (m_eState == eState)
        {
            return true;
        }
        else if (eState == WAITING)
        {
            if (m_eState == DONE)
            {
                m_bCannel = false;
                m_eState = eState;
                return true;
            }
        }
        else if (eState == PROCESSING)
        {
            if (m_eState == WAITING)
            {
                m_eState = eState;
                return true;
            }
        }
        else
        {
            m_eState = eState;
            return true;
        }
        return false;
    }

    void Cancel() override
    {
        std::lock_guard<std::mutex> lk(m_mtxLock);
        if (m_eState == DONE)
            return;
        m_eState = DONE;
        m_bCannel = true;
    }

    State GetState() const override
    { return m_eState; }
    bool IsWaiting() const override
    { return m_eState == WAITING; }
    bool IsProcessing() const override
    { return m_eState == PROCESSING; }
    bool IsDone() const override
    { return m_eState == DONE; }
    bool IsCancelled() const override
    { return m_bCannel; }
    void WaitDone() override;

protected:
    std::mutex m_mtxLock;
    State m_eState{WAITING};
    bool m_bCannel{false};
};

struct ThreadPoolExecutor
{
    using Holder = std::shared_ptr<ThreadPoolExecutor>;
    static Holder GetDefaultInstance();

    virtual bool EnqueueTask(AsyncTask::Holder hTask, bool bNonblock = false) = 0;
    virtual void SetMaxWaitingTaskCount(uint32_t cnt) = 0;
    virtual uint32_t GetWaitingTaskCount() const = 0;
    virtual void SetMaxThreadCount(uint32_t cnt) = 0;
    virtual uint32_t GetMaxThreadCount() const = 0;
    virtual void SetMinThreadCount(uint32_t cnt) = 0;
    virtual uint32_t GetMinThreadCount() const = 0;
    virtual void Terminate(bool bWaitAllTaskDone) = 0;
};
}