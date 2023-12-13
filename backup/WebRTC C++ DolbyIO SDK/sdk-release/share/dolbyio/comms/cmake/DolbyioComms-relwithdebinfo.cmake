#----------------------------------------------------------------
# Generated CMake target import file for configuration "RelWithDebInfo".
#----------------------------------------------------------------

# Commands may need to know the format version.
set(CMAKE_IMPORT_FILE_VERSION 1)

# Import target "DolbyioComms::media" for configuration "RelWithDebInfo"
set_property(TARGET DolbyioComms::media APPEND PROPERTY IMPORTED_CONFIGURATIONS RELWITHDEBINFO)
set_target_properties(DolbyioComms::media PROPERTIES
  IMPORTED_LOCATION_RELWITHDEBINFO "${_IMPORT_PREFIX}/lib/libdolbyio_comms_media.dylib"
  IMPORTED_SONAME_RELWITHDEBINFO "@rpath/libdolbyio_comms_media.dylib"
  )

list(APPEND _IMPORT_CHECK_TARGETS DolbyioComms::media )
list(APPEND _IMPORT_CHECK_FILES_FOR_DolbyioComms::media "${_IMPORT_PREFIX}/lib/libdolbyio_comms_media.dylib" )

# Import target "DolbyioComms::sdk" for configuration "RelWithDebInfo"
set_property(TARGET DolbyioComms::sdk APPEND PROPERTY IMPORTED_CONFIGURATIONS RELWITHDEBINFO)
set_target_properties(DolbyioComms::sdk PROPERTIES
  IMPORTED_LOCATION_RELWITHDEBINFO "${_IMPORT_PREFIX}/lib/libdolbyio_comms_sdk.dylib"
  IMPORTED_SONAME_RELWITHDEBINFO "@rpath/libdolbyio_comms_sdk.dylib"
  )

list(APPEND _IMPORT_CHECK_TARGETS DolbyioComms::sdk )
list(APPEND _IMPORT_CHECK_FILES_FOR_DolbyioComms::sdk "${_IMPORT_PREFIX}/lib/libdolbyio_comms_sdk.dylib" )

# Commands beyond this point should not need to know the version.
set(CMAKE_IMPORT_FILE_VERSION)
