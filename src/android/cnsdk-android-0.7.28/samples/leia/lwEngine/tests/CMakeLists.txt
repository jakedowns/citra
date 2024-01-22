set(CMAKE_FOLDER ${CMAKE_FOLDER}/tests)

if(WIN32)
    set(CMAKE_WIN32_EXECUTABLE ON)
    leia_add_executable(test_lwe_opengl
        SOURCES
            win/test_lwe_opengl.cpp
        LIBRARIES
            LW_Engine

            odbc32
            odbccp32)
elseif(ANDROID)
    # TODO: add target to build .apk
endif()
