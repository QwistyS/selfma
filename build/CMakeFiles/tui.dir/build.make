# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.30

# Delete rule output on recipe failure.
.DELETE_ON_ERROR:

#=============================================================================
# Special targets provided by cmake.

# Disable implicit rules so canonical targets will work.
.SUFFIXES:

# Disable VCS-based implicit rules.
% : %,v

# Disable VCS-based implicit rules.
% : RCS/%

# Disable VCS-based implicit rules.
% : RCS/%,v

# Disable VCS-based implicit rules.
% : SCCS/s.%

# Disable VCS-based implicit rules.
% : s.%

.SUFFIXES: .hpux_make_needs_suffix_list

# Command-line flag to silence nested $(MAKE).
$(VERBOSE)MAKESILENT = -s

#Suppress display of executed commands.
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
RM = /usr/bin/cmake -E rm -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /home/qwistys/src/selfma

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/qwistys/src/selfma/build

# Include any dependencies generated for this target.
include CMakeFiles/tui.dir/depend.make
# Include any dependencies generated by the compiler for this target.
include CMakeFiles/tui.dir/compiler_depend.make

# Include the progress variables for this target.
include CMakeFiles/tui.dir/progress.make

# Include the compile flags for this target's objects.
include CMakeFiles/tui.dir/flags.make

CMakeFiles/tui.dir/core/main.cpp.o: CMakeFiles/tui.dir/flags.make
CMakeFiles/tui.dir/core/main.cpp.o: /home/qwistys/src/selfma/core/main.cpp
CMakeFiles/tui.dir/core/main.cpp.o: CMakeFiles/tui.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green --progress-dir=/home/qwistys/src/selfma/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object CMakeFiles/tui.dir/core/main.cpp.o"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT CMakeFiles/tui.dir/core/main.cpp.o -MF CMakeFiles/tui.dir/core/main.cpp.o.d -o CMakeFiles/tui.dir/core/main.cpp.o -c /home/qwistys/src/selfma/core/main.cpp

CMakeFiles/tui.dir/core/main.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green "Preprocessing CXX source to CMakeFiles/tui.dir/core/main.cpp.i"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/qwistys/src/selfma/core/main.cpp > CMakeFiles/tui.dir/core/main.cpp.i

CMakeFiles/tui.dir/core/main.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green "Compiling CXX source to assembly CMakeFiles/tui.dir/core/main.cpp.s"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/qwistys/src/selfma/core/main.cpp -o CMakeFiles/tui.dir/core/main.cpp.s

# Object files for target tui
tui_OBJECTS = \
"CMakeFiles/tui.dir/core/main.cpp.o"

# External object files for target tui
tui_EXTERNAL_OBJECTS =

tui: CMakeFiles/tui.dir/core/main.cpp.o
tui: CMakeFiles/tui.dir/build.make
tui: CMakeFiles/tui.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green --bold --progress-dir=/home/qwistys/src/selfma/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Linking CXX executable tui"
	$(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/tui.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
CMakeFiles/tui.dir/build: tui
.PHONY : CMakeFiles/tui.dir/build

CMakeFiles/tui.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles/tui.dir/cmake_clean.cmake
.PHONY : CMakeFiles/tui.dir/clean

CMakeFiles/tui.dir/depend:
	cd /home/qwistys/src/selfma/build && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/qwistys/src/selfma /home/qwistys/src/selfma /home/qwistys/src/selfma/build /home/qwistys/src/selfma/build /home/qwistys/src/selfma/build/CMakeFiles/tui.dir/DependInfo.cmake "--color=$(COLOR)"
.PHONY : CMakeFiles/tui.dir/depend

