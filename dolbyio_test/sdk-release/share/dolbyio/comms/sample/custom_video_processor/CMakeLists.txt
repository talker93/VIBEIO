cmake_minimum_required(VERSION 3.0...3.21)

add_library(custom_video_processor STATIC
    custom_video_processor.h
	custom_video_processor.cc
)

target_link_libraries(custom_video_processor PRIVATE
	DolbyioComms::media
)

if(APPLE)
	find_library(CORE_VIDEO_LIBRARY CoreVideo)
	target_link_libraries(custom_video_processor PRIVATE
		${CORE_VIDEO_LIBRARY}
	)
endif(APPLE)
