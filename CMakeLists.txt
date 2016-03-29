cmake_minimum_required(VERSION 2.8.12)

project(EventBoundaryDetect)

#cmake_policy(SET CMP0054 OLD)
set(CMAKE_VERBOSE_MAKEFILE ON)

set(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR}/bin)
set(LIBRARY_OUTPUT_PATH ${CMAKE_BINARY_DIR}/bin)
set(ARCHIVE_OUTPUT_PATH ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake)

if (NOT PREFIX)
    set(CMAKE_INSTALL_PREFIX ${CMAKE_SOURCE_DIR}/dist)
else()
    set(CMAKE_INSTALL_PREFIX ${PREFIX})
endif()

if (NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Release")
endif (NOT CMAKE_BUILD_TYPE)


SET(CMAKE_DEBUG_POSTFIX  "d")

find_library(Boost Requried)

#comment for fast build in dev
#include(external)
include(compiler)

include_directories(
    ${PROJECT_SOURCE_DIR}/EventBoundDetect
    )

file(GLOB src_list
	"${PROJECT_SOURCE_DIR}/EventBoundDetect/*.h"
	"${PROJECT_SOURCE_DIR}/EventBoundDetect/*.cpp"
	)

add_executable(EventBoundaryDetect ${src_list})
list(APPEND TARGETS EventBoundaryDetect
    )

install(TARGETS ${TARGETS}
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION bin
    ARCHIVE DESTINATION lib
    )

# ========================== build platform ==========================
message(STATUS "")
message(STATUS "  Platform:")
message(STATUS "    Host:"             ${CMAKE_HOST_SYSTEM_NAME} ${CMAKE_HOST_SYSTEM_VERSION} ${CMAKE_HOST_SYSTEM_PROCESSOR})
if(CMAKE_CROSSCOMPILING)
    message(STATUS "    Target:"         ${CMAKE_SYSTEM_NAME} ${CMAKE_SYSTEM_VERSION} ${CMAKE_SYSTEM_PROCESSOR})
endif()
message(STATUS "    CMake:"            ${CMAKE_VERSION})
message(STATUS "    CMake generator:"  ${CMAKE_GENERATOR})
message(STATUS "    CMake build tool:" ${CMAKE_BUILD_TOOL})
if(MSVC)
    message(STATUS "    MSVC:"           ${MSVC_VERSION})
endif()
if(CMAKE_GENERATOR MATCHES Xcode)
    message(STATUS "    Xcode:"          ${XCODE_VERSION})
endif()
if(NOT CMAKE_GENERATOR MATCHES "Xcode")
    message(STATUS "    Configuration:"  ${CMAKE_BUILD_TYPE})
endif()

message(STATUS "")
message(STATUS "  C/C++:")
message(STATUS "    C Compiler:"           ${CMAKE_C_COMPILER})
message(STATUS "    C flags: "    ${CMAKE_C_FLAGS})
message(STATUS "    C flags (Release):"       ${CMAKE_C_FLAGS_RELEASE})
message(STATUS "    C flags (Debug):"         ${CMAKE_C_FLAGS_DEBUG})
message(STATUS "    C++ Compiler:"           ${CMAKE_CXX_COMPILER})
message(STATUS "    C++ flags: "    ${CMAKE_CXX_FLAGS})
message(STATUS "    C++ flags (Release):"    ${CMAKE_CXX_FLAGS_RELEASE})
message(STATUS "    C++ flags (Debug):"       ${CMAKE_CXX_FLAGS_DEBUG})
message(STATUS "    Linker flags:"  ${CMAKE_EXE_LINKER_FLAGS})
if(NOT WIN32)  
    message(STATUS "    Linker flags (Release):"  ${CMAKE_SHARED_LINKER_FLAGS_RELEASE})
    message(STATUS "    Linker flags (Debug):"    ${CMAKE_SHARED_LINKER_FLAGS_DEBUG})
endif()
