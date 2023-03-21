cmake_minimum_required(VERSION 3.20.0)

find_package(Zephyr)
project(demo_spi)

target_sources(app PRIVATE src/main.cpp 
	src/BlinkingThread.cpp src/BlinkingThread.h
	)