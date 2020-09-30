#include "SOEMGuiController.h"
#include "../Slave.h"

using namespace SOEMGui;

SOEMGuiController::SOEMGuiController(const std::string& ifname, QObject *parent) : QObject(parent)
{
    qRegisterMetaType<PDOValueT>("PDOValueT");

    m_ethercat_unit = new EthercatUnit(ifname);
    if (m_ethercat_unit->initSlaves())
    {
        m_unit_ready = true;

        m_runThreadRunning = true;
        m_runThread = std::thread
        {
            [this]
            {
                run();
            }
        };
    }
}

SOEMGuiController::~SOEMGuiController()
{
    m_runThreadRunning = false;
    m_runThread.join();
    delete m_ethercat_unit;
}

EthercatUnit *SOEMGuiController::getEthercatUnit() const
{
    if (m_unit_ready)
    {
        return m_ethercat_unit;
    }
    return nullptr;
}

void SOEMGuiController::testSlot() const
{
    m_ethercat_unit->printCurrentSlaveOutputs();
}

void SOEMGuiController::run()
{
    while (m_runThreadRunning)
    {
        if (m_ethercat_unit->run())
        {
            using namespace std::chrono_literals;
            for (const auto &s : *m_ethercat_unit->getSlaves())
            {
                for (const size_t hash : s.getAllPDOSubEntryHashes())
                {
                    emit dataAvailable(hash);
                }
            }

//            m_ethercat_unit->printCurrentSlaveOutputs();
            std::this_thread::sleep_for(1ms);
        }
    }
}
