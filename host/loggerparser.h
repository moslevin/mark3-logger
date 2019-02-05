#pragma once

#include "filemap.h"
#include "ll.h"
#include "logline.h"

constexpr auto TOKEN_LOG_END = (0xD00D);
constexpr auto TOKEN_LOG_START = (0xCAFE);

constexpr auto TOKEN_FILE_END = (0xABBA);
constexpr auto TOKEN_FILE_START	= (0xACDC);

enum class ParseState {
    Begin,
    LogBegin,
    LogString,
    LogLine,
    LogHash,
    LogEnd,
    FileBegin,
    FileName,
    FileHash,
    FileEnd
};

class LoggerParser {
public:
    LoggerParser(const char* szPath_);

    bool Init();
    bool Parse();
    void Serialize();

private:

    bool BeginHandler();
    bool LogBeginHandler();
    bool LogStringHandler();
    bool LogLineHandler();
    bool LogHashHandler();
    bool LogEndHandler();
    bool FileBeginHandler();
    bool FileNameHandler();
    bool FileHashHandler();
    bool FileEndHandler();

    int m_fd;
    int m_idx;

    bool m_bInit;
    const char* m_szPath;
    ParseState m_eParseState;

    FileMap		m_clTempMap;
    LogLine		m_clTempLine;

    FileMapList m_clFileMapList;
    LogLineList m_clLogLineList;

    bool m_bDirection;
};

