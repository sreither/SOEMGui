#include "Slave.h"

#include <ios>
#include <iomanip>
#include <sstream>
#include <iostream>
#include <cstddef>

using namespace SOEMGui;

Slave::Slave(unsigned int ID, std::string name, PDODescription pdo) :
    m_ID(ID),
    m_name(std::move(name)),
    m_pdo_description(std::move(pdo))
{
    if (!validatePDODescription(m_pdo_description))
    {
        throw std::logic_error("PDO for Slave '" + m_name + "' (ID: '" + std::to_string(m_ID) + "') ist not valid!");
    }
    else
    {
        for (const auto& entry : m_pdo_description.slaveOutputs)
        {
            m_output_name_name_to_sub_entries_map.insert(
                        std::make_pair<std::string, std::unordered_map<std::string, const PDOSubEntry*>>(std::string(entry.name), std::unordered_map<std::string, const PDOSubEntry*>()));
            m_output_name_id_to_sub_entries_map.insert(
                        std::make_pair<std::string, std::unordered_map<uint16_t, const PDOSubEntry*>>(std::string(entry.name), std::unordered_map<uint16_t, const PDOSubEntry*>()));

            for (const auto & subEntry : entry.entries)
            {
                m_output_name_name_to_sub_entries_map.at(entry.name)[subEntry.name] = &subEntry;
                m_output_name_id_to_sub_entries_map.at(entry.name)[subEntry.subIndex] = &subEntry;
                m_hash_to_entry_map[subEntry.hash] = &subEntry;
                m_all_hashes.push_back(subEntry.hash);
            }
        }

        for (const auto& entry : m_pdo_description.slaveInputs)
        {
            m_input_name_name_to_sub_entries_map.insert(
                        std::make_pair<std::string, std::unordered_map<std::string, const PDOSubEntry*>>(std::string(entry.name), std::unordered_map<std::string, const PDOSubEntry*>()));
            m_input_name_id_to_sub_entries_map.insert(
                        std::make_pair<std::string, std::unordered_map<uint16_t, const PDOSubEntry*>>(std::string(entry.name), std::unordered_map<uint16_t, const PDOSubEntry*>()));

            for (const auto & subEntry : entry.entries)
            {
                m_input_name_name_to_sub_entries_map.at(entry.name)[subEntry.name] = &subEntry;
                m_input_name_id_to_sub_entries_map.at(entry.name)[subEntry.subIndex] = &subEntry;
                m_hash_to_entry_map[subEntry.hash] = &subEntry;
                m_all_hashes.push_back(subEntry.hash);
            }
        }
    }
}

void Slave::setInputs(void *inputPtr)
{
    m_inputs = static_cast<ValueArrayT*>(inputPtr);
}

void Slave::setOutputs(void *outputPtr)
{
    m_outputs = static_cast<ValueArrayT*>(outputPtr);
}

bool Slave::hasEntry(std::size_t hash) const
{
    return m_hash_to_entry_map.count(hash);
}

std::vector<std::size_t> Slave::getAllPDOSubEntryHashes() const
{
    return m_all_hashes;
}

std::vector<std::string> Slave::getInputPDONames() const
{
    std::vector<std::string> vec;
    vec.reserve(m_pdo_description.slaveInputs.size());

    for (const auto& entry : m_pdo_description.slaveInputs)
    {
        vec.emplace_back(entry.name);
    }
    return vec;
}

std::vector<std::string> Slave::getOutputPDONames() const
{
    std::vector<std::string> vec;
    vec.reserve(m_pdo_description.slaveOutputs.size());

    for (const auto& entry : m_pdo_description.slaveOutputs)
    {
        vec.emplace_back(entry.name);
    }
    return vec;
}

const PDOEntry &Slave::getInputPDOEntryRef(const std::string &name) const
{
    for (const auto& entry : m_pdo_description.slaveInputs)
    {
        if (entry.name.compare(name) == 0)
        {
            return entry;
        }
    }
    throw std::logic_error(std::string("No InputPDOEntryRef with name '" + name + "' found!"));
}

const PDOEntry &Slave::getOutputPDOEntryRef(const std::string &name) const
{
    for (const auto& entry : m_pdo_description.slaveOutputs)
    {
        if (entry.name.compare(name) == 0)
        {
            return entry;
        }
    }
    throw std::logic_error(std::string("No OutputPDOEntryRef with name '" + name + "' found!"));
}

