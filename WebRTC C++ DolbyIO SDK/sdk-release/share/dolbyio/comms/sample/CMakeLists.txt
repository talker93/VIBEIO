cmake_minimum_required(VERSION 3.0...3.21)

project(dolbyio_comms_sample_code)

set(CMAKE_CXX_STANDARD 17)
if(NOT DOLBYIO_COMMS_MODULES_LOCATION)
	set(DOLBYIO_COMMS_MODULES_LOCATION ${CMAKE_CURRENT_LIST_DIR}/../cmake)
endif()

set(CMAKE_THREAD_PREFER_PTHREAD TRUE)
set(THREADS_PREFER_PTHREAD_FLAG TRUE)
find_package(Threads REQUIRED)

if(UNIX AND NOT APPLE AND NOT ANDROID)
	# Non-standard preprocessor definition added by gcc:
	add_definitions(-Ulinux)
endif()

if(WIN32)
	# The SDK uses the non-debug, MT CRT on Windows. The C++ API passes some memory ownership
	# between the application and the SDK, requiring the same allocator to be used in the SDK
	# and in the application. The Windows-specific SDK API allows the application to set the 
	# allocator override, if the application uses some custom allocator, but even if that does 
	# not happen, and that API is unused, the default CMake configuration uses the debug CRT 
	# when building debug mode. Here we enforce always using the release CRT, otherwise debug 
	# builds of the sample code will cause difficult to debug memory corruptions.
	set(CMAKE_MSVC_RUNTIME_LIBRARY MultiThreadedDLL)
endif(WIN32)

# The SDK and the multimedia streaming addon provide the CMake package
# files, which define the imported targets. The recommended way of integrating
# the SDK in a cmake-based project is to use these:
find_package(DolbyioComms REQUIRED
	PATHS ${DOLBYIO_COMMS_MODULES_LOCATION}
)

find_package(DolbyioCommsMultiMediaStreamingAddon
	PATHS ${DOLBYIO_COMMS_MODULES_LOCATION}
)

function(copy_runtime_deps_dlls __target)
	if(WIN32)
		set(DLL_PATH "${CMAKE_SOURCE_DIR}/../../../../bin")
		set(SDK_LOCATION ${DLL_PATH}/dolbyio_comms_sdk.dll)
		set(MEDIA_LOCATION ${DLL_PATH}/dolbyio_comms_media.dll)
		set(MEDIA_ADDON_LOCATION ${DLL_PATH}/dolbyio_comms_multimedia_streaming_addon.dll)
		set(AVCODEC_LOCATION ${DLL_PATH}/avcodec-58.dll)
		set(AVUTIL_LOCATION ${DLL_PATH}/avutil-56.dll)
		set(AVFORMAT_LOCATION ${DLL_PATH}/avformat-58.dll)
		set(DVC_LOCATION ${DLL_PATH}/dvclient.dll)
		set(DNR_LOCATION ${DLL_PATH}/dvdnr.dll)
		set(DNR_MODEL_LOCATION ${DLL_PATH}/model.dnr)

		add_custom_command(TARGET ${__target} POST_BUILD
			COMMAND ${CMAKE_COMMAND} -E copy ${SDK_LOCATION} $<TARGET_FILE_DIR:${__target}>
			COMMAND ${CMAKE_COMMAND} -E copy ${MEDIA_LOCATION} $<TARGET_FILE_DIR:${__target}>
			COMMAND ${CMAKE_COMMAND} -E copy ${MEDIA_ADDON_LOCATION} $<TARGET_FILE_DIR:${__target}>
			COMMAND ${CMAKE_COMMAND} -E copy ${AVCODEC_LOCATION} $<TARGET_FILE_DIR:${__target}>
			COMMAND ${CMAKE_COMMAND} -E copy ${AVUTIL_LOCATION} $<TARGET_FILE_DIR:${__target}>
			COMMAND ${CMAKE_COMMAND} -E copy ${AVFORMAT_LOCATION} $<TARGET_FILE_DIR:${__target}>
			COMMAND ${CMAKE_COMMAND} -E copy ${DVC_LOCATION} $<TARGET_FILE_DIR:${__target}>
			COMMAND ${CMAKE_COMMAND} -E copy ${DNR_LOCATION} $<TARGET_FILE_DIR:${__target}>
			COMMAND ${CMAKE_COMMAND} -E copy ${DNR_MODEL_LOCATION} $<TARGET_FILE_DIR:${__target}>
			COMMAND_EXPAND_LISTS
		)
	endif(WIN32)
endfunction()

if (NOT DolbyioCommsMultiMediaStreamingAddon_FOUND)
	message(FATAL_ERROR "For desktop applications Multimedia Streaming Addon must be present")
endif(NOT DolbyioCommsMultiMediaStreamingAddon_FOUND)

# The sample code uses ffmpeg library. The ffmpeg provided in the package
# is used by the SDK and the multimedia streaming addon, but it's not a part
# of the Dolby products' interfaces. The application does not have to explicitly
# use it. The sample code includes it in a simplest possible way, by hardcoding
# the paths:
set(FFMPEG_ROOT_DIR ${CMAKE_CURRENT_LIST_DIR}/../../../..)
set(FFMPEG_INCLUDE_DIR ${CMAKE_CURRENT_LIST_DIR}/ffmpeg-headers)
add_library(avcodec SHARED IMPORTED)
set_target_properties(avcodec PROPERTIES
IMPORTED_LOCATION ${FFMPEG_ROOT_DIR}/lib/libavcodec${CMAKE_SHARED_LIBRARY_SUFFIX}
INTERFACE_INCLUDE_DIRECTORIES ${FFMPEG_INCLUDE_DIR}
)

add_library(avformat SHARED IMPORTED)
set_target_properties(avformat PROPERTIES
IMPORTED_LOCATION ${FFMPEG_ROOT_DIR}/lib/libavformat${CMAKE_SHARED_LIBRARY_SUFFIX}
INTERFACE_INCLUDE_DIRECTORIES ${FFMPEG_INCLUDE_DIR}
)

add_library(avutil SHARED IMPORTED)
set_target_properties(avutil PROPERTIES
IMPORTED_LOCATION ${FFMPEG_ROOT_DIR}/lib/libavutil${CMAKE_SHARED_LIBRARY_SUFFIX}
INTERFACE_INCLUDE_DIRECTORIES ${FFMPEG_INCLUDE_DIR}
)

if(WIN32)
	set(LIB_PATH "${CMAKE_SOURCE_DIR}/../../../../lib")
	set_target_properties(avcodec PROPERTIES
		IMPORTED_IMPLIB ${LIB_PATH}/${CMAKE_STATIC_LIBRARY_PREFIX}avcodec.lib
	)
	set_target_properties(avformat PROPERTIES
		IMPORTED_IMPLIB ${LIB_PATH}/${CMAKE_STATIC_LIBRARY_PREFIX}avformat.lib
	)
	set_target_properties(avutil PROPERTIES
		IMPORTED_IMPLIB ${LIB_PATH}/${CMAKE_STATIC_LIBRARY_PREFIX}avutil.lib
	)
endif(WIN32)

add_library(ffmpeg INTERFACE)
target_link_libraries(ffmpeg INTERFACE avcodec avformat avutil)
add_subdirectory(custom_injector)
add_subdirectory(custom_recorder)
add_subdirectory(custom_video_processor)
add_subdirectory(media_source)
add_subdirectory(desktop_app)
add_subdirectory(utilities)
