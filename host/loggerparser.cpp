#include "loggerparser.h"

#include "filemap.h"
#include "ll.h"
#include "logline.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

//---------------------------------------------------------------------------
LoggerParser::LoggerParser(const char* szPath_)
: m_szPath{szPath_}
, m_bInit{false}
, m_idx{0}
, m_fd{0}
{}

//---------------------------------------------------------------------------
bool LoggerParser::Init()
{
    if (m_bInit) {
        return true;
    }

    m_fd = open(m_szPath, O_RDONLY);
    if (m_fd < 0) {
        return false;
    }

    m_bInit = true;
    return true;
}

//---------------------------------------------------------------------------
bool LoggerParser::Parse() {
    if (!m_bInit) {
        return false;
    }

    m_clTempMap.ClearNode();
    m_clTempLine.ClearNode();
    m_eParseState = ParseState::Begin;
    while (true) {
        switch (m_eParseState) {
            case ParseState::Begin: 	{ if (!BeginHandler()) 		{ return true; } } break;
            case ParseState::LogBegin: 	{ if (!LogBeginHandler()) 	{ return true; } } break;
            case ParseState::LogString: { if (!LogStringHandler())	{ return true; } } break;
            case ParseState::LogLine: 	{ if (!LogLineHandler()) 	{ return true; } } break;
            case ParseState::LogHash: 	{ if (!LogHashHandler())	{ return true; } } break;
            case ParseState::LogEnd: 	{ if (!LogEndHandler())		{ return true; } } break;
            case ParseState::FileBegin: { if (!FileBeginHandler())	{ return true; } } break;
            case ParseState::FileName: 	{ if (!FileNameHandler()) 	{ return true; } } break;
            case ParseState::FileHash: 	{ if (!FileHashHandler())	{ return true; } } break;
            case ParseState::FileEnd: 	{ if (!FileEndHandler())	{ return true; } } break;
            default:
                return false;
        }
    }
    return true;
}

//---------------------------------------------------------------------------
void LoggerParser::Serialize() {
    m_clFileMapList.Serialize();
    printf(",\n");
    m_clLogLineList.Serialize();
}

//---------------------------------------------------------------------------
bool LoggerParser::BeginHandler()
{
    uint16_t token;
    if (read(m_fd, &token, sizeof(token)) <= 0) {
        return false;
    }
    m_idx += sizeof(token);
    if (token == TOKEN_LOG_START) {
        m_eParseState = ParseState::LogBegin;
        m_bDirection = true;
    }
    else if (token == TOKEN_LOG_END) {
        m_eParseState = ParseState::LogBegin;
        m_bDirection = false;
    }
    else if (token == TOKEN_FILE_START) {
        m_eParseState = ParseState::FileBegin;
        m_bDirection = true;
    }
    else if (token == TOKEN_FILE_END) {
        m_eParseState = ParseState::FileBegin;
        m_bDirection = false;
    }
    return true;
}

//---------------------------------------------------------------------------
bool LoggerParser::LogBeginHandler()
{
    uint8_t token;
    do {
        if (read(m_fd, &token, sizeof(token)) <= 0) {
            return false;
        }
        m_idx++;
    } while (token == 0);
    lseek(m_fd, -1, SEEK_CUR);
    m_idx--;
    if (m_bDirection == false) {
        m_eParseState = ParseState::LogHash;
    } else {
        m_eParseState = ParseState::LogString;
    }
    return true;
}

//---------------------------------------------------------------------------
bool LoggerParser::LogStringHandler()
{
    char c;
    char tmp[256] = {0};
    char* buf = tmp;
    do {
        int nr = read(m_fd, &c, sizeof(c));
        if (nr <= 0) {
            return false;
        }
        m_idx++;
        *buf++ = c;
    } while (c != 0);
    if (m_clTempLine.m_szFormatString) {
        free(m_clTempLine.m_szFormatString);
    }
    m_clTempLine.m_szFormatString = strdup(tmp);

    // uint16_t alignment.
    if ((m_idx % 2) != 0) {
        if (read(m_fd, &c, sizeof(c)) <= 0) {
            return false;
        }
        m_idx++;
    }

    if (m_bDirection == false) {
        m_eParseState = ParseState::LogEnd;
        auto* newLogNode = new LogLine();
        newLogNode->ClearNode();
        newLogNode->m_szFormatString = strdup(m_clTempLine.m_szFormatString);
        newLogNode->m_fileHash = m_clTempLine.m_fileHash;
        newLogNode->m_clTempLine = m_clTempLine.m_clTempLine;
        m_clLogLineList.AddLog(newLogNode);
    } else {
        m_eParseState = ParseState::LogLine;
    }
    return true;
}

//---------------------------------------------------------------------------
bool LoggerParser::LogLineHandler()
{
    uint16_t token;
    do {
        if (read(m_fd, &token, sizeof(token)) <= 0) {
            return false;
        }
        m_idx += sizeof(token);
    } while (token == 0);

    m_clTempLine.m_clTempLine = token;
    char c;
    while ((m_idx % 4) != 0) {
        if (read(m_fd, &c, sizeof(c)) <= 0) {
            return false;
        }
        m_idx++;
    }

    if (m_bDirection == false) {
        m_eParseState = ParseState::LogString;
    } else {
        m_eParseState = ParseState::LogHash;
    }
    return true;
}

