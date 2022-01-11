cmake_minimum_required (VERSION 3.5.0)

option(PSV "build vpk for psvita." OFF)
option(NS "build nro for Nintendo Switch." OFF)

if (PSV)
	set(CMAKE_TOOLCHAIN_FILE "$ENV{VITASDK}/share/vita.toolchain.cmake" CACHE PATH "toolchain file")
	include("$ENV{VITASDK}/share/vita.cmake" REQUIRED)
	include_directories($ENV{VITASDK}/arm-vita-eabi/include/SDL2)
	#set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${CMAKE_C_FLAGS_DEBUG}")
	#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${CMAKE_CXX_FLAGS_DEBUG}")
	set(CMAKE_CXX_STANDARD 17)
elseif(NS)
	include("$ENV{DEVKITPRO}/cmake/Switch.cmake" REQUIRED)
	include_directories($ENV{DEVKITPRO}/portlibs/switch/include/SDL2)
	add_definitions("-D__NS__")
endif()

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17")
set(PROJECT_NAME "sdl2-ui")
set(PROJECT_ELF ${PROJECT_NAME}.elf)
project(${PROJECT_NAME})

set(BASIC_SOURCE "")
file( GLOB
		BASIC_SOURCE
		*.c*
		src/*.c*
		survival/*.c*
		dungeon/*.c*
		common/*.c*
		uilayout/*.c*
		third/lua/*.c*
		third/lutok3/src/lutok3/*.c*
		third/box2d/src/collision/*.c*
		third/box2d/src/common/*.c*
		third/box2d/src/dynamics/*.c*
		third/box2d/src/rope/*.c*
		third/delabella/delabella.cpp
		third/a-star/source/*.c*
		third/pugixml/src/*.c*
)

list(REMOVE_ITEM BASIC_SOURCE
		${CMAKE_CURRENT_SOURCE_DIR}/third/lua/lua.c
		${CMAKE_CURRENT_SOURCE_DIR}/third/lua/ltests.c)

include_directories(
		.
		third/entt/src
		third/cereal/include
		third/eluna/eluna
		third/lua
		third/lutok3/src/lutok3
		third/box2d/include
		third/box2d/src
		third/random/include
		third/delabella
		third/a-star/source
		third/pugixml/src
)

if (PSV)
	add_executable(${PROJECT_ELF} ${BASIC_SOURCE})
	add_definitions(-DLUA_C89_NUMBERS)
	target_link_libraries(${PROJECT_ELF}
			SDL2
			SDL2_ttf
			SDL2_image
			SDL2_mixer
			SDL2_gfx
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

	set(VITA_APP_NAME "Battle City")
	set(VITA_TITLEID  "SDL2BC001") #目录名必须9个字符
	set(VITA_VERSION  "01.00")

	vita_create_self(${PROJECT_NAME}.self ${PROJECT_ELF})
	vita_create_vpk(${PROJECT_NAME}.vpk ${VITA_TITLEID} ${PROJECT_NAME}.self
        	VERSION ${VITA_VERSION}
        	NAME ${VITA_APP_NAME}
        	FILE platform/psv/sce_sys/icon0.png sce_sys/icon0.png
        	FILE platform/psv/sce_sys/livearea/contents/bg.png sce_sys/livearea/contents/bg.png
        	FILE platform/psv/sce_sys/livearea/contents/startup.png sce_sys/livearea/contents/startup.png
        	FILE platform/psv/sce_sys/livearea/contents/template.xml sce_sys/livearea/contents/template.xml
        	${VPKFILES}
    	)
elseif(NINTENDO_SWITCH)
	add_definitions(-DLUA_C89_NUMBERS)
	add_executable(${PROJECT_NAME} ${BASIC_SOURCE})
	target_link_libraries(${PROJECT_NAME} PRIVATE
			SDL2_image
			SDL2_mixer
			SDL2
			SDL2_ttf
			SDL2_gfx
			vorbisidec
			freetype
			flac
			ogg
			png
			jpeg
			webp
			mpg123
			modplug
			opusfile
			opus
			nx
			m
			z
			bz2
			egl
			glapi
			drm_nouveau
	)
	file(COPY assets DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/romfs)
	nx_generate_nacp(${PROJECT_NAME}.nacp
			NAME "Battle City"
			AUTHOR "BAIFENG"
			VERSION "1.0.0"
	)
	nx_create_nro(${PROJECT_NAME}
			NACP ${PROJECT_NAME}.nacp
			ICON ${PROJECT_SOURCE_DIR}/platform/switch/icon.jpg
			ROMFS ${CMAKE_CURRENT_BINARY_DIR}/romfs
	)
elseif (PLATFORM_PSP)
	include_directories($ENV{PSPDEV}/psp/include/SDL2)
	add_definitions(-DLUA_C89_NUMBERS)
	add_executable(${PROJECT_NAME} ${BASIC_SOURCE})
	target_link_libraries(${PROJECT_NAME} PRIVATE
			SDL2_image
			SDL2_mixer
			SDL2main
			SDL2
			SDL2_ttf
			SDL2_gfx
			stdc++
			freetype
			jpeg
			vorbisidec
			mikmod
			ogg
			png
			m
			z
			GL
			pspvram
			${PSP_LIBRARIES}
	)
	create_pbp_file(
			TARGET ${PROJECT_NAME}
			ICON_PATH ICON0.PNG
			BACKGROUND_PATH NULL
			PREVIEW_PATH NULL
			TITLE ${PROJECT_NAME}
	)
	file(COPY platform/psp/ICON0.PNG DESTINATION ${CMAKE_BINARY_DIR})
	file(COPY assets DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
else()
	add_executable(${PROJECT_NAME} ${BASIC_SOURCE})
	file(COPY assets DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
	set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake" ${CMAKE_MODULE_PATH})
	find_package(SDL2 REQUIRED)
	find_package(SDL2_image REQUIRED)
	find_package(SDL2_ttf REQUIRED)
	find_package(SDL2_mixer REQUIRED)
	find_package(SDL2_gfx REQUIRED)
	include_directories(
			${SDL2_INCLUDE_DIRS}
			${SDL2_TTF_INCLUDE_DIRS}
			${SDL2_IMAGE_INCLUDE_DIRS}
			${SDL2GFX_INCLUDE_DIR}
	)
	target_link_libraries(
			${PROJECT_NAME}
			${SDL2_LIBRARY}
			${SDL2_IMAGE_LIBRARIES}
			${SDL2_TTF_LIBRARIES}
			${SDL2_MIXER_LIBRARIES}
			${SDL2GFX_LIBRARY}
	)
endif()
