cmake_minimum_required(VERSION 3.0...3.21)

add_library(media_source_file STATIC
	file/source_capture.h
	file/source_capture.cc
	file/source_context.h
	file/libav_wrapper/avcontext.h
	file/libav_wrapper/avcontext.cc
	file/libav_wrapper/decoder.h
	file/libav_wrapper/decoder.cc
	file/libav_wrapper/frame.h
	file/libav_wrapper/frame.cc
	file/utils/audio_buffer.h
	file/utils/audio_buffer.cc
	file/utils/media_frame.h
	file/utils/media_frame.cc
	file/utils/frame_pool.h
)

target_link_libraries(media_source_file PRIVATE
	Threads::Threads
	DolbyioComms::multimedia_streaming_addon
)

# Set the include directory 3 levels up, so headers can be included using
# the full path: "comms/sample/media_source/file/source_capture.h" and so on:
target_include_directories(media_source_file PUBLIC
	"${CMAKE_CURRENT_LIST_DIR}/../../.."
	"${CMAKE_CURRENT_LIST_DIR}/../../../.."
)

target_link_libraries(media_source_file PUBLIC
	ffmpeg
)


