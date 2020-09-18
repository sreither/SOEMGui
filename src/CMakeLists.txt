set(EXE_Name SOEMGui)

find_package(SOEM QUIET)

set(HEADERS
    EthercatUnit.h
    EthercatBus.h
    Slave.h
    common.h
    )

set(SOURCES
    EthercatUnit.cpp
    EthercatBus.cpp
    Slave.cpp
    )

# add the executable
add_executable(${EXE_Name} main.cpp ${HEADERS} ${SOURCES})

if (SOEM_FOUND)
    target_include_directories("${EXE_Name}" SYSTEM PUBLIC ${SOEM_INCLUDE_DIR})
    target_link_libraries("${EXE_Name}" PUBLIC ${SOEM_LIBRARIES})
endif()
