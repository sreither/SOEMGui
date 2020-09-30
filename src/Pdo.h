#ifndef PDO_H
#define PDO_H

#include <array>
#include <vector>
#include <variant>

#include <ethercattype.h>

namespace SOEMGui {
    enum EntryType
    {
        Input,
        Output
    };

    struct PDOSubEntry
    {
        std::string name;
        uint16_t subIndex;
        ec_datatype datatype;
        uint16_t bitLength;
        EntryType direction;
        uint16_t totalOffsetInBits;

        struct PDOSubEntryHash
        {
            std::size_t operator()(uint16_t slaveId, EntryType inputOutput, uint16_t pdoIndex, uint16_t pdoSubIndex) const noexcept
            {
                std::size_t h=0;
                helper::hash_combine(h, slaveId, inputOutput, pdoIndex, pdoSubIndex);
                return h;
            }
        };
        std::size_t hash;
    };

    struct PDOEntry
    {
        std::string name;
        uint16_t index;
        std::vector<PDOSubEntry> entries;
    };

    struct PDODescription
    {
        std::vector<PDOEntry> slaveOutputs;
        std::vector<PDOEntry> slaveInputs;
    };

    using PDOValueT = std::variant<int8_t, int16_t, int32_t, int64_t, uint8_t, uint16_t, uint32_t, uint64_t, std::string>;

    template<class... Ts> struct overload : Ts... { using Ts::operator()...; };
    template<class... Ts> overload(Ts...) -> overload<Ts...>;
    inline std::string PDOValue_toString(PDOValueT value)
    {
        std::string r;
        std::visit(overload{
                       [&](int8_t& v)       { r = std::to_string(v);},
                       [&](int16_t& v)      { r = std::to_string(v);},
                       [&](int32_t& v)      { r = std::to_string(v);},
                       [&](int64_t& v)      { r = std::to_string(v);},
                       [&](uint8_t& v)       { r = std::to_string(v);},
                       [&](uint16_t& v)      { r = std::to_string(v);},
                       [&](uint32_t& v)      { r = std::to_string(v);},
                       [&](uint64_t& v)      { r = std::to_string(v);},
                       [&](auto&& v)         { r = "Unknown value type";}
                   }, value);
        return r;
    }

    template <typename T,
              typename = typename std::enable_if<std::is_integral<T>::value, T>::type>
    inline PDOValueT PDOValue_fromIntegralType(T value, ec_datatype type)
    {
        auto [min, max] = helper::ec_datatype_getLimits(type);
        if (min > value || max < value)
        {
            throw std::logic_error("PDOValue_fromIntegralType: unable to convert value: "
                                   + std::to_string(value)
                                   + " for type "
                                   + std::string(helper::ec_datatype_toString(type)));
        }
        switch(type)
        {
            case ECT_BOOLEAN:
                return static_cast<bool>(value);
            case ECT_INTEGER8:
                return static_cast<int8_t>(value);
            case ECT_INTEGER16:
                return static_cast<int16_t>(value);
            case ECT_INTEGER24:
                return static_cast<int32_t>(value);
            case ECT_INTEGER32:
                return static_cast<int32_t>(value);
            case ECT_INTEGER64:
                return static_cast<int64_t>(value);
            case ECT_UNSIGNED8:
                return static_cast<uint8_t>(value);
            case ECT_UNSIGNED16:
                return static_cast<uint16_t>(value);
            case ECT_UNSIGNED24:
                return static_cast<uint32_t>(value);
            case ECT_UNSIGNED32:
                return static_cast<uint32_t>(value);
            case ECT_UNSIGNED64:
                return static_cast<uint64_t>(value);
            default:
                return PDOValueT();
        }
    }
}

#endif // PDO_H
