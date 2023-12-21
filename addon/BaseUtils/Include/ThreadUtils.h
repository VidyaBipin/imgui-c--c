/*
    Copyright (c) 2023 CodeWin

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU Lesser General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

#pragma once
#include <cstdint>
#include <string>
#include <thread>
#include <mutex>
#include <memory>
#include "BaseUtilsCommon.h"
#include "Logger.h"

namespace SysUtils
{
BASEUTILS_API void SetThreadName(std::thread& t, const std::string& name);

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
    virtual void WaitState(State eState, int64_t u64TimeOut = 0) = 0;
};

class BaseAsyncTask : public AsyncTask
{
public:
    void operator() () override
    {
        _TaskProc();
        SetState(DONE);
    }

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
                m_bCancel = false;
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
        m_bCancel = true;
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
    { return m_bCancel; }
    void WaitDone() override;
    void WaitState(State eState, int64_t i64TimeOut = 0) override;

protected:
    virtual void _TaskProc() = 0;

protected:
    std::mutex m_mtxLock;
    State m_eState{WAITING};
    bool m_bCancel{false};
};

struct ThreadPoolExecutor
{
    using Holder = std::shared_ptr<ThreadPoolExecutor>;
    static Holder GetDefaultInstance();
    static void ReleaseDefaultInstance();
    static Holder CreateInstance(const std::string& name);

    virtual bool EnqueueTask(AsyncTask::Holder hTask, bool bNonblock = false) = 0;
    virtual void SetMaxWaitingTaskCount(uint32_t cnt) = 0;
    virtual uint32_t GetWaitingTaskCount() const = 0;
    virtual void SetMaxThreadCount(uint32_t cnt) = 0;
    virtual uint32_t GetMaxThreadCount() const = 0;
    virtual void SetMinThreadCount(uint32_t cnt) = 0;
    virtual uint32_t GetMinThreadCount() const = 0;
    virtual void Terminate(bool bWaitAllTaskDone) = 0;

    virtual void SetLoggerLevel(Logger::Level l) = 0;
};
}
