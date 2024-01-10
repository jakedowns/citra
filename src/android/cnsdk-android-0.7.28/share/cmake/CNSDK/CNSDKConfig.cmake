set(LNK_BUILD_SHARED ON)

include(CMakeFindDependencyMacro)

set(CNSDK_PACKAGE_DIR "${CMAKE_CURRENT_LIST_DIR}/../../..")
if(NOT LNK_BUILD_SHARED)
    list(APPEND CMAKE_PREFIX_PATH ${CNSDK_PACKAGE_DIR})

    add_library(blink INTERFACE)
    find_library(BLINK_LIBRARY blink
        PATH_SUFFIXES
            ${CNSDK_PACKAGE_DIR}/lib/${CMAKE_ANDROID_ARCH_ABI}
            ${CNSDK_PACKAGE_DIR}/lib
        NO_CMAKE_FIND_ROOT_PATH
    )
    if(BLINK_LIBRARY)
        target_link_libraries(blink INTERFACE ${BLINK_LIBRARY})
    endif()
    add_library(blink::blink ALIAS blink)

    if(WIN32 AND NOT TARGET realsense2::realsense2)
        find_library(REALSENSE2_LIBRARY realsense2
            PATH_SUFFIXES
                lib/${CMAKE_ANDROID_ARCH_ABI}
                lib
            NO_CMAKE_FIND_ROOT_PATH
        )
        if(REALSENSE2_LIBRARY)
            add_library(realsense2 INTERFACE)
            add_library(realsense2::realsense2 ALIAS realsense2)
            target_link_libraries(realsense2 INTERFACE ${REALSENSE2_LIBRARY})
        else()
            message(FATAL_ERROR "Failed to find realsense2 library.")
        endif()
    endif()

    if(NOT TARGET cppzmq-static)
        # set(cppzmq_DIR ${CNSDK_PACKAGE_DIR}/share/cmake/cppzmq)
        # find_package(cppzmq CONFIG REQUIRED)
        add_library(cppzmq-static INTERFACE)
    endif()

    if(NOT TARGET flatbuffers::flatbuffers)
        add_library(flatbuffers INTERFACE)
        add_library(flatbuffers::flatbuffers ALIAS flatbuffers)
    endif()

    if(NOT WIN32 AND NOT TARGET spdlog)
        find_library(SPDLOG_LIBRARY
            NAMES
                leiaspdlog spdlog
            PATH_SUFFIXES
                lib/${CMAKE_ANDROID_ARCH_ABI}
                lib
            NO_CMAKE_FIND_ROOT_PATH
        )
        if(SPDLOG_LIBRARY)
            add_library(spdlog INTERFACE)
            add_library(spdlog::spdlog ALIAS spdlog)
            target_link_libraries(spdlog INTERFACE ${SPDLOG_LIBRARY})
        else()
            message(FATAL_ERROR "Failed to find spdlog library.")
        endif()
    endif()

    if(NOT WIN32 AND NOT TARGET simdjson::simdjson)
        find_library(SIMDJSON_LIBRARY simdjson
            PATH_SUFFIXES
                lib/${CMAKE_ANDROID_ARCH_ABI}
                lib
            NO_CMAKE_FIND_ROOT_PATH
        )
        if(SIMDJSON_LIBRARY)
            add_library(simdjson INTERFACE)
            add_library(simdjson::simdjson ALIAS simdjson)
            target_link_libraries(simdjson INTERFACE ${SIMDJSON_LIBRARY})
        else()
            message(FATAL_ERROR "Failed to find simdjson library.")
        endif()
    endif()

    if(NOT TARGET shaderc)
        find_library(SHADERC_LIBRARY
            NAMES
                shaderc shaderc_shared
            PATH_SUFFIXES
                lib/${CMAKE_ANDROID_ARCH_ABI}
                lib
            NO_CMAKE_FIND_ROOT_PATH
        )
        if(SHADERC_LIBRARY)
            add_library(shaderc INTERFACE)
            target_link_libraries(shaderc INTERFACE ${SHADERC_LIBRARY})
        else()
            message(FATAL_ERROR "Failed to find shaderc library.")
        endif()
    endif()

    if(WIN32 AND NOT TARGET leiaDisplaySdk)
        find_library(LEIADISPLAYSDK_LIBRARY LeiaDisplaySdkCpp
            PATH_SUFFIXES
                lib/${CMAKE_ANDROID_ARCH_ABI}
                lib
            NO_CMAKE_FIND_ROOT_PATH
        )
        if(LEIADISPLAYSDK_LIBRARY)
            add_library(leiaDisplaySdk INTERFACE)
            target_link_libraries(leiaDisplaySdk INTERFACE ${LEIADISPLAYSDK_LIBRARY})
        else()
            message(FATAL_ERROR "Failed to find leiaDisplaySdk library.")
        endif()
    endif()

    if(WIN32 AND NOT TARGET vulkan)
        if(TARGET Vulkan::Vulkan)
            add_library(vulkan ALIAS Vulkan::Vulkan)
        else()
            message(FATAL_ERROR "CNSDK requires vulkan target to be provided.")
        endif()
    endif()
endif(NOT LNK_BUILD_SHARED)

# find_dependency(xx 2.0)
include(${CMAKE_CURRENT_LIST_DIR}/CNSDKTargets.cmake)
