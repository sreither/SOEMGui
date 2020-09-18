#ifndef COMMON_H
#define COMMON_H

#include <string>
#include <iomanip>
#include <ostream>
#include <iostream>

#include <ethercattype.h>

namespace SOEMGui {

#define IO_MAP_SIZE 16384
#define SDO_READ_TIMEOUT 100000
#define SDO_WRITE_TIMEOUT 50000
#define EC_TIMEOUTMON 2000

namespace helper {
inline std::string ec_datatype_toString(ec_datatype dtype)
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

}
}

#endif // COMMON_H
