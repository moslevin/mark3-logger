#pragma once

#include <stdint.h>
#include <stdio.h>

#include "ll.h"

class FileMap : public LinkListNode {
public:
    FileMap()
    : filename{nullptr}
    , m_fileHash{0}
    {}
    char*		filename;
    uint32_t	m_fileHash;
};

class FileMapList {
public:
    void AddFile(FileMap* pclFile_) {
        m_clList.Add(pclFile_);
    }

    void Serialize() {
        auto* node = m_clList.GetHead();
        printf("\"fileMap\": [\n");
        while (node != nullptr) {
            auto* mapNode = static_cast<FileMap*>(node);
            printf(" {\n");
            printf("    \"fileName\": \"%s\",\n", mapNode->filename);
            printf("    \"fileHash\": %u\n", mapNode->m_fileHash);
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
