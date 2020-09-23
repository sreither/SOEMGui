#include "EthercatUnit.h"

#include <iostream>
#include <sstream>

#ifdef _WIN32
#include <Windows.h>
#else
#include <unistd.h>
#endif

using namespace SOEMGui;

EthercatUnit::EthercatUnit(const std::string_view adapterName)
{
    if (!EthercatBus::getBus().openBus(adapterName))
    {
        std::cerr << "Could not open ethercat bus, try executing as root\n";
    }
}

EthercatUnit::~EthercatUnit()
{
    EthercatBus::getBus().closeBus();
}

void EthercatUnit::initSlaves()
{
    if (EthercatBus::getBus().waitUntilAllSlavesReachedOP())
    {
        m_slaves = EthercatBus::getBus().createSlaves();
        printSlaves();
    }
}

bool EthercatUnit::run()
{
    if (EthercatBus::getBus().updateBus())
    {
        printCurrentSlaveOutputs();
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
                ss << std::left << std::setw(20) << entries.at(i).name << ":\t"
                   << PDOValue_toString(slave.getOutputValue(outName, i)) << "\n";
            }
        }
    }
    std::cerr << ss.str() << "\n";

    firstTime = false;
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