//---------------------------------------------------------------------------
bool LoggerParser::LogHashHandler()
{
    uint32_t token;
    do {
        if (read(m_fd, &token, sizeof(token)) <= 0) {
            return false;
        }
        m_idx += sizeof(token);
    } while (token == 0);
    m_clTempLine.m_fileHash = token;

    if (m_bDirection == false) {
        m_eParseState = ParseState::LogString;
    } else {
        m_eParseState = ParseState::LogEnd;
        auto* newLogNode = new LogLine();
        newLogNode->ClearNode();
        newLogNode->m_szFormatString = strdup(m_clTempLine.m_szFormatString);
        newLogNode->m_fileHash = m_clTempLine.m_fileHash;
        newLogNode->m_clTempLine = m_clTempLine.m_clTempLine;
        m_clLogLineList.AddLog(newLogNode);
    }
    return true;
}

//---------------------------------------------------------------------------
bool LoggerParser::LogEndHandler()
{
    uint16_t token;
    if (read(m_fd, &token, sizeof(token)) <= 0) {
        return false;
    }
    m_idx += sizeof(token);
    if (token == TOKEN_LOG_START) {
        m_eParseState = ParseState::Begin;
    }
    else if (token == TOKEN_LOG_END) {
        m_eParseState = ParseState::Begin;
    }
    else if (token == TOKEN_FILE_START) {
        m_eParseState = ParseState::Begin;
    }
    else if (token == TOKEN_FILE_END) {
        m_eParseState = ParseState::Begin;
    }
    return true;
}

//---------------------------------------------------------------------------
bool LoggerParser::FileBeginHandler()
{
    uint8_t token;
    do {
        if (read(m_fd, &token, sizeof(token)) <= 0) {
            return false;
        }
        m_idx++;
    } while (token == 0);
    lseek(m_fd, -1, SEEK_CUR);
    m_idx--;
    if (m_bDirection == true) {
        m_eParseState = ParseState::FileHash;
    } else {
        m_eParseState = ParseState::FileName;
    }
    return true;
}

//---------------------------------------------------------------------------
bool LoggerParser::FileNameHandler()
{
    char c;
    char tmp[256] = {0};
    char* buf = tmp;
    do {
        int nr = read(m_fd, &c, sizeof(c));
        if (nr <= 0) {
            return false;
        }
        m_idx++;
        *buf++ = c;
    } while (c != 0);
    if (m_clTempMap.filename) {
        free(m_clTempMap.filename);
    }
    m_clTempMap.filename = strdup(tmp);

    // uint32_t alignment.
    while ((m_idx % 4) != 0) {
        read(m_fd, &c, sizeof(c));
        m_idx++;
    }

    if (m_bDirection == true) {
        m_eParseState = ParseState::FileEnd;
        auto* newMapNode = new FileMap();
        newMapNode->ClearNode();
        newMapNode->filename = strdup(m_clTempMap.filename);
        newMapNode->m_fileHash = m_clTempMap.m_fileHash;
        m_clFileMapList.AddFile(newMapNode);
    } else {
        m_eParseState = ParseState::FileHash;
    }
    return true;
}

//---------------------------------------------------------------------------
bool LoggerParser::FileHashHandler()
{
    uint32_t token;
    do {
        if (read(m_fd, &token, sizeof(token)) <= 0) {
            return false;
        }
        m_idx += sizeof(token);
    } while (token == 0);
    m_clTempMap.m_fileHash = token;

    if (m_bDirection == true) {
        m_eParseState = ParseState::FileName;
    } else {
        m_eParseState = ParseState::FileEnd;
        auto* newMapNode = new FileMap();
        newMapNode->ClearNode();
        newMapNode->filename = strdup(m_clTempMap.filename);
        newMapNode->m_fileHash = m_clTempMap.m_fileHash;
        m_clFileMapList.AddFile(newMapNode);
    }
    return true;
}

//---------------------------------------------------------------------------
bool LoggerParser::FileEndHandler() {
    uint16_t token;
    if (read(m_fd, &token, sizeof(token)) <= 0) {
        return false;
    }
    m_idx += sizeof(token);
    if (token == TOKEN_LOG_START) {
        m_eParseState = ParseState::LogBegin;
        m_bDirection = true;
    }
    else if (token == TOKEN_LOG_END) {
        m_eParseState = ParseState::LogBegin;
        m_bDirection = false;
    }
    else if (token == TOKEN_FILE_START) {
        m_eParseState = ParseState::FileBegin;
        m_bDirection = true;
    }
    else if (token == TOKEN_FILE_END) {
        m_eParseState = ParseState::FileBegin;
        m_bDirection = false;
    }
    else {
        m_eParseState = ParseState::Begin;
    }
    return true;
}
