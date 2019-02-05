/*===========================================================================
     _____        _____        _____        _____
 ___|    _|__  __|_    |__  __|__   |__  __| __  |__  ______
|    \  /  | ||    \      ||     |     ||  |/ /     ||___   |
|     \/   | ||     \     ||     \     ||     \     ||___   |
|__/\__/|__|_||__|\__\  __||__|\__\  __||__|\__\  __||______|
    |_____|      |_____|      |_____|      |_____|

--[Mark3 Realtime Platform]--------------------------------------------------

Copyright (c) 2019 m0slevin, all rights reserved.
See license.txt for more information
=========================================================================== */

#include "mark3.h"

#include <stdint.h>
#include <stddef.h>

#include "logbuf.h"
#include "logmacro.h"
#include "memutil.h"

extern "C" {
void __cxa_guard_acquire() {}
void __cxa_guard_release() {}
}

//---------------------------------------------------------------------------
// This call generates .elf file metadata in the .logger section, allowing
// logs from this file to be identified as coming from this file.
EMIT_DBG_HEADER();

//---------------------------------------------------------------------------
extern "C" {
    extern void DebugPrint(const char* szString_);
}

//---------------------------------------------------------------------------
using namespace Mark3;
namespace {
    // Synchronization object used to wake the logger thread upon rollover/
    // half-rollover conditions in the logger's circular buffer
    Notify clLogNotify;

    // Thread objects
    Thread clAppThread;
    Thread clIdleThread;
    Thread clLogThread;

    // Threa stacks
    K_WORD wAppStack[PORT_KERNEL_DEFAULT_STACK_SIZE];
    K_WORD wIdleStack[PORT_KERNEL_DEFAULT_STACK_SIZE];
    K_WORD wLogStack[PORT_KERNEL_DEFAULT_STACK_SIZE];

    // Main application thread entrypoint -- print logs to the logger
    void AppTask(void* unused_)
    {
        int16_t counter = 0;
        while (1) {
            Thread::Sleep(10);

            // At compile time:
            //
            // Emits the format string to the .logger section of the .elf file, along
            // with the file's hash generated from EMIT_DBG_HEADER(), and the __LINE__.
            // This is enough metadata to uniquely identify the string in the program.
            //
            // At runtime:
            //
            // Write metadata consisting of the file hash and __LINE__, as well as any
            // argument data to the global LogBuf object.

            DEBUG_LOG("Testing0");

            Thread::Sleep(10);

            DEBUG_LOG("Testing: %d\n", TagInt16, counter++);
        }
    }

    // Idle task
    void IdleTask(void* unused_)
    {
        while (1) {
            /* Do Nothing */
        }
    }

    // Logging task - flushes log buffer contents over a physical interface
    void LogTask(void* unused_)
    {
        while (1) {
            // Wait until a 100ms timer has elapsed, or the
            // log buffer hits 50% or rolls over.  Then flush the
            // logging data out the debug interface
            clLogNotify.Wait(100, nullptr);
            auto &logBuf = LogBuf::Instance();
            logBuf.FlushData();
        }
    }

    void OnLogNotify()
    {
        clLogNotify.Signal();
    }

    void LogWriter(const uint8_t* data_, size_t length_)
    {
        char tmp[4];
        for (size_t i = 0; i < length_; i++) {
            MemUtil::DecimalToHex(data_[i], tmp);
            DebugPrint((const char*)tmp);
            DebugPrint(" ");
        }
        DebugPrint("\n");
    }

} // anonymous namespace

//---------------------------------------------------------------------------
int main()
{
    Kernel::Init();

    // Set the functions to be used by the application
    auto& logBuf = LogBuf::Instance();
    logBuf.SetNotifyCallback(OnLogNotify);
    logBuf.SetLogWriter(LogWriter);

    // Initialize the main application thread that logs data to the logger
    clAppThread.Init(wAppStack, sizeof(wAppStack), 2, AppTask, nullptr);
    clAppThread.Start();

    // Initialize the idle thread
    clIdleThread.Init(wIdleStack, sizeof(wIdleStack), 0, IdleTask, nullptr);
    clIdleThread.Start();

    // Initialize the thread that dumps queued log data to a physical interface
    clLogThread.Init(wLogStack, sizeof(wLogStack), 1, LogTask, nullptr);
    clLogThread.Start();

    Kernel::Start();
	return 0;
}
