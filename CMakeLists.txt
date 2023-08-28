# CMakeList.txt : CMake project for OrderCache, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.14)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

project (OrderCache LANGUAGES CXX)
include_directories(include)
add_executable (OrderCache
	./include/OrderCacheImpl.h
	./include/OrderCache.h
	./OrderCache.cpp)

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET OrderCache PROPERTY CXX_STANDARD 20)
endif()

if (DEFINED CMAKE_BOOST_ROOT)
	set(BOOST_ROOT ${CMAKE_BOOST_ROOT} CACHE PATH "Boost library path" )
else()
	set(BOOST_ROOT "D:/Code/thirparty/vcpkg/installed/x86-windows" CACHE PATH "Boost library path" )
endif()

message("Searching Boost at Root : ${BOOST_ROOT}")

find_package(Boost)
if(NOT Boost_FOUND)
    message(FATAL_ERROR "Boost Not found")
endif()

if(Boost_FOUND)
	message("Boost Include dir : ${Boost_INCLUDE_DIRS}")
	message("Boost Lib dir : ${Boost_LIBRARIES}")
    include_directories(SYSTEM ${Boost_INCLUDE_DIRS}) 
    target_link_libraries(OrderCache ${Boost_LIBRARIES})
endif()

enable_testing()
add_subdirectory (test)
