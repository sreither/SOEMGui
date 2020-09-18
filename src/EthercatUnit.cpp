#include "EthercatUnit.h"

#include <iostream>
#include <sstream>

#ifdef _WIN32
#include <Windows.h>
#else
#include <unistd.h>
#endif

using namespace SOEMGui;

EthercatUnit::EthercatUnit(const std::string& adapterName)
{
    EthercatBus m_bus = EthercatBus::getBus();
    if (m_bus.openBus(adapterName))
    {
        if (m_bus.waitUntilAllSlavesReachedOP())
        {
            m_slaves = m_bus.createSlaves();
            printSlaves();

            for (auto i = 0; i < 1000; i++)
            {
                osal_usleep(1000);
                if (m_bus.updateBus())
                {
                    printCurrentSlaveOutputs();
                }
            }
        }
    }
    else {
        std::cerr << "Could not open ethercat bus, try executing as root\n";
    }
}

EthercatUnit::~EthercatUnit()
{
    EthercatBus::getBus().closeBus();
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
            ss << "\033[1A";
        }
    }
    else
    {
        for (const auto& slave : m_slaves)
        {
            linesToClear++;
            for (const auto& outName : slave.getOutputPDONames())
            {
                linesToClear += slave.getOutputPDOEntryRef(outName).entries.size();
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
            for (unsigned int i = 1; i <= entries.size(); i++)
            {
                ss << std::left << std::setw(20) << entries.at(i-1).name << ":\t"
                   << std::to_string(std::get<int32_t>(m_slaves.begin()->getOutputValue(outName, i))) << "\n";
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
