# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.15

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
CMAKE_COMMAND = /opt/cmake-3.15.0/bin/cmake

# The command to remove a file.
RM = /opt/cmake-3.15.0/bin/cmake -E remove -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /home/cloud/Desktop/ranger_cpp_0.5.0/source

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/cloud/Desktop/ranger_cpp_0.5.0/source/bulid

# Utility rule file for release.

# Include the progress variables for this target.
include CMakeFiles/release.dir/progress.make

CMakeFiles/release:
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --blue --bold --progress-dir=/home/cloud/Desktop/ranger_cpp_0.5.0/source/bulid/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Switch CMAKE_BUILD_TYPE to Release"
	/opt/cmake-3.15.0/bin/cmake -DCMAKE_BUILD_TYPE=Release /home/cloud/Desktop/ranger_cpp_0.5.0/source
	/opt/cmake-3.15.0/bin/cmake --build /home/cloud/Desktop/ranger_cpp_0.5.0/source/bulid --target all

release: CMakeFiles/release
release: CMakeFiles/release.dir/build.make

.PHONY : release

# Rule to build all files generated by this target.
CMakeFiles/release.dir/build: release

.PHONY : CMakeFiles/release.dir/build

CMakeFiles/release.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles/release.dir/cmake_clean.cmake
.PHONY : CMakeFiles/release.dir/clean

CMakeFiles/release.dir/depend:
	cd /home/cloud/Desktop/ranger_cpp_0.5.0/source/bulid && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/cloud/Desktop/ranger_cpp_0.5.0/source /home/cloud/Desktop/ranger_cpp_0.5.0/source /home/cloud/Desktop/ranger_cpp_0.5.0/source/bulid /home/cloud/Desktop/ranger_cpp_0.5.0/source/bulid /home/cloud/Desktop/ranger_cpp_0.5.0/source/bulid/CMakeFiles/release.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : CMakeFiles/release.dir/depend

