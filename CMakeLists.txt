cmake_minimum_required (VERSION 2.8.1)

######## Project settings ########
PROJECT(ubiimgrdr)
SET(LICENSE "TBD")

######## Build and include settings ########
include_directories(
	inc
)

link_directories(
	${LINK_DIRECTORIES}
)


file(GLOB SOURCES
	"src/*.c"
)

add_executable(
	ubiimgrdr

	${SOURCES}
)

TARGET_LINK_LIBRARIES(
	ubiimgrdr
)

######## Install targets ########
INSTALL(TARGETS ubiimgrdr
	RUNTIME DESTINATION usr/bin
)
