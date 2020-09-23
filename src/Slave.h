#ifndef SLAVE_H
#define SLAVE_H

#include <array>
#include <vector>
#include <cstddef>
#include <string>
#include <variant>
#include <map>

#include "common.h"

namespace SOEMGui {
    struct PDOSubEntry
    {
        std::string name;
        uint16_t subIndex;
        ec_datatype datatype;
        uint16_t bitLength;
        uint16_t totalOffsetInBits;
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

    using PDOValueT = std::variant<int8_t, int16_t, int32_t, std::string>;

    class Slave
    {
        using InputT = std::array<std::byte, IO_MAP_SIZE>;
        using OutputT = std::array<std::byte, IO_MAP_SIZE>;
    public:
        Slave(unsigned int ID, std::string name, PDODescription pdo);
        void setInputs(void* inputPtr);
        void setOutputs(void* outputPtr);
        
        std::string toString() const;
        PDOValueT getOutputValue(const std::string_view pdoName, unsigned int subIndex) const;
        PDOValueT setInputValue(const PDOSubEntry& entry) const;

        std::vector<std::string> getInputPDONames() const;
        std::vector<std::string> getOutputPDONames() const;
        const PDOEntry &getInputPDOEntryRef(const std::string& name) const;
        const PDOEntry &getOutputPDOEntryRef(const std::string& name) const;
        std::string getName() const;

    private:
        bool validatePDODescription(const PDODescription& desc) const;

        InputT* m_inputs{nullptr};
        OutputT* m_outputs{nullptr};
        PDODescription m_pdo_description;
        std::map<std::string, std::map<std::string, const PDOSubEntry*>> m_input_name_name_to_sub_entries_map;
        std::map<std::string, std::map<uint16_t, const PDOSubEntry*>> m_input_name_id_to_sub_entries_map;
        std::map<std::string, std::map<std::string, const PDOSubEntry*>> m_output_name_name_to_sub_entries_map;
        std::map<std::string, std::map<uint16_t, const PDOSubEntry*>> m_output_name_id_to_sub_entries_map;

        unsigned int m_ID{0};
        std::string m_name;
    };

}

#endif // SLAVE_H