std::string Slave::getName() const
{
    return m_name;
}

std::string Slave::currentOutputsToString() const
{
    std::stringstream ss;
    for (unsigned int i = 0; i < 50; i++)
    {
       ss << helper::hex_toString(i,2) << " : " << static_cast<int>(m_outputs->at(i)) << "\n";
    }
    return ss.str();
}

PDOValueT Slave::getValue(const PDOSubEntry *entry) const
{
    ValueArrayT *data = (entry->direction == Input) ? m_inputs : m_outputs;
    switch(entry->datatype)
    {
    case ECT_INTEGER8:
        return static_cast<int8_t>(data->at(entry->totalOffsetInBits / 8));
    case ECT_INTEGER16:
        return static_cast<int16_t>(static_cast<uint16_t>(data->at(entry->totalOffsetInBits / 8)) |
                                    static_cast<uint16_t>(data->at(entry->totalOffsetInBits / 8 + 1)) << 8);
    case ECT_INTEGER24:
        return static_cast<int32_t>(static_cast<uint32_t>(data->at(entry->totalOffsetInBits / 8)) |
                                    static_cast<uint32_t>(data->at(entry->totalOffsetInBits / 8 + 1)) << 8 |
                                    static_cast<uint32_t>(data->at(entry->totalOffsetInBits / 8 + 2)) << 16);
    case ECT_INTEGER32:
        return static_cast<int32_t>(static_cast<uint32_t>(data->at(entry->totalOffsetInBits / 8)) |
                                    static_cast<uint32_t>(data->at(entry->totalOffsetInBits / 8 + 1)) << 8 |
                                    static_cast<uint32_t>(data->at(entry->totalOffsetInBits / 8 + 2)) << 16 |
                                    static_cast<uint32_t>(data->at(entry->totalOffsetInBits / 8 + 3)) << 24);
    case ECT_INTEGER64:
        return static_cast<int64_t>(static_cast<uint64_t>(data->at(entry->totalOffsetInBits / 8)) |
                                    static_cast<uint64_t>(data->at(entry->totalOffsetInBits / 8 + 1)) << 8 |
                                    static_cast<uint64_t>(data->at(entry->totalOffsetInBits / 8 + 2)) << 16 |
                                    static_cast<uint64_t>(data->at(entry->totalOffsetInBits / 8 + 3)) << 24 |
                                    static_cast<uint64_t>(data->at(entry->totalOffsetInBits / 8 + 4)) << 32 |
                                    static_cast<uint64_t>(data->at(entry->totalOffsetInBits / 8 + 5)) << 40 |
                                    static_cast<uint64_t>(data->at(entry->totalOffsetInBits / 8 + 6)) << 48 |
                                    static_cast<uint64_t>(data->at(entry->totalOffsetInBits / 8 + 7)) << 56);
    case ECT_UNSIGNED8:
        return static_cast<uint8_t>(data->at(entry->totalOffsetInBits / 8));
    case ECT_UNSIGNED16:
        return static_cast<uint16_t>(static_cast<uint16_t>(data->at(entry->totalOffsetInBits / 8)) |
                                    static_cast<uint16_t>(data->at(entry->totalOffsetInBits / 8 + 1)) << 8);
    case ECT_UNSIGNED24:
        return static_cast<uint32_t>(static_cast<uint32_t>(data->at(entry->totalOffsetInBits / 8)) |
                                    static_cast<uint32_t>(data->at(entry->totalOffsetInBits / 8 + 1)) << 8 |
                                    static_cast<uint32_t>(data->at(entry->totalOffsetInBits / 8 + 2)) << 16);
    case ECT_UNSIGNED32:
        return static_cast<uint32_t>(static_cast<uint32_t>(data->at(entry->totalOffsetInBits / 8)) |
                                    static_cast<uint32_t>(data->at(entry->totalOffsetInBits / 8 + 1)) << 8 |
                                    static_cast<uint32_t>(data->at(entry->totalOffsetInBits / 8 + 2)) << 16 |
                                    static_cast<uint32_t>(data->at(entry->totalOffsetInBits / 8 + 3)) << 24);
    case ECT_UNSIGNED64:
        return static_cast<uint64_t>(static_cast<uint64_t>(data->at(entry->totalOffsetInBits / 8)) |
                                    static_cast<uint64_t>(data->at(entry->totalOffsetInBits / 8 + 1)) << 8 |
                                    static_cast<uint64_t>(data->at(entry->totalOffsetInBits / 8 + 2)) << 16 |
                                    static_cast<uint64_t>(data->at(entry->totalOffsetInBits / 8 + 3)) << 24 |
                                    static_cast<uint64_t>(data->at(entry->totalOffsetInBits / 8 + 4)) << 32 |
                                    static_cast<uint64_t>(data->at(entry->totalOffsetInBits / 8 + 5)) << 40 |
                                    static_cast<uint64_t>(data->at(entry->totalOffsetInBits / 8 + 6)) << 48 |
                                    static_cast<uint64_t>(data->at(entry->totalOffsetInBits / 8 + 7)) << 56);
    default:
        return "Unknown type";
    }
}

