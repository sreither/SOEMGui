#include "EthercatUnit.h"
#include "Slave.h"

#include <iostream>
#include <sstream>

#include <unistd.h>


using namespace SOEMGui;

EthercatUnit::~EthercatUnit()
{
    EthercatBus::getBus().closeBus();
}

bool EthercatUnit::connectToAdapter(const std::string_view adapterName)
{
    return EthercatBus::getBus().openBus(adapterName);
}

bool EthercatUnit::initSlaves()
{
    if (EthercatBus::getBus().waitUntilAllSlavesReachedOP())
    {
        m_slaves = EthercatBus::getBus().createSlaves();
        m_slaves_initialized = true;
//        printSlaves();
        return true;
    }
    else
    {
        return false;
    }
}

bool EthercatUnit::run()
{
    std::scoped_lock lock(m_update_mutex);
    if (EthercatBus::getBus().updateBus())
    {
        return true;
    }
    else
    {
        std::cerr << "Could not update bus.\n";
        return false;
    }
}

void EthercatUnit::printCurrentSlaveOutputs() const
{
#if 0
    static bool firstTime = true;
    static unsigned int linesToClear = 0;

    std::stringstream ss;

    if (!firstTime)
    {
        for (auto j = 0; j < linesToClear; j++)
        {
            ss << "\033[1A" << "\033[K";
        }
    }
    else
    {
        for (const auto& slave : m_slaves)
        {
            linesToClear++;
            for (const auto& outName : slave.getOutputPDONames())
            {
                linesToClear += slave.getOutputPDOEntryRef(outName).entries.size() + 1;
            }
        }
        linesToClear += 1;

        firstTime = false;
    }

    for (const auto& slave : m_slaves)
    {
        ss << "############ " << slave.getName() << " ############\n";
        for (const auto& outName : slave.getOutputPDONames())
        {
            auto entries = slave.getOutputPDOEntryRef(outName).entries;
            ss << "--- " << outName << " ---\n";
            for (unsigned int i = 0; i < entries.size(); i++)
            {
                ss << std::left
                   << "[" << helper::hex_toString(static_cast<uint16_t>(entries.at(i).totalOffsetInBits / 8)) << "]  "
                   << std::setw(50) << entries.at(i).name << ":\t"
                   << PDOValue_toString(slave.getOutputValue(outName, i)) << "\n";
            }
        }
    }
    std::cerr << ss.str() << "\n";
    firstTime = false;
#else
    std::stringstream ss;
    for (const auto& slave : m_slaves)
    {
        ss << "############ " << slave.getName() << " ############\n";
//        ss << slave.currentOutputsToString();
        ss << EthercatBus::getBus().slaveOutputsToString(0);
    }
    std::cerr << ss.str() << "\n";
#endif
}

const std::vector<Slave> *EthercatUnit::getSlaves() const
{
    if (m_slaves_initialized)
    {
        return &m_slaves;
    }
    else
    {
        throw std::logic_error("Slaves have not been initialized!");
    }
}

std::vector<std::string> EthercatUnit::getAvailableAdapterNames() const
{
    return EthercatBus::getBus().getAdapterNames();
}

PDOValueT EthercatUnit::getValue(std::size_t hash) const
{
    for (const Slave& s : m_slaves)
    {
        if (s.hasEntry(hash))
        {
            return s.getValue(hash);
        }
    }
    throw std::logic_error("No slave has entry with hash" + std::to_string(hash));
}

//bool EthercatUnit::setInputValue(unsigned int slaveId, const std::string_view pdoName, unsigned int subIndex, PDOValueT value)
//{
//    return m_slaves.at(slaveId).setInputValue(pdoName, subIndex, value);
//}

bool EthercatUnit::setInputValue(std::size_t hash, PDOValueT value)
{
    std::scoped_lock lock(m_update_mutex);
    for (Slave& s : m_slaves)
    {
        if (s.hasEntry(hash))
        {
            return s.setInputValue(hash, value);
        }
    }
    throw std::logic_error("No slave has entry with hash" + std::to_string(hash));
}

void EthercatUnit::printSlaves() const
{
    std::stringstream ss;
    for (const auto& slave : m_slaves)
    {
        ss << slave.toString();
    }
    std::cerr << ss.str();
}
