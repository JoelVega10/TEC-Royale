cmake_minimum_required(VERSION 3.19)
project(tec-royale)

set(CMAKE_C_STANDARD 11)

find_package(Curses REQUIRED)
include_directories(${CURSES_INCLUDE_DIR})
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -pthread")

add_executable(tec-royale main.c
                GameServer.c
                GameServer.h
                ini.c
                ini.h
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
        )

target_link_libraries(tec-royale m  ${CURSES_LIBRARIES})
