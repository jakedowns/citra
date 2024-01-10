if(COMMAND register_leia_module)
    leia_install_sample_dir(${CMAKE_CURRENT_SOURCE_DIR} leia)
endif()

set(LWE_SOURCES
    LWE_Camera.cpp
    LWE_Camera.h
    LWE_Core.cpp
    LWE_Core.h
    LWE_Types.cpp
    LWE_Types.h)

if(NOT COMMAND register_leia_module)
    # Set up a standalone project
    set(LWE_STANDALONE ON)
    set(LNK_BUILD_TESTS ON)

    cmake_minimum_required(VERSION 3.13)
    project(LWE)

    cmake_policy(SET CMP0003 NEW)
    set(CMAKE_CXX_STANDARD 17)

    if(MSVC)
        set(CMAKE_VS_INCLUDE_INSTALL_TO_DEFAULT_BUILD 1)

        STRING(REPLACE "/O2" "/Od" CMAKE_CXX_FLAGS_RELWITHDEBINFO ${CMAKE_CXX_FLAGS_RELWITHDEBINFO})
        SET(CMAKE_CXX_FLAGS_RELWITHDEBINFO ${CMAKE_CXX_FLAGS_RELWITHDEBINFO} CACHE STRING "compile flags" FORCE)

        add_compile_options(/MP)
        add_compile_definitions(UNICODE)
        add_compile_definitions(NOMINMAX)
    endif()

    function(leia_add_executable name)
        cmake_parse_arguments(
            ARGS
            ""
            ""
            "SOURCES;LIBRARIES"
            ${ARGN}
        )

        add_executable(${name} ${ARGS_SOURCES})
        target_link_libraries(${name} PRIVATE ${ARGS_LIBRARIES})

        install(TARGETS ${name})
        if(WIN32)
            install(
                FILES $<TARGET_PDB_FILE:${name}>
                DESTINATION bin
                CONFIGURATIONS "Debug" "RelWithDebInfo")
        endif()
    endfunction()

    function(leia_target_link_libraries target)
        set(accessSpecifiers PRIVATE PUBLIC INTERFACE)
        cmake_parse_arguments(
            ARGS
            ""
            ""
            "${accessSpecifiers}"
            ${ARGN}
        )

        foreach(accessSpecifier ${accessSpecifiers})
            foreach(lib ${ARGS_${accessSpecifier}})
                target_link_libraries(${target} ${accessSpecifier} ${lib})
            endforeach()
        endforeach()
    endfunction()

    set(INSTALL_ASSETDIR bin)
    add_library(LW_Engine STATIC ${LWE_SOURCES})
    target_compile_definitions(LW_Engine PRIVATE _LWE_STANDALONE)
    target_include_directories(LW_Engine PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/../..
        ${CMAKE_CURRENT_SOURCE_DIR}/../../third_party)
    if(WIN32)
        target_link_libraries(LW_Engine PRIVATE
            ${CMAKE_CURRENT_SOURCE_DIR}/../../third_party/VulkanSDK/Lib/x64/vulkan-1.lib)
    endif()
    if(ANDROID)
        target_link_libraries(LW_Engine PRIVATE vulkan)
    endif()

    ################ CNSDK ################
    find_package(CNSDK CONFIG REQUIRED)
    target_link_libraries(LW_Engine PUBLIC CNSDK::leiaSDK)

    if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
        set(prefixPath ${CNSDK_PACKAGE_DIR})
        file(TO_CMAKE_PATH "${prefixPath}" prefixPath)
        set(CMAKE_INSTALL_PREFIX "${prefixPath}" CACHE PATH "Default install prefix" FORCE)
        message(STATUS "Install prefix: ${CMAKE_INSTALL_PREFIX}")
    endif(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)

    # glm and others
    target_include_directories(LW_Engine PUBLIC
        ${CNSDK_PACKAGE_DIR}/samples/third_party)
else()
    register_leia_module(lwe)

    if(NOT LWE_INSTALL_PREFIX)
        set(LWE_INSTALL_PREFIX sdk)
    endif()
    set_local_install_prefix(${LWE_INSTALL_PREFIX})

    leia_add_library(LW_Engine STATIC ${LWE_SOURCES})

    leia_target_link_libraries(LW_Engine PUBLIC
        leiaSDK
    )
    target_link_imgui(LW_Engine PUBLIC)
    target_link_glm(LW_Engine PUBLIC)
    target_link_ovr(LW_Engine PUBLIC)
    target_link_tracy(LW_Engine PUBLIC)
    target_link_vulkan(LW_Engine PUBLIC)
    target_include_directories(LW_Engine PUBLIC
        $<BUILD_INTERFACE:${PUBLIC_HEADERS_BIN_DIR}>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/../..>
    )

    add_shader_files(LW_Engine
        assets/shaders/billboardShader.fs
        assets/shaders/billboardShader.vs
        assets/shaders/modelShader.fs
        assets/shaders/modelShader.vs
        assets/shaders/vulkan_modelShader.fs
        assets/shaders/vulkan_modelShader.vs
        assets/shaders/text.fs
        assets/shaders/text.vs
        assets/shaders/uiShader.fs
        assets/shaders/uiShader.vs)
    group_sources(LW_Engine)
    install(TARGETS LW_Engine)
    leia_add_dependency(LW_Engine assets bin)

    leia_install_sample_dir(${CMAKE_CURRENT_SOURCE_DIR} leia)
endif()

if(WIN32)
    find_package(OpenGL REQUIRED)
    target_link_libraries(LW_Engine PUBLIC OpenGL::GL)
endif()

install(
    DIRECTORY
        assets
    DESTINATION
        ${INSTALL_ASSETDIR})

if(LNK_BUILD_TESTS)
    add_subdirectory(tests)
endif()