std::string Slave::toString() const
{
    std::stringstream ss;

    auto entries_toString = [&](const std::vector<PDOEntry>& entries) {
        for (const auto& ie : entries)
        {
            ss << ie.name << " - Index: " << helper::hex_toString(ie.index) << '\n';
            for (const auto& sub : ie.entries)
            {
                ss << std::left
                   << '\t'
                   << helper::hex_toString((uint16_t)(sub.totalOffsetInBits / 8)) << ":" << sub.totalOffsetInBits % 8 << '\t'
                   << std::setw(50) << sub.name
                   << helper::ec_datatype_toString(sub.datatype)
                   << '\n';
            }
        }
        return;
    };

    ss << "---------------------------------------------------------------------\n";
    ss << "Name: " << m_name << " - ID: " << m_ID << '\n';
    ss << "Inputs:\n";
    entries_toString(m_pdo_description.slaveInputs);
    ss << "Outputs:\n";
    entries_toString(m_pdo_description.slaveOutputs);
    ss << "---------------------------------------------------------------------\n";

    return ss.str();
}

PDOValueT Slave::getOutputValue(const std::string_view pdoName, unsigned int subIndex) const
{
    return getValue(m_output_name_id_to_sub_entries_map.at(pdoName.data()).at(subIndex));
}

PDOValueT Slave::getValue(std::size_t hash) const
{
    return getValue(m_hash_to_entry_map.at(hash));
}

bool Slave::setInputValue(const std::string_view pdoName, unsigned int subIndex, PDOValueT value)
{
    return setInputValue(m_input_name_id_to_sub_entries_map.at(pdoName.data()).at(subIndex), value);
}

bool Slave::setInputValue(std::size_t hash, PDOValueT value)
{
    return setInputValue(m_hash_to_entry_map.at(hash), value);
}

