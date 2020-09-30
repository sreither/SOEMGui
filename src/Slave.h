#ifndef SLAVE_H
#define SLAVE_H

#include <array>
#include <vector>
#include <string>
#include <unordered_map>

#include "common.h"
#include "Pdo.h"

namespace SOEMGui {
    class Slave
    {
        using ValueArrayT = std::array<std::byte, IO_MAP_SIZE>;
    public:
        Slave(unsigned int ID, std::string name, PDODescription pdo);
        void setInputs(void* inputPtr);
        void setOutputs(void* outputPtr);
        
        bool hasEntry(std::size_t hash) const;
        std::vector<std::size_t> getAllPDOSubEntryHashes() const;

        std::string toString() const;
        PDOValueT getOutputValue(const std::string_view pdoName, unsigned int subIndex) const;
        PDOValueT getValue(std::size_t hash) const;
        bool setInputValue(const std::string_view pdoName, unsigned int subIndex, PDOValueT value);
        bool setInputValue(std::size_t hash, PDOValueT value);

        std::vector<std::string> getInputPDONames() const;
        std::vector<std::string> getOutputPDONames() const;
        const PDOEntry &getInputPDOEntryRef(const std::string& name) const;
        const PDOEntry &getOutputPDOEntryRef(const std::string& name) const;
        std::string getName() const;  

        std::string currentOutputsToString() const;

    private:
        PDOValueT getValue(const PDOSubEntry* entry) const;
        bool setInputValue(const PDOSubEntry* entry, PDOValueT value);
        bool validatePDODescription(const PDODescription& desc) const;

        ValueArrayT* m_inputs{nullptr};
        ValueArrayT* m_outputs{nullptr};
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
