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
CMAKE_COMMAND = /home/cristofer/.local/share/JetBrains/Toolbox/apps/CLion/ch-0/181.4668.70/bin/cmake/bin/cmake

# The command to remove a file.
RM = /home/cristofer/.local/share/JetBrains/Toolbox/apps/CLion/ch-0/181.4668.70/bin/cmake/bin/cmake -E remove -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /home/cristofer/Downloads/pc/pc/linkedList_sem

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/cristofer/Downloads/pc/pc/linkedList_sem/cmake-build-debug

# Include any dependencies generated for this target.
include CMakeFiles/linkedList_sem.dir/depend.make

# Include the progress variables for this target.
include CMakeFiles/linkedList_sem.dir/progress.make

# Include the compile flags for this target's objects.
include CMakeFiles/linkedList_sem.dir/flags.make

CMakeFiles/linkedList_sem.dir/LinkedList.c.o: CMakeFiles/linkedList_sem.dir/flags.make
CMakeFiles/linkedList_sem.dir/LinkedList.c.o: ../LinkedList.c
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/cristofer/Downloads/pc/pc/linkedList_sem/cmake-build-debug/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building C object CMakeFiles/linkedList_sem.dir/LinkedList.c.o"
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -o CMakeFiles/linkedList_sem.dir/LinkedList.c.o   -c /home/cristofer/Downloads/pc/pc/linkedList_sem/LinkedList.c

CMakeFiles/linkedList_sem.dir/LinkedList.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/linkedList_sem.dir/LinkedList.c.i"
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E /home/cristofer/Downloads/pc/pc/linkedList_sem/LinkedList.c > CMakeFiles/linkedList_sem.dir/LinkedList.c.i

CMakeFiles/linkedList_sem.dir/LinkedList.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/linkedList_sem.dir/LinkedList.c.s"
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S /home/cristofer/Downloads/pc/pc/linkedList_sem/LinkedList.c -o CMakeFiles/linkedList_sem.dir/LinkedList.c.s

CMakeFiles/linkedList_sem.dir/LinkedList.c.o.requires:

.PHONY : CMakeFiles/linkedList_sem.dir/LinkedList.c.o.requires

CMakeFiles/linkedList_sem.dir/LinkedList.c.o.provides: CMakeFiles/linkedList_sem.dir/LinkedList.c.o.requires
	$(MAKE) -f CMakeFiles/linkedList_sem.dir/build.make CMakeFiles/linkedList_sem.dir/LinkedList.c.o.provides.build
.PHONY : CMakeFiles/linkedList_sem.dir/LinkedList.c.o.provides

CMakeFiles/linkedList_sem.dir/LinkedList.c.o.provides.build: CMakeFiles/linkedList_sem.dir/LinkedList.c.o


# Object files for target linkedList_sem
linkedList_sem_OBJECTS = \
"CMakeFiles/linkedList_sem.dir/LinkedList.c.o"

# External object files for target linkedList_sem
linkedList_sem_EXTERNAL_OBJECTS =

linkedList_sem: CMakeFiles/linkedList_sem.dir/LinkedList.c.o
linkedList_sem: CMakeFiles/linkedList_sem.dir/build.make
linkedList_sem: CMakeFiles/linkedList_sem.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/home/cristofer/Downloads/pc/pc/linkedList_sem/cmake-build-debug/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Linking C executable linkedList_sem"
	$(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/linkedList_sem.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
CMakeFiles/linkedList_sem.dir/build: linkedList_sem

.PHONY : CMakeFiles/linkedList_sem.dir/build

CMakeFiles/linkedList_sem.dir/requires: CMakeFiles/linkedList_sem.dir/LinkedList.c.o.requires

.PHONY : CMakeFiles/linkedList_sem.dir/requires

CMakeFiles/linkedList_sem.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles/linkedList_sem.dir/cmake_clean.cmake
.PHONY : CMakeFiles/linkedList_sem.dir/clean

CMakeFiles/linkedList_sem.dir/depend:
	cd /home/cristofer/Downloads/pc/pc/linkedList_sem/cmake-build-debug && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/cristofer/Downloads/pc/pc/linkedList_sem /home/cristofer/Downloads/pc/pc/linkedList_sem /home/cristofer/Downloads/pc/pc/linkedList_sem/cmake-build-debug /home/cristofer/Downloads/pc/pc/linkedList_sem/cmake-build-debug /home/cristofer/Downloads/pc/pc/linkedList_sem/cmake-build-debug/CMakeFiles/linkedList_sem.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : CMakeFiles/linkedList_sem.dir/depend
