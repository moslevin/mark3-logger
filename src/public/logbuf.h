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
  @file logbuf.h Threadsafe logging buffer implementation
 */
#pragma once

#include "logtypes.h"

#include <stdint.h>
#include <stddef.h>

//---------------------------------------------------------------------------
using LogNotification_t = void (*)();
using LogWrite_t = void (*)(const uint8_t* data_, size_t length_);

//---------------------------------------------------------------------------
/**
 * @brief The LogBuf class
 *
 * Implements a simple system logging buffer, conisting of a circular buffer
 * with read/write indexing.  Iimplementation ensures that in the worst case
 * scenario (i.e. cannot flush data out as fast as logging occurs), that no
 * adverse side effects will occur on the target -- although the client may
 * need to re-synchronize on the next valid packet.
 *
 * Guarantees thread/interrupt safety under certain conditions.
 */
class LogBuf {
public:

    /**
     * @brief Instance
     * @return Reference to the singleton LogBuf instance
     */
    static LogBuf& Instance();

    /**
     * @brief SetNotifyCallback
     *
     * Set a callback to be invoked whenever the object detects that a FIFO
     * rollover or half-rollover has occurred.  This is used to provide a means
     * for the application to call FlushData() on the object from the desired
     * context in a timely fashion (i.e., before the next rollover can occur).
     *
     * @param pfHandler_ Notification function to call on FIFO rollover/half-rollover
     */
    void SetNotifyCallback(LogNotification_t pfHandler_);

    /**
     * @brief SetLogWriter
     *
     * Set the function to call to write data payloads over the wire.
     *
     * @param pfLogWriter_ Function to call to write log data.
     */
    void SetLogWriter(LogWrite_t pfLogWriter_);

    /**
     * @brief WriteLog
     *
     * Write a complete logbuf header and associated TLV-format arguments to the buffer
     * with all associated headers.
     *
     * @param length_ length of data to write
     * @param header_ LogBuf log header
     * @param data TLV formatted argument data
     * @return Index to continue writing data at.
     */
    void WriteLog(int length_, const LogHeader_t* header_, const Tlv_t data[]);

    /**
     * @brief FlushData
     *
     * Flush all written data to the debug interface.
     */
    void FlushData();

private:

    /**
     * @brief BeginWrite
     *
     * Begin a new log - reserving the appropriate number of bytes in the buffer,
     * and returning an offset into the buffer at which logs will be written.
     *
     * Note: in a multithreaded application, or an application where logs are written
     * from ISRs, BeginWrite()/Write()/EndWrite() calls must be called from within
     * a single critical section.
     *
     * @param size_ Number of bytes to reserve in the buffer
     * @return Index to begin logging at
     */
    int BeginWrite(int size_);

    /**
     * @brief Write
     * Write a payload of arbitrary data to the log buffer
     *
     * Note: in a multithreaded application, or an application where logs are written
     * from ISRs, BeginWrite()/Write()/EndWrite() calls must be called from within
     * a critical section.
     *
     * @param idx_ Index at which to log in the buffer.
     * @param data_  Data to write to the logger
     * @param length_ Length of data (in bytes) to log
     *
     * @return Index to continue writing at
     */
    int Write(int idx_, const void* data_, uint8_t length_);

    /**
     * @brief EndWrite
     *
     * Signal the completion of a log-write operation.
     *
     * Note: in a multithreaded application, or an application where logs are written
     * from ISRs, BeginWrite()/Write()/EndWrite() calls must be called from within
     * a critical section.
     *
     * @param idx_ Index at which to log in the buffer.
     */
    void EndWrite(int idx_);

    static constexpr auto m_uBufferSize = 512;
    LogNotification_t m_pfNotificationHandler = nullptr;
    LogWrite_t m_pfLogWriter = nullptr;
    uint8_t m_buf[m_uBufferSize];
    int m_iWriteIdx = 0;
    int m_iReadIdx = 0;
    int m_iLastReadIdx = 0;
    bool m_bDoNotify = false;
    bool m_bPending = false;
    int m_iCount = 0;
};
