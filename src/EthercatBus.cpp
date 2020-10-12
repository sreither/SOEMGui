#include "EthercatBus.h"
#include "Slave.h"

#include <ethercat.h>

#include <cstring>

#undef EC_MAXNAME
#define EC_MAXNAME 100

using namespace SOEMGui;

EthercatBus &EthercatBus::getBus()
{
    static EthercatBus instance;
    return instance;
}

bool EthercatBus::openBus(const std::string_view ifname)
{
    if (ec_init(ifname.data()) > 0)
    {
        std::cerr << "Initialized bus on adapter " << ifname.data() << "\n";
        return true;
    }

    std::cerr << "Could not init EtherCAT on adpater " << ifname.data() << "\n";
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
            return true;
        }
    }
    return false;
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

std::vector<std::string> EthercatBus::getAdapterNames() const
{
    std::vector<std::string> result;
    ec_adaptert* adapter = ec_find_adapters();
    while (adapter != nullptr)
    {
        result.emplace_back(adapter->name);
        adapter = adapter->next;
    }
    return result;
}

std::string EthercatBus::slaveOutputsToString(int slaveId) const
{
    std::stringstream ss;
    auto iloop = ec_slave[0].Ibytes;
    for(unsigned int j = 0 ; j < iloop; j++)
    {
        ss << helper::hex_toString(j,2) << " : " << static_cast<int>(*(ec_slave[slaveId].outputs + j)) << "\n";
    }
    return ss.str();
}

