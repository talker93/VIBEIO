cmake_minimum_required(VERSION 3.0...3.21)

get_target_property(DOLBYIO_SDK_HEADERS DolbyioComms::sdk_config INTERFACE_INCLUDE_DIRECTORIES)
if(NOT DOLBYIO_SDK_HEADERS)
    get_target_property(DOLBYIO_SDK_HEADERS DolbyioComms::sdk_config INCLUDE_DIRECTORIES)
endif()

add_library(utils STATIC
    interactor.h
    commands_handler.h
    commands_handler.cc
    sdk/interactions.h
    sdk/interactions.cc
    sdk/events.h
    sdk/events.cc
    sdk/device_manager/interactions.h
    sdk/device_manager/interactions.cc
    media/media_io_interactions.h
    media/media_io_interactions.cc
    ui_loop/ui.h
    ui_loop/ui.cc
    ui_loop/macos_ui.h
)

target_link_libraries(utils PUBLIC
    media_source_file
    DolbyioComms::multimedia_streaming_addon
    DolbyioComms::media
)

if(APPLE)
	find_library(CORE_VIDEO_LIBRARY CoreVideo)
	find_library(CORE_ANIMATION_LIBRARY QuartzCore)
	find_library(COCOA_LIBRARY Cocoa)
	target_sources(utils PRIVATE
		ui_loop/macos_ui.mm
	)
	target_link_libraries(utils PUBLIC
		${CORE_VIDEO_LIBRARY}
		${CORE_ANIMATION_LIBRARY}
		${COCOA_LIBRARY}
	)
endif(APPLE)


# Set the include directory 3 levels up, so headers can be included using
# the full path: "comms/sample/utilities/interactions.h" and so on:
target_include_directories(utils PUBLIC
	"${CMAKE_CURRENT_LIST_DIR}/../../.."
	${DOLBYIO_SDK_HEADERS}
)
