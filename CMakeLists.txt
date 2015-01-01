cmake_minimum_required(VERSION 2.8)

set(NAME "Robot-2015")

project(${NAME})

set(CMAKE_CXX_FLAGS "-O3 -Wall -s -std=c++11 -pthread")
set(CMAKE_C_FLAGS "-O3 -Wall -s -std=c11")

set(DIRS
    AutonomousModes/*
    LiveGrapherHost/*
    Logging/*
    MotionProfile/*
    SFMLNetwork/*
    SFMLSystem/*
    Subsystems/*
)

file(GLOB SRC *.cpp *.c)

foreach(ELEM ${DIRS})
    file(GLOB_RECURSE FILES ${ELEM}.cpp ${ELEM}.c)
    list(APPEND SRC ${FILES})
endforeach(ELEM)

add_executable(${NAME} ${SRC})

target_link_libraries(${NAME} wpilib_nonshared HALAthena NetworkTables libFRC_NetworkCommunication.so libNiFpga.so libNiFpgaLv.so libNiRioSrv.so libRoboRIO_FRC_ChipObject.so libspi.so libi2c.so)