bool Slave::setInputValue(const PDOSubEntry *entry, PDOValueT value)
{
    switch(entry->datatype)
    {
    case ECT_INTEGER8:
        if (auto v = std::get_if<int8_t>(&value))
        {
            m_inputs->at(entry->totalOffsetInBits / 8) = static_cast<std::byte>(*v);
            return true;
        }
        return false;
    case ECT_INTEGER16:
        if (auto v = std::get_if<int16_t>(&value))
        {
            m_inputs->at(entry->totalOffsetInBits / 8) = static_cast<std::byte>(*v);
            m_inputs->at(entry->totalOffsetInBits / 8 + 1) = static_cast<std::byte>((*v >> 8));
            return true;
        }
        return false;
    case ECT_INTEGER24:
        return static_cast<int32_t>(static_cast<uint32_t>(m_outputs->at(entry->totalOffsetInBits / 8)) |
                                    static_cast<uint32_t>(m_outputs->at(entry->totalOffsetInBits / 8 + 1)) << 8 |
                                    static_cast<uint32_t>(m_outputs->at(entry->totalOffsetInBits / 8 + 2)) << 16);
    case ECT_INTEGER32:
        return static_cast<int32_t>(static_cast<uint32_t>(m_outputs->at(entry->totalOffsetInBits / 8)) |
                                    static_cast<uint32_t>(m_outputs->at(entry->totalOffsetInBits / 8 + 1)) << 8 |
                                    static_cast<uint32_t>(m_outputs->at(entry->totalOffsetInBits / 8 + 2)) << 16 |
                                    static_cast<uint32_t>(m_outputs->at(entry->totalOffsetInBits / 8 + 3)) << 24);
    case ECT_INTEGER64:
        return static_cast<int64_t>(static_cast<uint64_t>(m_outputs->at(entry->totalOffsetInBits / 8)) |
                                    static_cast<uint64_t>(m_outputs->at(entry->totalOffsetInBits / 8 + 1)) << 8 |
                                    static_cast<uint64_t>(m_outputs->at(entry->totalOffsetInBits / 8 + 2)) << 16 |
                                    static_cast<uint64_t>(m_outputs->at(entry->totalOffsetInBits / 8 + 3)) << 24 |
                                    static_cast<uint64_t>(m_outputs->at(entry->totalOffsetInBits / 8 + 4)) << 32 |
                                    static_cast<uint64_t>(m_outputs->at(entry->totalOffsetInBits / 8 + 5)) << 40 |
                                    static_cast<uint64_t>(m_outputs->at(entry->totalOffsetInBits / 8 + 6)) << 48 |
                                    static_cast<uint64_t>(m_outputs->at(entry->totalOffsetInBits / 8 + 7)) << 56);
    case ECT_UNSIGNED8:
        if (auto v = std::get_if<uint8_t>(&value))
        {
            m_inputs->at(entry->totalOffsetInBits / 8) = static_cast<std::byte>(*v);
            return true;
        }
        return false;
    case ECT_UNSIGNED16:
        return static_cast<uint16_t>(static_cast<uint16_t>(m_outputs->at(entry->totalOffsetInBits / 8)) |
                                    static_cast<uint16_t>(m_outputs->at(entry->totalOffsetInBits / 8 + 1)) << 8);
    case ECT_UNSIGNED24:
        return static_cast<uint32_t>(static_cast<uint32_t>(m_outputs->at(entry->totalOffsetInBits / 8)) |
                                    static_cast<uint32_t>(m_outputs->at(entry->totalOffsetInBits / 8 + 1)) << 8 |
                                    static_cast<uint32_t>(m_outputs->at(entry->totalOffsetInBits / 8 + 2)) << 16);
    case ECT_UNSIGNED32:
        return static_cast<uint32_t>(static_cast<uint32_t>(m_outputs->at(entry->totalOffsetInBits / 8)) |
                                    static_cast<uint32_t>(m_outputs->at(entry->totalOffsetInBits / 8 + 1)) << 8 |
                                    static_cast<uint32_t>(m_outputs->at(entry->totalOffsetInBits / 8 + 2)) << 16 |
                                    static_cast<uint32_t>(m_outputs->at(entry->totalOffsetInBits / 8 + 3)) << 24);
    case ECT_UNSIGNED64:
        return static_cast<uint64_t>(static_cast<uint64_t>(m_outputs->at(entry->totalOffsetInBits / 8)) |
                                    static_cast<uint64_t>(m_outputs->at(entry->totalOffsetInBits / 8 + 1)) << 8 |
                                    static_cast<uint64_t>(m_outputs->at(entry->totalOffsetInBits / 8 + 2)) << 16 |
                                    static_cast<uint64_t>(m_outputs->at(entry->totalOffsetInBits / 8 + 3)) << 24 |
                                    static_cast<uint64_t>(m_outputs->at(entry->totalOffsetInBits / 8 + 4)) << 32 |
                                    static_cast<uint64_t>(m_outputs->at(entry->totalOffsetInBits / 8 + 5)) << 40 |
                                    static_cast<uint64_t>(m_outputs->at(entry->totalOffsetInBits / 8 + 6)) << 48 |
                                    static_cast<uint64_t>(m_outputs->at(entry->totalOffsetInBits / 8 + 7)) << 56);
    default:
        return "Unknown type";
    }
}

bool Slave::validatePDODescription(const PDODescription &desc) const
{
    auto checkEntryIndices = [](const std::vector<PDOEntry>& entry) {
        uint16_t lastIndex = 0;
        for (const auto& e : entry)
        {
            uint16_t entryBitLength = 0;
            for (const auto subE : e.entries)
            {
                entryBitLength += subE.bitLength;
            }
            if (lastIndex < e.index + static_cast<uint16_t>(entryBitLength/32))
            {
                lastIndex = e.index;
            }
            else
            {
                return false;
            }
        }
        return true;
    };

    bool valid = true;
    valid &= checkEntryIndices(desc.slaveInputs);
    valid &= checkEntryIndices(desc.slaveOutputs);

    return valid;
}
