#ifndef SLAVE_H
#define SLAVE_H

#include <array>
#include <vector>
#include <cstddef>
#include <string>
#include <variant>
#include <map>
#include <unordered_map>

#include "common.h"

namespace SOEMGui {

    struct PDOSubEntry
    {
        std::string name;
        uint16_t subIndex;
        ec_datatype datatype;
        uint16_t bitLength;
        uint16_t totalOffsetInBits;


        struct PDOSubEntryHash
        {
            std::size_t operator()(uint16_t slaveId, uint8_t inputOutput, uint16_t pdoIndex, uint16_t pdoSubIndex) const noexcept
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

    class Slave
    {
        using InputT = std::array<std::byte, IO_MAP_SIZE>;
        using OutputT = std::array<std::byte, IO_MAP_SIZE>;
    public:
        Slave(unsigned int ID, std::string name, PDODescription pdo);
        void setInputs(void* inputPtr);
        void setOutputs(void* outputPtr);
        
        bool hasEntry(std::size_t hash) const;
        std::vector<std::size_t> getAllPDOSubEntryHashes() const;

        std::string toString() const;
        PDOValueT getOutputValue(const std::string_view pdoName, unsigned int subIndex) const;
        PDOValueT getOutputValue(std::size_t hash) const;
        PDOValueT setInputValue(const PDOSubEntry& entry) const;

        std::vector<std::string> getInputPDONames() const;
        std::vector<std::string> getOutputPDONames() const;
        const PDOEntry &getInputPDOEntryRef(const std::string& name) const;
        const PDOEntry &getOutputPDOEntryRef(const std::string& name) const;
        std::string getName() const;  

        std::string currentOutputsToString() const;

    private:
        PDOValueT getOutputValue(const PDOSubEntry* entry) const;
        bool validatePDODescription(const PDODescription& desc) const;

        InputT* m_inputs{nullptr};
        OutputT* m_outputs{nullptr};
        PDODescription m_pdo_description;
        std::unordered_map<std::string, std::unordered_map<std::string, const PDOSubEntry*>> m_input_name_name_to_sub_entries_map;
        std::unordered_map<std::string, std::unordered_map<uint16_t, const PDOSubEntry*>> m_input_name_id_to_sub_entries_map;
        std::unordered_map<std::string, std::unordered_map<std::string, const PDOSubEntry*>> m_output_name_name_to_sub_entries_map;
        std::unordered_map<std::string, std::unordered_map<uint16_t, const PDOSubEntry*>> m_output_name_id_to_sub_entries_map;
        std::unordered_map<std::size_t, const PDOSubEntry*> m_hash_to_entry_map;
        std::vector<std::size_t> m_all_hashes;

        unsigned int m_ID{0};
        std::string m_name;
    };

}

#endif // SLAVE_H
