#ifndef ETHERCATBUS_H
#define ETHERCATBUS_H

#include <string>
#include <cstddef>
#include <array>

#include "ethercat.h"

#include "common.h"
#include "Slave.h"

namespace SOEMGui {
    class EthercatBus
    {
    public:
        static EthercatBus& getBus();

        bool openBus(const std::string_view ifname);
        void closeBus();
        bool updateBus();

        bool waitUntilAllSlavesReachedOP();
        std::vector<Slave> createSlaves() const;

    private:
        PDODescription createPDODescription(uint16_t slaveId) const;
        EthercatBus();

        /** @brief IOmap the IO map where the process data are mapped in */
        alignas(alignof(std::max_align_t)) std::array<char, IO_MAP_SIZE> m_IOmap;

        int m_expectedWKC;
        bool m_busReady = false;
    };

}


#endif // ETHERCATBUS_H
