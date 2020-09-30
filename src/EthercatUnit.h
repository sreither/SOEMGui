#ifndef ETHERCATUNIT_H
#define ETHERCATUNIT_H

#include <mutex>

#include "EthercatBus.h"
#include "Pdo.h"

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
        bool setInputValue(unsigned int slaveId, const std::string_view pdoName, unsigned int subIndex, PDOValueT value);
        bool setInputValue(std::size_t hash, PDOValueT value);
    private:
        void printSlaves() const;

        std::string m_adapterName;

        std::vector<Slave> m_slaves;
        std::mutex m_update_mutex{};
        bool m_slaves_initialized {false};
    };

}

#endif // ETHERCATUNIT_H
