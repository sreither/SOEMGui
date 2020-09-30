#ifndef ETHERCATUNIT_H
#define ETHERCATUNIT_H

#include "EthercatBus.h"

namespace SOEMGui {

    class EthercatUnit
    {
    public:
        explicit EthercatUnit(const std::string_view adapterName);
        ~EthercatUnit();

        bool initSlaves();
        bool run();
        void printCurrentSlaveOutputs() const;
        const std::vector<Slave>* getSlaves() const;
        PDOValueT getValue(std::size_t hash) const;
        bool setValue(std::size_t hash, PDOValueT value);
    private:
        void printSlaves() const;

        std::string m_adapterName;

        std::vector<Slave> m_slaves;
        bool m_slaves_initialized {false};
    };

}

#endif // ETHERCATUNIT_H
