#include <string>

#include <ethercat.h>

#include "EthercatUnit.h"

using namespace SOEMGui;

int main(int argc, char* argv[])
{
    const std::string ifname = "enx503eaa70e406";

    EthercatUnit unit{ifname};

    return 0;
}

