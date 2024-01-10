#----------------------------------------------------------------
# Generated CMake target import file for configuration "Release".
#----------------------------------------------------------------

# Commands may need to know the format version.
set(CMAKE_IMPORT_FILE_VERSION 1)

# Import target "CNSDK::leiaSDK-faceTrackingInService-shared" for configuration "Release"
set_property(TARGET CNSDK::leiaSDK-faceTrackingInService-shared APPEND PROPERTY IMPORTED_CONFIGURATIONS RELEASE)
set_target_properties(CNSDK::leiaSDK-faceTrackingInService-shared PROPERTIES
  IMPORTED_LOCATION_RELEASE "${_IMPORT_PREFIX}/lib/arm64-v8a/libleiaSDK-faceTrackingInService.so"
  IMPORTED_SONAME_RELEASE "libleiaSDK-faceTrackingInService.so"
  )

list(APPEND _cmake_import_check_targets CNSDK::leiaSDK-faceTrackingInService-shared )
list(APPEND _cmake_import_check_files_for_CNSDK::leiaSDK-faceTrackingInService-shared "${_IMPORT_PREFIX}/lib/arm64-v8a/libleiaSDK-faceTrackingInService.so" )

# Commands beyond this point should not need to know the version.
set(CMAKE_IMPORT_FILE_VERSION)
