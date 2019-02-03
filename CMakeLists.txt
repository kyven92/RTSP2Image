## CMAKE INITIALISATION
PROJECT(vision)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
CMAKE_MINIMUM_REQUIRED(VERSION 2.8)


SET(CMAKE_MODULE_PATH "${CMAKE_MODULE_PATH};${CMAKE_CURRENT_SOURCE_DIR}/cmake/cmake_modules")
## END CMAKE INITIALISATION

## PROJECT LANGUAGE
ENABLE_LANGUAGE(CXX C)
## END PROJECT LANGUAGE

if(NOT Boost_FOUND)
	set(Boost_USE_MULTITHREADED ON)		# default is ON, use -mt variant
	set(Boost_USE_STATIC_LIBS ON)		# default is OFF, use boost static library
#	set(Boost_USE_STATIC_RUNTIME ON)	# default is not set.

	find_package(Boost 1.50.0 COMPONENTS serialization thread system chrono)
	if(NOT Boost_FOUND)
		set(BOOST_ROOT "C:/Program Files/boost/boost_1_50_0")		# if not set, it will let cmake to find it.
		set(BOOST_LIBRARYDIR ${BOOST_ROOT}/stage/lib/x86)			# might need to set this too.
		set(Boost_NO_SYSTEM_PATHS ON)								# default is OFF, set to ON if BOOST_ROOT is set
		find_package(Boost 1.50.0 COMPONENTS serialization thread system chrono REQUIRED)
	endif()
endif()

set(EXECUTABLE_OUTPUT_PATH ${CMAKE_SOURCE_DIR}/bin/)

# Don't warn about zero-length format strings, which we sometimes
# use when constructing error objectss.
# SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D__STDC_CONSTANT_MACROS")
SET (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wno-format-zero-length")
############################################################################################################
## PROJECT INCUDES DIRECTORIES

#LINK_DIRECTORIES(/$ENV{HOME}/ffmpeg_build/lib/)
#LINK_DIRECTORIES(/usr/lib/x86_64-linux-gnu/)

#SET(FFMPEG_INCLUDE_DIR /$ENV{HOME}/ffmpeg_build/include/)

## PROJCET INCLUDES DIRECTORIES
#INCLUDE_DIRECTORIES(${FFMPEG_INCLUDE_DIR})

###########################################################################


if (FFMPEG_LIBRARIES AND FFMPEG_INCLUDE_DIR)
# in cache already
set(FFMPEG_FOUND TRUE)
else (FFMPEG_LIBRARIES AND FFMPEG_INCLUDE_DIR)
# use pkg-config to get the directories and then use these values
# in the FIND_PATH() and FIND_LIBRARY() calls
find_package(PkgConfig)
if (PKG_CONFIG_FOUND)
pkg_check_modules(_FFMPEG_AVCODEC libavcodec)
pkg_check_modules(_FFMPEG_AVFORMAT libavformat)
pkg_check_modules(_FFMPEG_AVUTIL libavutil)
endif (PKG_CONFIG_FOUND)

find_path(FFMPEG_AVCODEC_INCLUDE_DIR
NAMES libavcodec/avcodec.h
PATHS ${_FFMPEG_AVCODEC_INCLUDE_DIRS} /usr/include /usr/local/include /opt/local/include /sw/include
PATH_SUFFIXES ffmpeg libav
)

find_library(FFMPEG_LIBAVCODEC
NAMES avcodec
PATHS ${_FFMPEG_AVCODEC_LIBRARY_DIRS} /usr/lib /usr/local/lib /opt/local/lib /sw/lib
)

find_library(FFMPEG_LIBAVFORMAT
NAMES avformat
PATHS ${_FFMPEG_AVFORMAT_LIBRARY_DIRS} /usr/lib /usr/local/lib /opt/local/lib /sw/lib
)

find_library(FFMPEG_LIBAVUTIL
NAMES avutil
PATHS ${_FFMPEG_AVUTIL_LIBRARY_DIRS} /usr/lib /usr/local/lib /opt/local/lib /sw/lib
)

if (FFMPEG_LIBAVCODEC AND FFMPEG_LIBAVFORMAT)
set(FFMPEG_FOUND TRUE)
endif()

if (FFMPEG_FOUND)
set(FFMPEG_INCLUDE_DIR ${FFMPEG_AVCODEC_INCLUDE_DIR})

set(FFMPEG_LIBRARIES
${FFMPEG_LIBAVCODEC}
${FFMPEG_LIBAVFORMAT}
${FFMPEG_LIBAVUTIL}
)

endif (FFMPEG_FOUND)

if (FFMPEG_FOUND)
if (NOT FFMPEG_FIND_QUIETLY)
message(STATUS "Found FFMPEG or Libav: ${FFMPEG_LIBRARIES}, ${FFMPEG_INCLUDE_DIR}")
endif (NOT FFMPEG_FIND_QUIETLY)
else (FFMPEG_FOUND)
if (FFMPEG_FIND_REQUIRED)
message(FATAL_ERROR "Could not find libavcodec or libavformat or libavutil")
endif (FFMPEG_FIND_REQUIRED)
endif (FFMPEG_FOUND)

endif (FFMPEG_LIBRARIES AND FFMPEG_INCLUDE_DIR)



INCLUDE_DIRECTORIES(include)
SET(SOURCES
 #${CMAKE_CURRENT_LIST_DIR}/src/Save2Video.cpp
  ${CMAKE_CURRENT_LIST_DIR}/src/rtsp2image.cpp

)

#ADD_EXECUTABLE(vision ${SOURCES})

ADD_EXECUTABLE(rtsp2image ${SOURCES})


#TARGET_LINK_LIBRARIES(vision avdevice avfilter swscale avformat avcodec avutil swresample m va va-x11 pthread vorbis vpx opus z postproc dl fdk-aac mp3lame vorbisenc vorbisfile x264 theora x265 theoraenc theoradec dl rt bz2 asound SDL SDLmain SDL_ttf freetype ass lzma ftgl perl crypto xcb xcb-shm xcb-xfixes ao xcb-shape fftw3 X11 vdpau drm va-drm numa log4cpp)
#target_link_libraries(vision ${Boost_LIBRARIES})

TARGET_LINK_LIBRARIES(rtsp2image avdevice avfilter swscale avformat avcodec avutil swresample m va va-x11 pthread vorbis vpx opus z postproc dl fdk-aac mp3lame vorbisenc vorbisfile x264 theora x265 theoraenc theoradec dl rt bz2 asound SDL SDLmain SDL_ttf freetype ass lzma ftgl perl crypto xcb xcb-shm xcb-xfixes ao xcb-shape fftw3 X11 vdpau drm va-drm numa log4cpp)
target_link_libraries(rtsp2image ${Boost_LIBRARIES})

##TARGET_LINK_LIBRARIES(vision avdevice avfilter swscale avformat avcodec avutil swresample m va pthread vorbis vpx opus z postproc dl fdk-aac mp3lame vorbisenc vorbisfile x264 theora x265 theoraenc theoradec dl rt bz2 asound SDL SDLmain SDL_ttf freetype ass lzma ftgl perl crypto xcb xcb-shm xcb-xfixes ao xcb-shape fftw3 boost_system)



message( "*************************************" )
message( "*************************************" )
message( "*************** VISION **************" )
message( "*************************************" )
message( "*************************************" )
