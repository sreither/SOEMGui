#include <string>
#include <iostream>
#include <thread>

#include <signal.h>
#include <cstring>
#include <unistd.h>
#include <atomic>

#include <ethercat.h>

#include "EthercatUnit.h"

using namespace SOEMGui;

std::atomic<bool> quit(false);
void got_signal(int)
{
    quit.store(true);
}

int main(int argc, char* argv[])
{
    if (argc != 2)
    {
        std::cerr << "Missing argument for adapter name\n" << std::flush;
        return 0;
    }

    struct sigaction sa;
    memset( &sa, 0, sizeof(sa) );
    sa.sa_handler = got_signal;
    sigfillset(&sa.sa_mask);
    sigaction(SIGINT,&sa,NULL);


    const std::string ifname = std::string(argv[1]);

    EthercatUnit unit{ifname};;
    unit.initSlaves();
    while (true)
    {
        using namespace std::chrono_literals;

        if (!unit.run() ) break;
        std::this_thread::sleep_for(1ms);
        if( quit.load() ) break;
    }

    return 0;
}

