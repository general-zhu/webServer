# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.10

# Delete rule output on recipe failure.
.DELETE_ON_ERROR:


#=============================================================================
# Special targets provided by cmake.

# Disable implicit rules so canonical targets will work.
.SUFFIXES:


# Remove some rules from gmake that .SUFFIXES does not remove.
SUFFIXES =

.SUFFIXES: .hpux_make_needs_suffix_list


# Suppress display of executed commands.
$(VERBOSE).SILENT:


# A target that is always out of date.
cmake_force:

.PHONY : cmake_force

#=============================================================================
# Set environment variables for the build.

# The shell in which to execute make rules.
SHELL = /bin/sh

# The CMake executable.
CMAKE_COMMAND = /usr/bin/cmake

# The command to remove a file.
RM = /usr/bin/cmake -E remove -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /home/c404/data1/zhuchun/wprkerspace/job_project/webServer/version_muduo/muduo

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/c404/data1/zhuchun/wprkerspace/job_project/webServer/version_muduo/build/release-cpp11

# Include any dependencies generated for this target.
include muduo/net/tests/CMakeFiles/buffer_unittest.dir/depend.make

# Include the progress variables for this target.
include muduo/net/tests/CMakeFiles/buffer_unittest.dir/progress.make

# Include the compile flags for this target's objects.
include muduo/net/tests/CMakeFiles/buffer_unittest.dir/flags.make

muduo/net/tests/CMakeFiles/buffer_unittest.dir/Buffer_unittest.cc.o: muduo/net/tests/CMakeFiles/buffer_unittest.dir/flags.make
muduo/net/tests/CMakeFiles/buffer_unittest.dir/Buffer_unittest.cc.o: /home/c404/data1/zhuchun/wprkerspace/job_project/webServer/version_muduo/muduo/muduo/net/tests/Buffer_unittest.cc
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/c404/data1/zhuchun/wprkerspace/job_project/webServer/version_muduo/build/release-cpp11/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object muduo/net/tests/CMakeFiles/buffer_unittest.dir/Buffer_unittest.cc.o"
	cd /home/c404/data1/zhuchun/wprkerspace/job_project/webServer/version_muduo/build/release-cpp11/muduo/net/tests && /usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/buffer_unittest.dir/Buffer_unittest.cc.o -c /home/c404/data1/zhuchun/wprkerspace/job_project/webServer/version_muduo/muduo/muduo/net/tests/Buffer_unittest.cc

muduo/net/tests/CMakeFiles/buffer_unittest.dir/Buffer_unittest.cc.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/buffer_unittest.dir/Buffer_unittest.cc.i"
	cd /home/c404/data1/zhuchun/wprkerspace/job_project/webServer/version_muduo/build/release-cpp11/muduo/net/tests && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/c404/data1/zhuchun/wprkerspace/job_project/webServer/version_muduo/muduo/muduo/net/tests/Buffer_unittest.cc > CMakeFiles/buffer_unittest.dir/Buffer_unittest.cc.i

muduo/net/tests/CMakeFiles/buffer_unittest.dir/Buffer_unittest.cc.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/buffer_unittest.dir/Buffer_unittest.cc.s"
	cd /home/c404/data1/zhuchun/wprkerspace/job_project/webServer/version_muduo/build/release-cpp11/muduo/net/tests && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/c404/data1/zhuchun/wprkerspace/job_project/webServer/version_muduo/muduo/muduo/net/tests/Buffer_unittest.cc -o CMakeFiles/buffer_unittest.dir/Buffer_unittest.cc.s

muduo/net/tests/CMakeFiles/buffer_unittest.dir/Buffer_unittest.cc.o.requires:

.PHONY : muduo/net/tests/CMakeFiles/buffer_unittest.dir/Buffer_unittest.cc.o.requires

muduo/net/tests/CMakeFiles/buffer_unittest.dir/Buffer_unittest.cc.o.provides: muduo/net/tests/CMakeFiles/buffer_unittest.dir/Buffer_unittest.cc.o.requires
	$(MAKE) -f muduo/net/tests/CMakeFiles/buffer_unittest.dir/build.make muduo/net/tests/CMakeFiles/buffer_unittest.dir/Buffer_unittest.cc.o.provides.build
.PHONY : muduo/net/tests/CMakeFiles/buffer_unittest.dir/Buffer_unittest.cc.o.provides

muduo/net/tests/CMakeFiles/buffer_unittest.dir/Buffer_unittest.cc.o.provides.build: muduo/net/tests/CMakeFiles/buffer_unittest.dir/Buffer_unittest.cc.o


# Object files for target buffer_unittest
buffer_unittest_OBJECTS = \
"CMakeFiles/buffer_unittest.dir/Buffer_unittest.cc.o"

# External object files for target buffer_unittest
buffer_unittest_EXTERNAL_OBJECTS =

bin/buffer_unittest: muduo/net/tests/CMakeFiles/buffer_unittest.dir/Buffer_unittest.cc.o
bin/buffer_unittest: muduo/net/tests/CMakeFiles/buffer_unittest.dir/build.make
bin/buffer_unittest: lib/libmuduo_net.a
bin/buffer_unittest: lib/libmuduo_base.a
bin/buffer_unittest: muduo/net/tests/CMakeFiles/buffer_unittest.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/home/c404/data1/zhuchun/wprkerspace/job_project/webServer/version_muduo/build/release-cpp11/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Linking CXX executable ../../../bin/buffer_unittest"
	cd /home/c404/data1/zhuchun/wprkerspace/job_project/webServer/version_muduo/build/release-cpp11/muduo/net/tests && $(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/buffer_unittest.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
muduo/net/tests/CMakeFiles/buffer_unittest.dir/build: bin/buffer_unittest

.PHONY : muduo/net/tests/CMakeFiles/buffer_unittest.dir/build

muduo/net/tests/CMakeFiles/buffer_unittest.dir/requires: muduo/net/tests/CMakeFiles/buffer_unittest.dir/Buffer_unittest.cc.o.requires

.PHONY : muduo/net/tests/CMakeFiles/buffer_unittest.dir/requires

muduo/net/tests/CMakeFiles/buffer_unittest.dir/clean:
	cd /home/c404/data1/zhuchun/wprkerspace/job_project/webServer/version_muduo/build/release-cpp11/muduo/net/tests && $(CMAKE_COMMAND) -P CMakeFiles/buffer_unittest.dir/cmake_clean.cmake
.PHONY : muduo/net/tests/CMakeFiles/buffer_unittest.dir/clean

muduo/net/tests/CMakeFiles/buffer_unittest.dir/depend:
	cd /home/c404/data1/zhuchun/wprkerspace/job_project/webServer/version_muduo/build/release-cpp11 && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/c404/data1/zhuchun/wprkerspace/job_project/webServer/version_muduo/muduo /home/c404/data1/zhuchun/wprkerspace/job_project/webServer/version_muduo/muduo/muduo/net/tests /home/c404/data1/zhuchun/wprkerspace/job_project/webServer/version_muduo/build/release-cpp11 /home/c404/data1/zhuchun/wprkerspace/job_project/webServer/version_muduo/build/release-cpp11/muduo/net/tests /home/c404/data1/zhuchun/wprkerspace/job_project/webServer/version_muduo/build/release-cpp11/muduo/net/tests/CMakeFiles/buffer_unittest.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : muduo/net/tests/CMakeFiles/buffer_unittest.dir/depend

