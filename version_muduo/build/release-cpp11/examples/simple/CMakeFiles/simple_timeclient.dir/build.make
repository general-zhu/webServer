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
include examples/simple/CMakeFiles/simple_timeclient.dir/depend.make

# Include the progress variables for this target.
include examples/simple/CMakeFiles/simple_timeclient.dir/progress.make

# Include the compile flags for this target's objects.
include examples/simple/CMakeFiles/simple_timeclient.dir/flags.make

examples/simple/CMakeFiles/simple_timeclient.dir/timeclient/timeclient.cc.o: examples/simple/CMakeFiles/simple_timeclient.dir/flags.make
examples/simple/CMakeFiles/simple_timeclient.dir/timeclient/timeclient.cc.o: /home/c404/data1/zhuchun/wprkerspace/job_project/webServer/version_muduo/muduo/examples/simple/timeclient/timeclient.cc
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/c404/data1/zhuchun/wprkerspace/job_project/webServer/version_muduo/build/release-cpp11/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object examples/simple/CMakeFiles/simple_timeclient.dir/timeclient/timeclient.cc.o"
	cd /home/c404/data1/zhuchun/wprkerspace/job_project/webServer/version_muduo/build/release-cpp11/examples/simple && /usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/simple_timeclient.dir/timeclient/timeclient.cc.o -c /home/c404/data1/zhuchun/wprkerspace/job_project/webServer/version_muduo/muduo/examples/simple/timeclient/timeclient.cc

examples/simple/CMakeFiles/simple_timeclient.dir/timeclient/timeclient.cc.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/simple_timeclient.dir/timeclient/timeclient.cc.i"
	cd /home/c404/data1/zhuchun/wprkerspace/job_project/webServer/version_muduo/build/release-cpp11/examples/simple && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/c404/data1/zhuchun/wprkerspace/job_project/webServer/version_muduo/muduo/examples/simple/timeclient/timeclient.cc > CMakeFiles/simple_timeclient.dir/timeclient/timeclient.cc.i

examples/simple/CMakeFiles/simple_timeclient.dir/timeclient/timeclient.cc.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/simple_timeclient.dir/timeclient/timeclient.cc.s"
	cd /home/c404/data1/zhuchun/wprkerspace/job_project/webServer/version_muduo/build/release-cpp11/examples/simple && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/c404/data1/zhuchun/wprkerspace/job_project/webServer/version_muduo/muduo/examples/simple/timeclient/timeclient.cc -o CMakeFiles/simple_timeclient.dir/timeclient/timeclient.cc.s

examples/simple/CMakeFiles/simple_timeclient.dir/timeclient/timeclient.cc.o.requires:

.PHONY : examples/simple/CMakeFiles/simple_timeclient.dir/timeclient/timeclient.cc.o.requires

examples/simple/CMakeFiles/simple_timeclient.dir/timeclient/timeclient.cc.o.provides: examples/simple/CMakeFiles/simple_timeclient.dir/timeclient/timeclient.cc.o.requires
	$(MAKE) -f examples/simple/CMakeFiles/simple_timeclient.dir/build.make examples/simple/CMakeFiles/simple_timeclient.dir/timeclient/timeclient.cc.o.provides.build
.PHONY : examples/simple/CMakeFiles/simple_timeclient.dir/timeclient/timeclient.cc.o.provides

examples/simple/CMakeFiles/simple_timeclient.dir/timeclient/timeclient.cc.o.provides.build: examples/simple/CMakeFiles/simple_timeclient.dir/timeclient/timeclient.cc.o


# Object files for target simple_timeclient
simple_timeclient_OBJECTS = \
"CMakeFiles/simple_timeclient.dir/timeclient/timeclient.cc.o"

# External object files for target simple_timeclient
simple_timeclient_EXTERNAL_OBJECTS =

bin/simple_timeclient: examples/simple/CMakeFiles/simple_timeclient.dir/timeclient/timeclient.cc.o
bin/simple_timeclient: examples/simple/CMakeFiles/simple_timeclient.dir/build.make
bin/simple_timeclient: lib/libmuduo_net.a
bin/simple_timeclient: lib/libmuduo_base.a
bin/simple_timeclient: examples/simple/CMakeFiles/simple_timeclient.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/home/c404/data1/zhuchun/wprkerspace/job_project/webServer/version_muduo/build/release-cpp11/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Linking CXX executable ../../bin/simple_timeclient"
	cd /home/c404/data1/zhuchun/wprkerspace/job_project/webServer/version_muduo/build/release-cpp11/examples/simple && $(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/simple_timeclient.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
examples/simple/CMakeFiles/simple_timeclient.dir/build: bin/simple_timeclient

.PHONY : examples/simple/CMakeFiles/simple_timeclient.dir/build

examples/simple/CMakeFiles/simple_timeclient.dir/requires: examples/simple/CMakeFiles/simple_timeclient.dir/timeclient/timeclient.cc.o.requires

.PHONY : examples/simple/CMakeFiles/simple_timeclient.dir/requires

examples/simple/CMakeFiles/simple_timeclient.dir/clean:
	cd /home/c404/data1/zhuchun/wprkerspace/job_project/webServer/version_muduo/build/release-cpp11/examples/simple && $(CMAKE_COMMAND) -P CMakeFiles/simple_timeclient.dir/cmake_clean.cmake
.PHONY : examples/simple/CMakeFiles/simple_timeclient.dir/clean

examples/simple/CMakeFiles/simple_timeclient.dir/depend:
	cd /home/c404/data1/zhuchun/wprkerspace/job_project/webServer/version_muduo/build/release-cpp11 && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/c404/data1/zhuchun/wprkerspace/job_project/webServer/version_muduo/muduo /home/c404/data1/zhuchun/wprkerspace/job_project/webServer/version_muduo/muduo/examples/simple /home/c404/data1/zhuchun/wprkerspace/job_project/webServer/version_muduo/build/release-cpp11 /home/c404/data1/zhuchun/wprkerspace/job_project/webServer/version_muduo/build/release-cpp11/examples/simple /home/c404/data1/zhuchun/wprkerspace/job_project/webServer/version_muduo/build/release-cpp11/examples/simple/CMakeFiles/simple_timeclient.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : examples/simple/CMakeFiles/simple_timeclient.dir/depend

