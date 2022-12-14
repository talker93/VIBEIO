cmake_minimum_required(VERSION 3.0...3.21)

add_library(custom_injector STATIC
	custom_injector.h
	custom_injector.cc
)

target_link_libraries(custom_injector
  DolbyioComms::sdk
)
