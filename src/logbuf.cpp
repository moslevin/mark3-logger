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
/*!
  @file logbuf.cpp Threadsafe logging buffer implementation
 */
#include "logbuf.h"

#include "mark3.h"

using namespace Mark3;

//---------------------------------------------------------------------------
namespace {
    constexpr auto syncBegin = 0xCAFE;
    constexpr auto syncEnd = 0xF00D;
    LogBuf clLogBuf;
} // anonymous namespace

//---------------------------------------------------------------------------
LogBuf& LogBuf::Instance()
{
    return clLogBuf;
}

//---------------------------------------------------------------------------
void LogBuf::SetNotifyCallback(LogNotification_t pfHandler_)
{
    m_pfNotificationHandler = pfHandler_;
}

//---------------------------------------------------------------------------
void LogBuf::SetLogWriter(LogWrite_t pfLogWriter_)
{
    m_pfLogWriter = pfLogWriter_;
}

//---------------------------------------------------------------------------
void LogBuf::WriteLog(int length_, const LogHeader_t* header_, const Tlv_t data_[])
{
    auto idx_ = BeginWrite(length_);
    idx_ = Write(idx_, header_, sizeof(LogHeader_t));
    for (auto i = 0; i < header_->log_count; i++) {
        idx_ = Write(idx_, &data_[i], sizeof(uint8_t) + data_[i].length);
    }
    EndWrite(idx_);
}

//---------------------------------------------------------------------------
int LogBuf::BeginWrite(int length_)
{
    int writeIdx;
    CS_ENTER();
    writeIdx = m_iWriteIdx;
    m_iWriteIdx += length_ + (2 * sizeof(uint16_t));
    if (m_iWriteIdx > m_uBufferSize) {
        m_iWriteIdx -= m_uBufferSize;
        m_bDoNotify = true;
    } else if ((writeIdx < (m_uBufferSize / 2)) && (m_iWriteIdx >= (m_uBufferSize / 2))) {
        m_bDoNotify = true;
    }
    m_iCount++;
    CS_EXIT();

    uint16_t sync = syncBegin;
    return Write(writeIdx, &sync, sizeof(sync));
}

//---------------------------------------------------------------------------
int LogBuf::Write(int idx_, const void* data, uint8_t length)
{
    auto src = (uint8_t*)data;
    auto dst = &m_buf[idx_];
    for (auto i = 0; i < length; i++, idx_++) {
        if (idx_ == m_uBufferSize) {
            idx_ = 0;
            dst = m_buf;
        }
        *dst++ = *src++;
    }
    return idx_;
}

//---------------------------------------------------------------------------
void LogBuf::EndWrite(int idx_)
{
    bool doNotify = false;

    uint16_t sync = syncEnd;
    idx_ = Write(idx_, &sync, sizeof(sync));

    CS_ENTER();
    if (m_iCount > 0) {
        m_iCount--;
        if (!m_iCount) {
            m_iReadIdx = m_iWriteIdx;
            m_bPending = true;
            if (m_bDoNotify) {
                doNotify = true;
            }
        }
    }
    CS_EXIT();

    if (doNotify && m_pfNotificationHandler) {        
        m_pfNotificationHandler();
    }
}

//---------------------------------------------------------------------------
void LogBuf::FlushData()
{
    int iReadIdx;
    int iLastReadIdx;
    bool bPending;

    CS_ENTER();
    bPending = m_bPending;
    m_bDoNotify = false;
    m_bPending = false;
    iReadIdx = m_iReadIdx;
    iLastReadIdx = m_iLastReadIdx;
    m_iLastReadIdx = m_iReadIdx;
    CS_EXIT();

    if (!m_pfLogWriter && !bPending) {
        return;
    }

    if (iReadIdx < iLastReadIdx) {
        m_pfLogWriter(&m_buf[iLastReadIdx], m_uBufferSize - iLastReadIdx);
        m_pfLogWriter(m_buf, iReadIdx);
    } else {
        m_pfLogWriter(&m_buf[iLastReadIdx], iReadIdx - iLastReadIdx);
    }
}
