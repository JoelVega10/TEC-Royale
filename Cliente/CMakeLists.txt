cmake_minimum_required(VERSION 3.19)
project(Cliente)

set(CMAKE_C_STANDARD 11)

find_package(Curses REQUIRED)
include_directories(${CURSES_INCLUDE_DIR})
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -pthread -g -Wstack-protector")

add_executable(Cliente
        client.c
        client.h
        ini.c
        ini.h
        algoritmo_genetico.c
        algoritmo_genetico.h
        mypthreads/DataStructures.h
        mypthreads/DataStructures.c
        mypthreads/MyThreadErrors.h
        mypthreads/MyThread.h
        mypthreads/MyThread.c
        mypthreads/MyMutexDataStructure.h
        mypthreads/MyMutexDataStructure.c
        mypthreads/RoundRobinScheduler.h
        mypthreads/RoundRobinScheduler.c
        mypthreads/LotteryScheduler.h
        mypthreads/LotteryScheduler.c
        mypthreads/RealTimeScheduler.h
        mypthreads/RealTimeScheduler.c
        queue.c queue.h)


target_link_libraries(Cliente  m  ${CURSES_LIBRARIES})
