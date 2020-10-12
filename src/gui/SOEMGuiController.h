#ifndef SOEMGUICONTROLLER_H
#define SOEMGUICONTROLLER_H

#include <QObject>
#include <thread>

#include "../EthercatUnit.h"

namespace SOEMGui {
    class SOEMGuiController : public QObject
    {
        Q_OBJECT
    public:
        explicit SOEMGuiController(const std::string& ifname, QObject *parent = nullptr);
        ~SOEMGuiController();

        EthercatUnit* getEthercatUnit() const;
        QStringList getAdapterNames() const;

    signals:
        void dataAvailable(std::size_t pdoSubEntry_hash);
        void ethercatUnitConnected();
        void connectionLost();

    public slots:
        void testSlot() const;
        void connectToAdapter(const QString ifname);

    private:
        void run();

        EthercatUnit* m_ethercat_unit;
        bool m_unit_ready {false};
        bool m_runThreadRunning {false};
        std::thread m_runThread;
    };

}


#endif // SOEMGUICONTROLLER_H
