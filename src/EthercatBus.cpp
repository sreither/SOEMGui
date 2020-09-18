#include "EthercatBus.h"

#include <ethercat.h>

#include <iostream>

#include <inttypes.h>
#include <cstring>

using namespace SOEMGui;

EthercatBus &EthercatBus::getBus()
{
    static EthercatBus instance;
    return instance;
}

bool EthercatBus::openBus(const std::string ifname)
{
    if (ec_init(ifname.c_str()) > 0)
    {
        std::cerr << "Initialized bus\n";
        return true;
    }

    std::cerr << "Could not init EtherCAT\n";
    return false;
}

void EthercatBus::closeBus()
{
    ec_close();
    m_busReady = false;
    std::cerr << "Closed bus\n";
}

bool EthercatBus::updateBus()
{
    if (m_busReady)
    {
        ec_send_processdata();
        int lastWorkCounter = ec_receive_processdata(EC_TIMEOUTMON * 10);
        if (lastWorkCounter >= m_expectedWKC)
        {
//            for (auto i = 0; i < ec_slave[0].Ibytes; i++)
//            {
//                printf(" %2.2x", *(ec_slave[0].inputs + i));
//            }
//            printf("\n");
//            printf(" T:%"PRId64"\n",ec_DCtime);
        }
    }
    return true;
}

bool EthercatBus::waitUntilAllSlavesReachedOP()
{
    if (ec_config(FALSE, m_IOmap.data()) > 0)
    {
        ec_configdc();
        while(EcatError) printf("%s", ec_elist2string());
        printf("%d slaves found and configured.\n",ec_slavecount);
        /* wait for all slaves to reach SAFE_OP state */
        ec_statecheck(0, EC_STATE_SAFE_OP,  EC_TIMEOUTSTATE * 3);
        if (ec_slave[0].state != EC_STATE_SAFE_OP )
        {
            printf("Not all slaves reached safe operational state.\n");
            ec_readstate();
            for(int i = 1; i<=ec_slavecount ; i++)
            {
                if(ec_slave[i].state != EC_STATE_SAFE_OP)
                {
                    printf("Slave %d State=%2x StatusCode=%4x : %s\n",
                    i, ec_slave[i].state, ec_slave[i].ALstatuscode, ec_ALstatuscode2string(ec_slave[i].ALstatuscode));
                }
            }
        }
        else
        {
            std::cerr << "All slaves reached SAFE-OP\n";

            m_expectedWKC = (ec_group[0].outputsWKC * 2) + ec_group[0].inputsWKC;
            printf("Calculated workcounter %d\n", m_expectedWKC);

            for (unsigned int slave = 1; slave <= ec_slavecount ; slave++)
            {
                ec_slave[slave].state = EC_STATE_OPERATIONAL;
                ec_writestate(slave);
            }
            ec_statecheck(0, EC_STATE_OPERATIONAL,  EC_TIMEOUTSTATE * 3);
            for (unsigned int i = 1; i <= ec_slavecount ; i++)
            {
                if(ec_slave[i].state != EC_STATE_OPERATIONAL)
                {
                    printf("Slave %d State=%2x StatusCode=%4x : %s\n",
                    i, ec_slave[i].state, ec_slave[i].ALstatuscode, ec_ALstatuscode2string(ec_slave[i].ALstatuscode));
                }
                else
                {
                    m_busReady = true;
                    printf("All slaves reached OP\n");
                }
            }
        }
        return true;
    }
    else
    {
         printf("No slaves found!\n");
         closeBus();
         return false;
    }
}

std::vector<Slave> EthercatBus::createSlaves() const
{
    static bool slavesCreated = false;
    if(slavesCreated)
    {
        throw std::logic_error("Slave have already been created! There must be a logic error.");
    }

    std::vector<Slave> slaves;

    if (!m_busReady)
    {
        std::cerr << "Bus not ready!\n";
    }
    else
    {
        for (unsigned int i = 1; i <= ec_slavecount ; i++)
        {
            slaves.emplace_back(i, std::string(ec_slave[i].name), createPDODescription(i));
            slaves.back().setInputs(ec_slave[i].outputs);
            slaves.back().setOutputs(ec_slave[i].inputs);
        }
    }

    slavesCreated = true;
    return slaves;
}

PDODescription EthercatBus::createPDODescription(uint16_t slaveId) const
{
    PDODescription pdoDescription;

    ec_OElistt OElist;
    ec_ODlistt ODlist;
    ODlist.Entries = 0;
    memset(&ODlist, 0, sizeof(ODlist));

    if(ec_readODlist(slaveId, &ODlist))
    {
        for(unsigned int i = 0 ; i < ODlist.Entries ; i++)
        {
            ec_readODdescription(i, &ODlist);
            while(EcatError) printf("%s", ec_elist2string());
            if (ODlist.Index[i] >= 0x6000 && ODlist.Index[i] < 0x8000)
            {
                PDOEntry pdoE;
                pdoE.index = ODlist.Index[i];
                pdoE.name = ODlist.Name[i];

                memset(&OElist, 0, sizeof(OElist));
                ec_readOE(i, &ODlist, &OElist);
                for(uint16_t j = 1 ; j < ODlist.MaxSub[i]+1 ; j++)
                {
                    if ((OElist.DataType[j] > 0) && (OElist.BitLength[j] > 0))
                    {
                        pdoE.entries.emplace_back(PDOSubEntry{OElist.Name[j], j, static_cast<ec_datatype>(OElist.DataType[j]), OElist.BitLength[j]});
                    }
                }

                // --------------------------------------------
                // IMPORTANT: SOEM internally calls the entries that will be sent to the slave "Outputs"
                // while in our slave-implementation we call them "Inputs" because the Slave-object represents the Slave itself
                // --------------------------------------------
                if (ODlist.Index[i] >= 0x6000 && ODlist.Index[i] < 0x7000)
                {
                    pdoDescription.slaveOutputs.push_back(pdoE);
                }
                else
                {
                    pdoDescription.slaveInputs.push_back(pdoE);
                }
            }
            else
            {
                continue;
            }
        }
    }
    else
    {
        std::cerr << "Could not read ODList\n";
    }

    // Calculating offsets
    unsigned int iBitOffset = 0;
    for (auto & entry : pdoDescription.slaveInputs)
    {
        for (auto& subEntry : entry.entries)
        {
            subEntry.totalOffsetInBits = iBitOffset;
            iBitOffset += subEntry.bitLength;
        }
    }

    unsigned int oBitOffset = 0;
    for (auto & entry : pdoDescription.slaveOutputs)
    {
        for (auto& subEntry : entry.entries)
        {
            subEntry.totalOffsetInBits = oBitOffset;
            oBitOffset += subEntry.bitLength;
        }
    }

    return pdoDescription;
}

EthercatBus::EthercatBus()
{

}

