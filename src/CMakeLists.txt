set(EXE_Name SOEMGui)

find_package(SOEM QUIET)

find_package(Qt5 COMPONENTS Widgets REQUIRED)

add_definitions( ${QT_DEFINITIONS} )

set(HEADERS
    EthercatUnit.h
    EthercatBus.h
    Slave.h
    common.h
    gui/MainWindow.h
    gui/SlaveTreeModel.h
    gui/SOEMGuiController.h
    )

set(SOURCES
    EthercatUnit.cpp
    EthercatBus.cpp
    Slave.cpp
    gui/MainWindow.cpp
    gui/SlaveTreeModel.cpp
    gui/SOEMGuiController.cpp
    )

set(UIS
    gui/MainWindow.ui
    )

# add the executable
add_executable(${EXE_Name} main.cpp ${HEADERS} ${SOURCES} ${UIS})

if (SOEM_FOUND)
    message(STATUS ${SOEM_INCLUDE_DIR})
    message(STATUS ${SOEM_LIBRARIES})

    target_include_directories("${EXE_Name}" SYSTEM PUBLIC ${SOEM_INCLUDE_DIR})
    target_link_libraries("${EXE_Name}" PUBLIC ${SOEM_LIBRARIES} Qt5::Widgets pthread)
endif()
