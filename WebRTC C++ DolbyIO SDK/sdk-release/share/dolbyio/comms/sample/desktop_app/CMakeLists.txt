cmake_minimum_required(VERSION 3.0...3.21)

add_executable(desktop_app
	desktop_app.cc
)

target_link_libraries(desktop_app
	DolbyioComms::sdk
	utils
	DolbyioComms::media
	DolbyioComms::multimedia_streaming_addon
	media_source_file
)

copy_runtime_deps_dlls(desktop_app)
