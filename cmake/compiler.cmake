IF(${CMAKE_CXX_COMPILER_ID} STREQUAL "MSVC")
    set_property(GLOBAL PROPERTY USE_FOLDERS ON)

    IF ("${CMAKE_BUILD_TYPE}" STREQUAL "")
        set (CMAKE_BUILD_TYPE "Debug|Release")
    ENDIF()
ELSE()
    # use, i.e. don't skip the full RPATH for the build tree
    SET(CMAKE_SKIP_BUILD_RPATH  FALSE)

    # when building, don't use the install RPATH already
    # (but later on when installing)
    SET(CMAKE_BUILD_WITH_INSTALL_RPATH FALSE) 

    # add the automatically determined parts of the RPATH
    # which point to directories outside the build tree to the install RPATH
    SET(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)

    # the RPATH to be used when installing, but only if it's not a system directory
    LIST(FIND CMAKE_PLATFORM_IMPLICIT_LINK_DIRECTORIES "${CMAKE_INSTALL_RPATH}" isSystemDir)
    IF("${isSystemDir}" STREQUAL "-1")
        SET(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/bin")
    ENDIF("${isSystemDir}" STREQUAL "-1")

    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Wall")
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -s")
    set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -Wall")
    set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -s")
    IF("${CMAKE_BUILD_TYPE}" STREQUAL "")
        set(CMAKE_BUILD_TYPE Debug)
    ENDIF()
ENDIF()

#force load all symbols
#macro(force_load_all tgt_name lib_name)
#    IF(${CMAKE_CXX_COMPILER_ID} STREQUAL "MSVC")
#        target_link_libraries(${tgt_name} ${lib_name})
#    ELSE()
#        target_link_libraries(${tgt_name} -Wl,-whole-archive ${lib_name} -Wl,-no-whole-archive)
#    ENDIF()
#endmacro()

#copied from opencv
# turns off warnings
macro(ocv_warnings_disable)
    if(NOT ENABLE_NOISY_WARNINGS)
        set(_flag_vars "")
        set(_msvc_warnings "")
        set(_gxx_warnings "")
        foreach(arg ${ARGN})
            if(arg MATCHES "^CMAKE_")
                list(APPEND _flag_vars ${arg})
            elseif(arg MATCHES "^/wd")
                list(APPEND _msvc_warnings ${arg})
            elseif(arg MATCHES "^-W")
                list(APPEND _gxx_warnings ${arg})
            endif()
        endforeach()
        if(MSVC AND _msvc_warnings AND _flag_vars)
            foreach(var ${_flag_vars})
                foreach(warning ${_msvc_warnings})
                    set(${var} "${${var}} ${warning}")
                endforeach()
            endforeach()
        elseif((CMAKE_COMPILER_IS_GNUCXX OR (UNIX AND CV_ICC)) AND _gxx_warnings AND _flag_vars)
            foreach(var ${_flag_vars})
                foreach(warning ${_gxx_warnings})
                    if(NOT warning MATCHES "^-Wno-")
                        string(REPLACE "${warning}" "" ${var} "${${var}}")
                        string(REPLACE "-W" "-Wno-" warning "${warning}")
                    endif()
                    ocv_check_flag_support(${var} "${warning}" _varname)
                    if(${_varname})
                        set(${var} "${${var}} ${warning}")
                    endif()
                endforeach()
            endforeach()
        endif()
        unset(_flag_vars)
        unset(_msvc_warnings)
        unset(_gxx_warnings)
    endif(NOT ENABLE_NOISY_WARNINGS)
endmacro()

ocv_warnings_disable(CMAKE_CXX_FLAGS /wd4251) # class 'std::XXX' needs to have dll-interface to be used by clients of YYY
