cmake_minimum_required(VERSION 3.6)
project(AccProject)

set(CMAKE_CXX_STANDARD 11)

add_executable(opti src/main.cpp src/OptiMacroNode.cpp src/Memory.cpp src/CGScheduling.cpp src/MacroNode.cpp
        src/DataBlock.cpp src/MacroNodeTemplate.cpp src/Scheduling.cpp src/Pattern.cpp src/ComputationGraph.cpp
        src/Hardware.cpp src/ProcessConfig.cpp)
