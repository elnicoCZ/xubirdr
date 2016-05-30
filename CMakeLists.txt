cmake_minimum_required (VERSION 2.8.1)

######## Project settings ########
PROJECT(xubirdr)
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
	xubirdr

	${SOURCES}
)

TARGET_LINK_LIBRARIES(
	xubirdr
)

######## Install targets ########
INSTALL(TARGETS xubirdr
	RUNTIME DESTINATION bin
)
