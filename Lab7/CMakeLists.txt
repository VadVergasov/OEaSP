cmake_minimum_required(VERSION 3.15)
project(Chat)

set(CMAKE_CXX_STANDARD 20)

set(Boost_USE_STATIC_LIBS ON)
find_package(Boost REQUIRED thread)

message(STATUS "Boost_INCLUDE_DIRS: ${Boost_INCLUDE_DIRS}")
message(STATUS "Boost_LIBRARIES: ${Boost_LIBRARIES}")
message(STATUS "Boost_VERSION: ${Boost_VERSION}")

include_directories(SYSTEM ${Boost_INCLUDE_DIRS})

add_executable(Client Client.cpp ChatMessage.h User.h Room.h Session.h Strings.h)
add_executable(Server Server.cpp ChatMessage.h)

target_link_libraries(Client ${Boost_LIBRARIES})
target_link_libraries(Server ${Boost_LIBRARIES})
