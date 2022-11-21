cmake_minimum_required(VERSION 3.20)

project(tests)

enable_language(CXX)

set(CMAKE_ARGS "-DBUILD_GMOCK=OFF")
set(BUILD_GMOCK ON CACHE INTERNAL "DisableGMock")

include(FetchContent)
FetchContent_Declare(
  googletest
  GIT_REPOSITORY https://github.com/google/googletest.git
  GIT_TAG        58d77fa8070e8cec2dc1ed015d66b454c8d78850 # release-1.12.1
)
# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

# Disable GMock
set(BUILD_GMOCK OFF CACHE INTERNAL "")
FetchContent_MakeAvailable(googletest)

set(INCLUDES
../include
../include/intfs
)
include_directories(${INCLUDES})

set(SOURCES
	TestData.cpp
	TestTools.cpp
	TestToolsTests.cpp
	VirtualNodeTests.cpp
	VolumeNodeTests.cpp
	../src/utils/NameRegistrar.cpp
	../src/utils/UniqueIdGenerator.cpp)
	
enable_testing()

add_executable(VirtStorageTests ${SOURCES})
target_compile_features(VirtStorageTests PRIVATE cxx_std_17)

target_link_libraries(
  VirtStorageTests
  GTest::gtest_main
)

include(GoogleTest)

gtest_discover_tests(VirtStorageTests)