PDODescription EthercatBus::createPDODescription(uint16_t slaveId) const
{
    PDODescription pdoDescription;

    ec_OElistt OElist;
    ec_ODlistt ODlist;
    ODlist.Entries = 0;
    memset(&ODlist, 0, sizeof(ODlist));

    uint16_t totalOffsetBits_input = 0;
    uint16_t totalOffsetBits_output = 0;

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

                EntryType direction = ODlist.Index[i] < 0x7000 ? Output : Input;
                uint16_t id = 0;

                for(uint16_t j = 0 ; j < ODlist.MaxSub[i]+1 ; j++)
                {
                    bool isSubIndex000Entry = std::string(OElist.Name[j]).rfind("SubIndex 000", 0) == 0;
                    bool isPadding = std::string(OElist.Name[j]).rfind("SubIndex", 0) == 0;
                    if (!isSubIndex000Entry)
                    {
                        if (!isPadding)
                        {
                            pdoE.entries.emplace_back(PDOSubEntry{OElist.Name[j],
                                                                  id,
                                                                  static_cast<ec_datatype>(OElist.DataType[j]),
                                                                  OElist.BitLength[j],
                                                                  direction,
                                                                  direction == Output ? totalOffsetBits_output : totalOffsetBits_input});
                            pdoE.entries.back().hash = PDOSubEntry::PDOSubEntryHash{}(slaveId,
                                                                                      direction,
                                                                                      pdoE.index,
                                                                                      id);
                            id++;
                        }


                        if (direction == Output)
                        {
                            totalOffsetBits_output += OElist.BitLength[j];
                        }
                        else
                        {
                            totalOffsetBits_input += OElist.BitLength[j];
                        }

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

    return pdoDescription;
}

EthercatBus::EthercatBus()
{

}

//std::vector<PDOEntry> EthercatBus::si_siiPDO(uint16 slave, uint8 t, int mapoffset, int bitoffset) const
//{
//    std::vector<PDOEntry> result;

//    uint16 a , w, c, e, er, Size;
//    uint8 eectl;
//    uint16 obj_idx;
//    uint8 obj_subidx;
//    uint8 obj_name;
//    uint8 obj_datatype;
//    uint8 bitlen;
//    int totalsize;
//    ec_eepromPDOt eepPDO;
//    ec_eepromPDOt *PDO;
//    int abs_offset, abs_bit;
//    char str_name[EC_MAXNAME + 1];

//    eectl = ec_slave[slave].eep_pdi;
//    Size = 0;
//    totalsize = 0;
//    PDO = &eepPDO;
//    PDO->nPDO = 0;
//    PDO->Length = 0;
//    PDO->Index[1] = 0;
//    for (c = 0 ; c < EC_MAXSM ; c++) PDO->SMbitsize[c] = 0;
//    if (t > 1)
//        t = 1;
//    PDO->Startpos = ec_siifind(slave, ECT_SII_PDO + t);
//    if (PDO->Startpos > 0)
//    {
//        a = PDO->Startpos;
//        w = ec_siigetbyte(slave, a++);
//        w += (ec_siigetbyte(slave, a++) << 8);
//        PDO->Length = w;
//        c = 1;
//        /* traverse through all PDOs */
//        do
//        {
//            PDO->nPDO++;
//            PDO->Index[PDO->nPDO] = ec_siigetbyte(slave, a++);
//            PDO->Index[PDO->nPDO] += (ec_siigetbyte(slave, a++) << 8);
//            PDO->BitSize[PDO->nPDO] = 0;
//            c++;
//            /* number of entries in PDO */
//            e = ec_siigetbyte(slave, a++);
//            PDO->SyncM[PDO->nPDO] = ec_siigetbyte(slave, a++);
//            a++;
//            obj_name = ec_siigetbyte(slave, a++);
//            a += 2;
//            c += 2;

//            PDOEntry pdoE;
//            pdoE.index = PDO->Index[PDO->nPDO];
//            str_name[0] = 0;
//            if(obj_name)
//               ec_siistring(str_name, slave, obj_name);
//            pdoE.name = std::string(str_name);


//            if (PDO->SyncM[PDO->nPDO] < EC_MAXSM) /* active and in range SM? */
//            {
//                /* read all entries defined in PDO */
//                for (er = 1; er <= e; er++)
//                {
//                    c += 4;
//                    obj_idx = ec_siigetbyte(slave, a++);
//                    obj_idx += (ec_siigetbyte(slave, a++) << 8);
//                    obj_subidx = ec_siigetbyte(slave, a++);
//                    obj_name = ec_siigetbyte(slave, a++);
//                    obj_datatype = ec_siigetbyte(slave, a++);
//                    bitlen = ec_siigetbyte(slave, a++);
//                    abs_offset = mapoffset + (bitoffset / 8);
//                    abs_bit = bitoffset % 8;

//                    PDO->BitSize[PDO->nPDO] += bitlen;
//                    a += 2;

//                    /* skip entry if filler (0x0000:0x00) */
//                    if(obj_idx || obj_subidx)
//                    {
//                       str_name[0] = 0;
//                       if(obj_name)
//                          ec_siistring(str_name, slave, obj_name);

//                       printf("  [0x%4.4X.%1d] 0x%4.4X:0x%2.2X 0x%2.2X", abs_offset, abs_bit, obj_idx, obj_subidx, bitlen);
//                       printf(" %-12s %s\n", helper::ec_datatype_toString((ec_datatype)obj_datatype).data(), str_name);

//                       // We have a valid entry, now we need to determine to which PDO entry it belongs

//                       pdoE.entries.emplace_back(PDOSubEntry{std::string(str_name),
//                                                 static_cast<uint16_t>(obj_subidx),
//                                                 static_cast<ec_datatype>(obj_datatype),
//                                                 static_cast<uint16_t>(bitlen),
//                                                             static_cast<EntryType>(t),
//                                                 static_cast<uint16_t>(abs_offset * 8 + abs_bit)});


//                    }
//                    bitoffset += bitlen;
//                    totalsize += bitlen;
//                }
//                PDO->SMbitsize[ PDO->SyncM[PDO->nPDO] ] += PDO->BitSize[PDO->nPDO];
//                Size += PDO->BitSize[PDO->nPDO];
//                c++;

//                result.push_back(pdoE);
//            }
//            else /* PDO deactivated because SM is 0xff or > EC_MAXSM */
//            {
//                c += 4 * e;
//                a += 8 * e;
//                c++;
//            }
//            if (PDO->nPDO >= (EC_MAXEEPDO - 1)) c = PDO->Length; /* limit number of PDO entries in buffer */
//        }
//        while (c < PDO->Length);
//    }
//    if (eectl) ec_eeprom2pdi(slave); /* if eeprom control was previously pdi then restore */
//    return result;
//}

