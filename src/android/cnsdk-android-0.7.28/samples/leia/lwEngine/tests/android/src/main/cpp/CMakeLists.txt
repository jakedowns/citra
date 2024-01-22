cmake_minimum_required(VERSION 3.13)

project(LWESample)

if(LWE_STANDALONE)
    set(LWE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../../../../..)
    add_subdirectory(${LWE_DIR} ${CMAKE_CURRENT_BINARY_DIR}/lwe_root)
else()
    set(LNK_SHARED_LIBS leiaSDK)
    set(REPO_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../../../../../../..)
    include(${REPO_DIR}/cmake/commonSetup.cmake)
    add_subdirectory(${REPO_DIR} ${CMAKE_CURRENT_BINARY_DIR}/sdk_root)
endif()

add_library(native-activity SHARED
    main.cpp
    android_native_app_glue.c
    android_native_app_glue.h)
leia_target_link_libraries(native-activity PRIVATE
    LW_Engine

    log
    android
    GLESv1_CM
    EGL)
if(NOT LWE_STANDALONE)
    target_link_imgui(native-activity PRIVATE)
endif()
