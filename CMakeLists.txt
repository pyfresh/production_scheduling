cmake_minimum_required(VERSION 3.30)
project(production_scheduling)

set(CMAKE_CXX_STANDARD 20)

add_executable(production_scheduling main.cpp
        Order.cpp
        Order.h
        Job.cpp
        Job.h
        Machine.cpp
        Machine.h
        Schedule.cpp
        Schedule.h
        DataProc.cpp
        DataProc.h
        GreedySearch.cpp
        GreedySearch.h
        RandomSearch.cpp
        RandomSearch.h)
