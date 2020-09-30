#ifndef COMMON_H
#define COMMON_H

#include <string>
#include <iomanip>
#include <iostream>

#include <ethercattype.h>

namespace SOEMGui {

#define IO_MAP_SIZE 16384
#define SDO_READ_TIMEOUT 100000
#define SDO_WRITE_TIMEOUT 50000
#define EC_TIMEOUTMON 2000

namespace helper {
inline constexpr std::string_view ec_datatype_toString(ec_datatype dtype)
{
    switch(dtype)
    {
        case ECT_BOOLEAN:
            return "ETC_BOOLEAN";
        case ECT_INTEGER8:
            return "ECT_INTEGER8";
        case ECT_INTEGER16:
            return "ECT_INTEGER16";
        case ECT_INTEGER24:
            return "ECT_INTEGER24";
        case ECT_INTEGER32:
            return "ECT_INTEGER32";
        case ECT_INTEGER64:
            return "ECT_INTEGER64";
        case ECT_UNSIGNED8:
            return "ECT_UNSIGNED8";
        case ECT_UNSIGNED16:
            return "ECT_UNSIGNED16";
        case ECT_UNSIGNED24:
            return "ECT_UNSIGNED24";
        case ECT_UNSIGNED32:
            return "ECT_UNSIGNED32";
        case ECT_UNSIGNED64:
            return "ECT_UNSIGNED64";
        case ECT_REAL32:
            return "ECT_REAL32";
        case ECT_REAL64:
            return "ECT_REAL64";
        case ECT_BIT1:
            return "ECT_BIT1";
        case ECT_BIT2:
            return "ECT_BIT2";
        case ECT_BIT3:
            return "ECT_BIT3";
        case ECT_BIT4:
            return "ECT_BIT4";
        case ECT_BIT5:
            return "ECT_BIT5";
        case ECT_BIT6:
            return "ECT_BIT6";
        case ECT_BIT7:
            return "ECT_BIT7";
        case ECT_BIT8:
            return "ECT_BIT8";
        case ECT_VISIBLE_STRING:
            return "ECT_VISIBLE_STRING";
        case ECT_OCTET_STRING:
            return "ECT_OCTET_STRING";
        default:
            return "Unknown type";
    }
}

inline constexpr std::pair<long, long> ec_datatype_getLimits(ec_datatype dtype)
{
    switch(dtype)
    {
        case ECT_BOOLEAN:
            return std::make_pair(std::numeric_limits<bool>::min(),std::numeric_limits<bool>::max());
        case ECT_INTEGER8:
            return std::make_pair(std::numeric_limits<int8_t>::min(),std::numeric_limits<int8_t>::max());
        case ECT_INTEGER16:
            return std::make_pair(std::numeric_limits<int16_t>::min(),std::numeric_limits<int16_t>::max());
        case ECT_INTEGER24:
            return std::make_pair(0,0);
        case ECT_INTEGER32:
            return std::make_pair(std::numeric_limits<int32_t>::min(),std::numeric_limits<int32_t>::max());
        case ECT_INTEGER64:
            return std::make_pair(std::numeric_limits<int64_t>::min(),std::numeric_limits<int64_t>::max());
        case ECT_UNSIGNED8:
            return std::make_pair(std::numeric_limits<uint8_t>::min(),std::numeric_limits<uint8_t>::max());
        case ECT_UNSIGNED16:
            return std::make_pair(std::numeric_limits<uint16_t>::min(),std::numeric_limits<uint16_t>::max());
        case ECT_UNSIGNED24:
            return std::make_pair(0,0);
        case ECT_UNSIGNED32:
            return std::make_pair(std::numeric_limits<uint32_t>::min(),std::numeric_limits<uint32_t>::max());
        case ECT_UNSIGNED64:
            return std::make_pair(std::numeric_limits<uint64_t>::min(),std::numeric_limits<uint64_t>::max());
        default:
            return std::make_pair(0,0);
    }
}

inline std::string hex_toString(const uint32_t input, const unsigned precision = 8)
{
    std::ostringstream oss;
    oss << "0x" << std::setw(precision) << std::setfill('0') << std::hex << input;
    return oss.str();
}

inline std::string hex_toString(const uint16_t input, const unsigned precision = 4)
{
    std::ostringstream oss;
    oss << "0x" << std::setw(precision) << std::setfill('0') << std::hex << input;
    return oss.str();
}

inline std::string hex_toString(const uint8_t input, const unsigned precision = 2)
{
    std::ostringstream oss;
    oss << "0x" << std::setw(precision) << std::setfill('0') << std::hex << static_cast<uint16_t>(input);
    return oss.str();
}

template <typename T, typename... Rest>
inline void hash_combine(std::size_t& seed, const T& v, const Rest&... rest)
{
    seed ^= std::hash<T>{}(v) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
    (hash_combine(seed, rest), ...);
}
}
}

#endif // COMMON_H
