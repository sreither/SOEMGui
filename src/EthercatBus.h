#ifndef ETHERCATBUS_H
#define ETHERCATBUS_H

#include <string>
#include <cstddef>
#include <vector>

#include "common.h"

namespace SOEMGui {
    class Slave;
    class PDODescription;

    class EthercatBus
    {
    public:
        EthercatBus(EthercatBus const&)     = delete;
        void operator=(EthercatBus const&)  = delete;
        static EthercatBus& getBus();

        bool openBus(const std::string_view ifname);
        void closeBus();
        bool updateBus();

        bool waitUntilAllSlavesReachedOP();
        std::vector<Slave> createSlaves() const;

        std::string slaveOutputsToString(int slaveId) const;

    private:
//        std::vector<PDOEntry> si_siiPDO(uint16 slave, uint8 t, int mapoffset, int bitoffset) const;
        PDODescription createPDODescription(uint16_t slaveId) const;
        EthercatBus();

        /** @brief IOmap the IO map where the process data are mapped in */
        alignas(alignof(std::max_align_t)) std::array<char, IO_MAP_SIZE> m_IOmap;

        int m_expectedWKC;
        bool m_busReady = false;
    };

}


#endif // ETHERCATBUS_H
