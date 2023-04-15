
project (examples)
cmake_minimum_required(VERSION 3.10)

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)


add_executable(producer_consumer producer_consumer.cpp)
target_link_libraries(producer_consumer PRIVATE umsg Threads::Threads)
set_property(TARGET producer_consumer PROPERTY CXX_STANDARD 17)


add_executable(vanilla_queue vanilla_queue.cpp)
target_link_libraries(vanilla_queue PRIVATE umsg)
set_property(TARGET vanilla_queue PROPERTY CXX_STANDARD 17)

add_executable(tcp_comms tcp_comms.cpp)
target_link_libraries(tcp_comms PRIVATE umsg Threads::Threads)
set_property(TARGET tcp_comms PROPERTY CXX_STANDARD 17)
