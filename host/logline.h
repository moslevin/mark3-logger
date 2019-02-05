#pragma once

#include <stdint.h>
#include <stdio.h>

#include "ll.h"

//---------------------------------------------------------------------------
class LogLine : public LinkListNode {
public:
    LogLine()
    : m_fileHash{0}
    , m_clTempLine{0}
    , m_szFormatString{nullptr}
    {}

    uint32_t	m_fileHash;
    uint32_t	m_clTempLine;
    char*		m_szFormatString;
};

//---------------------------------------------------------------------------
class LogLineList {
public:
    void AddLog(LogLine* pclLog_) {
        m_clList.Add(pclLog_);
    }

    void Serialize() {
        auto* node = m_clList.GetHead();
        printf("\"logLines\": [\n");
        while (node != nullptr) {
            auto* logNode = static_cast<LogLine*>(node);
            printf(" {\n");
            printf("    \"formatString\": \"%s\",\n", logNode->m_szFormatString);
            printf("    \"fileHash\": %u\n,", logNode->m_fileHash);
            printf("    \"fileLine\": %u\n", logNode->m_clTempLine);
            printf(" }");
            node = node->GetNext();
            if (node != nullptr) {
                printf(",");
            }
            printf("\n");
        }
        printf("]");
    }

private:
    DoubleLinkList m_clList;
};
