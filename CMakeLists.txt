cmake_minimum_required (VERSION 2.8.3)

option(BUILD_PSV "build vpk for psvita." OFF)

if (BUILD_PSV)
	if(NOT DEFINED CMAKE_TOOLCHAIN_FILE)
  		if(DEFINED ENV{VITASDK})
    		set(CMAKE_TOOLCHAIN_FILE "$ENV{VITASDK}/share/vita.toolchain.cmake" CACHE PATH "toolchain file")
  		else()
    		message(FATAL_ERROR "Please define VITASDK to point to your SDK path!")
  		endif()
	endif()
	include("$ENV{VITASDK}/share/vita.cmake" REQUIRED)
	include_directories($ENV{VITASDK}/arm-vita-eabi/include/SDL2)
else()
	set(CMAKE_CXX_STANDARD 14)
endif()

set(PROJECT_NAME "sdl2-ui")
project(${PROJECT_NAME})

set(BASIC_SOURCE "")
file( GLOB
		BASIC_SOURCE
        *.cpp
		src/*.cpp
		common/*.cpp
)

add_executable(${PROJECT_NAME} ${BASIC_SOURCE})

if (BUILD_PSV)
	target_link_libraries(${PROJECT_NAME}
			SDL2
			SDL2_ttf
			SDL2_image
			SDL2_mixer
			flac
			mikmod
			mpg123
			vorbis
			vorbisfile
			ogg
			vita2d
			debugnet
			freetype
			SceDisplay_stub
			SceCtrl_stub
			SceNetCtl_stub
			SceNet_stub
			SceAudio_stub
			SceSysmodule_stub
			SceGxm_stub
			SceCommonDialog_stub
			SceTouch_stub
			SceHid_stub
			SceMotion_stub
			webp
			png
			jpeg
			m
			z
			-Wl,--whole-archive pthread -Wl,--no-whole-archive # fix std::thread crash after use
			)

    file( GLOB_RECURSE ASSETS RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} assets/*)

	set(VPKFILES "")

	FOREACH(ITEM ${ASSETS})
		list(APPEND VPKFILES "FILE")
		list(APPEND VPKFILES "${ITEM}")
		list(APPEND VPKFILES "${ITEM}")
	ENDFOREACH()

	set(VITA_APP_NAME "SDL2-UI")
	set(VITA_TITLEID  "SDL200001") #目录名必须9个字符
	set(VITA_VERSION  "01.00")

	vita_create_self(${PROJECT_NAME}.self ${PROJECT_NAME})
	vita_create_vpk(${PROJECT_NAME}.vpk ${VITA_TITLEID} ${PROJECT_NAME}.self
        	VERSION ${VITA_VERSION}
        	NAME ${VITA_APP_NAME}
        	FILE psv/sce_sys/icon0.png sce_sys/icon0.png
        	FILE psv/sce_sys/livearea/contents/bg.png sce_sys/livearea/contents/bg.png
        	FILE psv/sce_sys/livearea/contents/startup.png sce_sys/livearea/contents/startup.png
        	FILE psv/sce_sys/livearea/contents/template.xml sce_sys/livearea/contents/template.xml
        	${VPKFILES}
    	)
else()
	file(COPY assets DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
	set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake" ${CMAKE_MODULE_PATH})
	find_package(SDL2 REQUIRED)
	find_package(SDL2_image REQUIRED)
	find_package(SDL2_ttf REQUIRED)
	find_package(SDL2_mixer REQUIRED)
	include_directories(${SDL2_INCLUDE_DIRS})
	include_directories(${SDL2_TTF_INCLUDE_DIRS})
	include_directories(${SDL2_IMAGE_INCLUDE_DIRS})
	target_link_libraries(
		${PROJECT_NAME}
		${SDL2_LIBRARY}
		${SDL2_IMAGE_LIBRARIES}
		${SDL2_TTF_LIBRARIES}
		${SDL2_MIXER_LIBRARIES}
	)
endif()
