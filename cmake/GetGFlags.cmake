find_package(Threads REQUIRED)

include(FetchContent)
FetchContent_Declare(gflags
	GIT_REPOSITORY	https://github.com/gflags/gflags.git
	GIT_TAG	        v2.2.2
)
FetchContent_GetProperties(gflags)
if(NOT gflags_POPULATED)
	FetchContent_Populate(gflags)
	cmake_policy(SET CMP0069 NEW)
	add_subdirectory(${gflags_SOURCE_DIR} ${gflags_BINARY_DIR})
endif()
FetchContent_MakeAvailable(gflags)