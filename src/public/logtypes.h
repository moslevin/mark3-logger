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
  @file logtypes.h Datatypes and definitions used for creating a TLV logger
 */
#pragma once
#include <stdint.h>
#include <stddef.h>

//---------------------------------------------------------------------------
// Define the width of the bitfields used to encode TLV headers
constexpr auto tag_bits = 4;
constexpr auto length_bits = 8 - tag_bits;

//---------------------------------------------------------------------------
// Enumeration describing the different types of argument data that are
// supported by the logging macros.
enum class LogTag {
    LogTagUint8,
    LogTagUint16,
    LogTagUint32,
    LogTagUint64,
    LogTagInt8,
    LogTagInt16,
    LogTagInt32,
    LogTagInt64,
    LogTagVoidptr,
    LogTagFloat,
    LogTagDouble,
    LogTagChar,
};

//---------------------------------------------------------------------------
// Enum used to implement a lookup table for "Tag" values in logging macros
// at compile-time.
enum class SizeTag : uint8_t {
    LogTagUint8 =     (sizeof(uint8_t)),
    LogTagUint16 =    (sizeof(uint16_t)),
    LogTagUint32 =    (sizeof(uint32_t)),
    LogTagUint64 =    (sizeof(uint64_t)),
    LogTagInt8 =      (sizeof(int8_t)),
    LogTagInt16 =     (sizeof(int16_t)),
    LogTagInt32 =     (sizeof(int32_t)),
    LogTagInt64 =     (sizeof(int64_t)),
    LogTagVoidptr =   (sizeof(void*)),
    LogTagFloat =     (sizeof(float)),
    LogTagDouble =    (sizeof(double)),
    LogTagChar =      (sizeof(char))
};

//---------------------------------------------------------------------------
// Generic type that can be used to represent any logging value based on a
// macro "Tag".
typedef union __attribute__((packed)) {
    uint8_t     v_TagUint8;
    uint16_t    v_TagUint16;
    uint32_t    v_TagUint32;
    uint64_t    v_TagUint64;
    int8_t      v_TagInt8;
    int16_t     v_TagInt16;
    int32_t     v_TagInt32;
    int64_t     v_TagInt64;
    void*       v_TagVoidptr;
    float       v_TagFloat;
    double      v_TagDouble;
    char        v_TagChar;
} LogVariant_t;

//---------------------------------------------------------------------------
// Tag/Length/Value structure.  This is packed such that the contents can be
// serialized directly from this struct, without incurring any waste due to
// unused/alignment bytes.  The tag/length can further be used to determine
// how many "value" bytes need to be are used.
typedef struct __attribute__((packed)) {
    unsigned int tag        : tag_bits;
    unsigned int length     : length_bits;
    LogVariant_t value;
} Tlv_t;

//---------------------------------------------------------------------------
// Struct containing data sufficient for encoding/storing a line of log data
// prior to transmission.
typedef struct __attribute__((packed)) {
    uint32_t file_id;
    uint32_t timestamp;
    uint16_t line;
    uint8_t log_count;
} LogHeader_t;
