cmake_minimum_required(VERSION 3.10)
project(request_method)

set(CMAKE_CXX_STANDARD 17)
set(VSOMEIP_NAME "vsomeip3")

find_package(${VSOMEIP_NAME})
find_package(Threads REQUIRED)
if (NOT ${VSOMEIP_NAME}_FOUND)
    message("${VSOMEIP_NAME} was not found. Please specify vsomeip_DIR")
endif()

include_directories (
    ${Boost_INCLUDE_DIR}
    ${VSOMEIP_INCLUDE_DIRS}
)

add_executable(service service.cpp)
target_link_libraries(service ${VSOMEIP_NAME} ${Boost_LIBRARIES} Threads::Threads)

add_executable(client client.cpp)
target_link_libraries(client ${VSOMEIP_NAME} ${Boost_LIBRARIES} Threads::Threads)
