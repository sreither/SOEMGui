#ifndef ETHERCATUNIT_H
#define ETHERCATUNIT_H

#include "EthercatBus.h"

namespace SOEMGui {

    class EthercatUnit
    {
    public:
        explicit EthercatUnit(const std::string_view adapterName);
        ~EthercatUnit();

        void initSlaves();
        bool run();
        void printCurrentSlaveOutputs() const;
    private:
        void printSlaves() const;

        std::string m_adapterName;

        std::vector<Slave> m_slaves;
    };

}

#endif // ETHERCATUNIT_H
