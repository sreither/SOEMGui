#include "SOEMGuiController.h"
#include "../Slave.h"

#include <QVector>

using namespace SOEMGui;

SOEMGuiController::SOEMGuiController(const std::string& ifname, QObject *parent) : QObject(parent)
{
    qRegisterMetaType<PDOValueT>("PDOValueT");

    m_ethercat_unit = new EthercatUnit();
}

SOEMGuiController::~SOEMGuiController()
{
    m_runThreadRunning = false;
    if (m_runThread.joinable())
    {
        m_runThread.join();
    }
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

QStringList SOEMGuiController::getAdapterNames() const
{
    QStringList list;
    for (const auto& name : m_ethercat_unit->getAvailableAdapterNames())
    {
        list.push_back(QString::fromStdString(name));
    }
    return list;
}

void SOEMGuiController::testSlot() const
{
    m_ethercat_unit->printCurrentSlaveOutputs();
}

void SOEMGuiController::connectToAdapter(const QString ifname)
{
    if (m_runThread.joinable())
    {
        m_runThread.join();
    }

    if (m_ethercat_unit->connectToAdapter(ifname.toStdString()))
    {
        if (m_ethercat_unit->initSlaves())
        {
            m_unit_ready = true;
            emit ethercatUnitConnected();

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
}

void SOEMGuiController::run()
{
//    m_ethercat_unit->printCurrentSlaveOutputs();
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
            std::this_thread::sleep_for(1ms);
        }
        else
        {
            m_runThreadRunning = false;
            emit connectionLost();
        }
    }
}
