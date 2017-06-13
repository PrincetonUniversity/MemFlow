# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.6

# Default target executed when no arguments are given to make.
default_target: all

.PHONY : default_target

# Allow only one "make -f Makefile2" at a time, but pass parallelism.
.NOTPARALLEL:


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
CMAKE_COMMAND = /usr/local/bin/cmake

# The command to remove a file.
RM = /usr/local/bin/cmake -E remove -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /u/qnie/research/MappingOpti/ALG

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /u/qnie/research/MappingOpti/ALG

#=============================================================================
# Targets provided globally by CMake.

# Special rule for the target rebuild_cache
rebuild_cache:
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --cyan "Running CMake to regenerate build system..."
	/usr/local/bin/cmake -H$(CMAKE_SOURCE_DIR) -B$(CMAKE_BINARY_DIR)
.PHONY : rebuild_cache

# Special rule for the target rebuild_cache
rebuild_cache/fast: rebuild_cache

.PHONY : rebuild_cache/fast

# Special rule for the target edit_cache
edit_cache:
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --cyan "Running CMake cache editor..."
	/usr/local/bin/ccmake -H$(CMAKE_SOURCE_DIR) -B$(CMAKE_BINARY_DIR)
.PHONY : edit_cache

# Special rule for the target edit_cache
edit_cache/fast: edit_cache

.PHONY : edit_cache/fast

# The main all target
all: cmake_check_build_system
	$(CMAKE_COMMAND) -E cmake_progress_start /u/qnie/research/MappingOpti/ALG/CMakeFiles /u/qnie/research/MappingOpti/ALG/CMakeFiles/progress.marks
	$(MAKE) -f CMakeFiles/Makefile2 all
	$(CMAKE_COMMAND) -E cmake_progress_start /u/qnie/research/MappingOpti/ALG/CMakeFiles 0
.PHONY : all

# The main clean target
clean:
	$(MAKE) -f CMakeFiles/Makefile2 clean
.PHONY : clean

# The main clean target
clean/fast: clean

.PHONY : clean/fast

# Prepare targets for installation.
preinstall: all
	$(MAKE) -f CMakeFiles/Makefile2 preinstall
.PHONY : preinstall

# Prepare targets for installation.
preinstall/fast:
	$(MAKE) -f CMakeFiles/Makefile2 preinstall
.PHONY : preinstall/fast

# clear depends
depend:
	$(CMAKE_COMMAND) -H$(CMAKE_SOURCE_DIR) -B$(CMAKE_BINARY_DIR) --check-build-system CMakeFiles/Makefile.cmake 1
.PHONY : depend

#=============================================================================
# Target rules for targets named opti

# Build rule for target.
opti: cmake_check_build_system
	$(MAKE) -f CMakeFiles/Makefile2 opti
.PHONY : opti

# fast build rule for target.
opti/fast:
	$(MAKE) -f CMakeFiles/opti.dir/build.make CMakeFiles/opti.dir/build
.PHONY : opti/fast

src/CGScheduling.o: src/CGScheduling.cpp.o

.PHONY : src/CGScheduling.o

# target to build an object file
src/CGScheduling.cpp.o:
	$(MAKE) -f CMakeFiles/opti.dir/build.make CMakeFiles/opti.dir/src/CGScheduling.cpp.o
.PHONY : src/CGScheduling.cpp.o

src/CGScheduling.i: src/CGScheduling.cpp.i

.PHONY : src/CGScheduling.i

# target to preprocess a source file
src/CGScheduling.cpp.i:
	$(MAKE) -f CMakeFiles/opti.dir/build.make CMakeFiles/opti.dir/src/CGScheduling.cpp.i
.PHONY : src/CGScheduling.cpp.i

src/CGScheduling.s: src/CGScheduling.cpp.s

.PHONY : src/CGScheduling.s

# target to generate assembly for a file
src/CGScheduling.cpp.s:
	$(MAKE) -f CMakeFiles/opti.dir/build.make CMakeFiles/opti.dir/src/CGScheduling.cpp.s
.PHONY : src/CGScheduling.cpp.s

src/ComputationGraph.o: src/ComputationGraph.cpp.o

.PHONY : src/ComputationGraph.o

# target to build an object file
src/ComputationGraph.cpp.o:
	$(MAKE) -f CMakeFiles/opti.dir/build.make CMakeFiles/opti.dir/src/ComputationGraph.cpp.o
.PHONY : src/ComputationGraph.cpp.o

src/ComputationGraph.i: src/ComputationGraph.cpp.i

.PHONY : src/ComputationGraph.i

# target to preprocess a source file
src/ComputationGraph.cpp.i:
	$(MAKE) -f CMakeFiles/opti.dir/build.make CMakeFiles/opti.dir/src/ComputationGraph.cpp.i
.PHONY : src/ComputationGraph.cpp.i

src/ComputationGraph.s: src/ComputationGraph.cpp.s

.PHONY : src/ComputationGraph.s

# target to generate assembly for a file
src/ComputationGraph.cpp.s:
	$(MAKE) -f CMakeFiles/opti.dir/build.make CMakeFiles/opti.dir/src/ComputationGraph.cpp.s
.PHONY : src/ComputationGraph.cpp.s

src/DRAMAccess.o: src/DRAMAccess.cpp.o

.PHONY : src/DRAMAccess.o

# target to build an object file
src/DRAMAccess.cpp.o:
	$(MAKE) -f CMakeFiles/opti.dir/build.make CMakeFiles/opti.dir/src/DRAMAccess.cpp.o
.PHONY : src/DRAMAccess.cpp.o

src/DRAMAccess.i: src/DRAMAccess.cpp.i

.PHONY : src/DRAMAccess.i

# target to preprocess a source file
src/DRAMAccess.cpp.i:
	$(MAKE) -f CMakeFiles/opti.dir/build.make CMakeFiles/opti.dir/src/DRAMAccess.cpp.i
.PHONY : src/DRAMAccess.cpp.i

src/DRAMAccess.s: src/DRAMAccess.cpp.s

.PHONY : src/DRAMAccess.s

# target to generate assembly for a file
src/DRAMAccess.cpp.s:
	$(MAKE) -f CMakeFiles/opti.dir/build.make CMakeFiles/opti.dir/src/DRAMAccess.cpp.s
.PHONY : src/DRAMAccess.cpp.s

src/Data.o: src/Data.cpp.o

.PHONY : src/Data.o

# target to build an object file
src/Data.cpp.o:
	$(MAKE) -f CMakeFiles/opti.dir/build.make CMakeFiles/opti.dir/src/Data.cpp.o
.PHONY : src/Data.cpp.o

src/Data.i: src/Data.cpp.i

.PHONY : src/Data.i

# target to preprocess a source file
src/Data.cpp.i:
	$(MAKE) -f CMakeFiles/opti.dir/build.make CMakeFiles/opti.dir/src/Data.cpp.i
.PHONY : src/Data.cpp.i

src/Data.s: src/Data.cpp.s

.PHONY : src/Data.s

# target to generate assembly for a file
src/Data.cpp.s:
	$(MAKE) -f CMakeFiles/opti.dir/build.make CMakeFiles/opti.dir/src/Data.cpp.s
.PHONY : src/Data.cpp.s

src/DataBlock.o: src/DataBlock.cpp.o

.PHONY : src/DataBlock.o

# target to build an object file
src/DataBlock.cpp.o:
	$(MAKE) -f CMakeFiles/opti.dir/build.make CMakeFiles/opti.dir/src/DataBlock.cpp.o
.PHONY : src/DataBlock.cpp.o

src/DataBlock.i: src/DataBlock.cpp.i

.PHONY : src/DataBlock.i

# target to preprocess a source file
src/DataBlock.cpp.i:
	$(MAKE) -f CMakeFiles/opti.dir/build.make CMakeFiles/opti.dir/src/DataBlock.cpp.i
.PHONY : src/DataBlock.cpp.i

src/DataBlock.s: src/DataBlock.cpp.s

.PHONY : src/DataBlock.s

# target to generate assembly for a file
src/DataBlock.cpp.s:
	$(MAKE) -f CMakeFiles/opti.dir/build.make CMakeFiles/opti.dir/src/DataBlock.cpp.s
.PHONY : src/DataBlock.cpp.s

src/Hardware.o: src/Hardware.cpp.o

.PHONY : src/Hardware.o

# target to build an object file
src/Hardware.cpp.o:
	$(MAKE) -f CMakeFiles/opti.dir/build.make CMakeFiles/opti.dir/src/Hardware.cpp.o
.PHONY : src/Hardware.cpp.o

src/Hardware.i: src/Hardware.cpp.i

.PHONY : src/Hardware.i

# target to preprocess a source file
src/Hardware.cpp.i:
	$(MAKE) -f CMakeFiles/opti.dir/build.make CMakeFiles/opti.dir/src/Hardware.cpp.i
.PHONY : src/Hardware.cpp.i

src/Hardware.s: src/Hardware.cpp.s

.PHONY : src/Hardware.s

# target to generate assembly for a file
src/Hardware.cpp.s:
	$(MAKE) -f CMakeFiles/opti.dir/build.make CMakeFiles/opti.dir/src/Hardware.cpp.s
.PHONY : src/Hardware.cpp.s

src/MacroNode.o: src/MacroNode.cpp.o

.PHONY : src/MacroNode.o

# target to build an object file
src/MacroNode.cpp.o:
	$(MAKE) -f CMakeFiles/opti.dir/build.make CMakeFiles/opti.dir/src/MacroNode.cpp.o
.PHONY : src/MacroNode.cpp.o

src/MacroNode.i: src/MacroNode.cpp.i

.PHONY : src/MacroNode.i

# target to preprocess a source file
src/MacroNode.cpp.i:
	$(MAKE) -f CMakeFiles/opti.dir/build.make CMakeFiles/opti.dir/src/MacroNode.cpp.i
.PHONY : src/MacroNode.cpp.i

src/MacroNode.s: src/MacroNode.cpp.s

.PHONY : src/MacroNode.s

# target to generate assembly for a file
src/MacroNode.cpp.s:
	$(MAKE) -f CMakeFiles/opti.dir/build.make CMakeFiles/opti.dir/src/MacroNode.cpp.s
.PHONY : src/MacroNode.cpp.s

src/MacroNodeTemplate.o: src/MacroNodeTemplate.cpp.o

.PHONY : src/MacroNodeTemplate.o

# target to build an object file
src/MacroNodeTemplate.cpp.o:
	$(MAKE) -f CMakeFiles/opti.dir/build.make CMakeFiles/opti.dir/src/MacroNodeTemplate.cpp.o
.PHONY : src/MacroNodeTemplate.cpp.o

src/MacroNodeTemplate.i: src/MacroNodeTemplate.cpp.i

.PHONY : src/MacroNodeTemplate.i

# target to preprocess a source file
src/MacroNodeTemplate.cpp.i:
	$(MAKE) -f CMakeFiles/opti.dir/build.make CMakeFiles/opti.dir/src/MacroNodeTemplate.cpp.i
.PHONY : src/MacroNodeTemplate.cpp.i

src/MacroNodeTemplate.s: src/MacroNodeTemplate.cpp.s

.PHONY : src/MacroNodeTemplate.s

# target to generate assembly for a file
src/MacroNodeTemplate.cpp.s:
	$(MAKE) -f CMakeFiles/opti.dir/build.make CMakeFiles/opti.dir/src/MacroNodeTemplate.cpp.s
.PHONY : src/MacroNodeTemplate.cpp.s

src/Memory.o: src/Memory.cpp.o

.PHONY : src/Memory.o

# target to build an object file
src/Memory.cpp.o:
	$(MAKE) -f CMakeFiles/opti.dir/build.make CMakeFiles/opti.dir/src/Memory.cpp.o
.PHONY : src/Memory.cpp.o

src/Memory.i: src/Memory.cpp.i

.PHONY : src/Memory.i

# target to preprocess a source file
src/Memory.cpp.i:
	$(MAKE) -f CMakeFiles/opti.dir/build.make CMakeFiles/opti.dir/src/Memory.cpp.i
.PHONY : src/Memory.cpp.i

src/Memory.s: src/Memory.cpp.s

.PHONY : src/Memory.s

# target to generate assembly for a file
src/Memory.cpp.s:
	$(MAKE) -f CMakeFiles/opti.dir/build.make CMakeFiles/opti.dir/src/Memory.cpp.s
.PHONY : src/Memory.cpp.s

src/OptiMacroNode.o: src/OptiMacroNode.cpp.o

.PHONY : src/OptiMacroNode.o

# target to build an object file
src/OptiMacroNode.cpp.o:
	$(MAKE) -f CMakeFiles/opti.dir/build.make CMakeFiles/opti.dir/src/OptiMacroNode.cpp.o
.PHONY : src/OptiMacroNode.cpp.o

src/OptiMacroNode.i: src/OptiMacroNode.cpp.i

.PHONY : src/OptiMacroNode.i

# target to preprocess a source file
src/OptiMacroNode.cpp.i:
	$(MAKE) -f CMakeFiles/opti.dir/build.make CMakeFiles/opti.dir/src/OptiMacroNode.cpp.i
.PHONY : src/OptiMacroNode.cpp.i

src/OptiMacroNode.s: src/OptiMacroNode.cpp.s

.PHONY : src/OptiMacroNode.s

# target to generate assembly for a file
src/OptiMacroNode.cpp.s:
	$(MAKE) -f CMakeFiles/opti.dir/build.make CMakeFiles/opti.dir/src/OptiMacroNode.cpp.s
.PHONY : src/OptiMacroNode.cpp.s

src/Pattern.o: src/Pattern.cpp.o

.PHONY : src/Pattern.o

# target to build an object file
src/Pattern.cpp.o:
	$(MAKE) -f CMakeFiles/opti.dir/build.make CMakeFiles/opti.dir/src/Pattern.cpp.o
.PHONY : src/Pattern.cpp.o

src/Pattern.i: src/Pattern.cpp.i

.PHONY : src/Pattern.i

# target to preprocess a source file
src/Pattern.cpp.i:
	$(MAKE) -f CMakeFiles/opti.dir/build.make CMakeFiles/opti.dir/src/Pattern.cpp.i
.PHONY : src/Pattern.cpp.i

src/Pattern.s: src/Pattern.cpp.s

.PHONY : src/Pattern.s

# target to generate assembly for a file
src/Pattern.cpp.s:
	$(MAKE) -f CMakeFiles/opti.dir/build.make CMakeFiles/opti.dir/src/Pattern.cpp.s
.PHONY : src/Pattern.cpp.s

src/ProcessConfig.o: src/ProcessConfig.cpp.o

.PHONY : src/ProcessConfig.o

# target to build an object file
src/ProcessConfig.cpp.o:
	$(MAKE) -f CMakeFiles/opti.dir/build.make CMakeFiles/opti.dir/src/ProcessConfig.cpp.o
.PHONY : src/ProcessConfig.cpp.o

src/ProcessConfig.i: src/ProcessConfig.cpp.i

.PHONY : src/ProcessConfig.i

# target to preprocess a source file
src/ProcessConfig.cpp.i:
	$(MAKE) -f CMakeFiles/opti.dir/build.make CMakeFiles/opti.dir/src/ProcessConfig.cpp.i
.PHONY : src/ProcessConfig.cpp.i

src/ProcessConfig.s: src/ProcessConfig.cpp.s

.PHONY : src/ProcessConfig.s

# target to generate assembly for a file
src/ProcessConfig.cpp.s:
	$(MAKE) -f CMakeFiles/opti.dir/build.make CMakeFiles/opti.dir/src/ProcessConfig.cpp.s
.PHONY : src/ProcessConfig.cpp.s

src/Scheduling.o: src/Scheduling.cpp.o

.PHONY : src/Scheduling.o

# target to build an object file
src/Scheduling.cpp.o:
	$(MAKE) -f CMakeFiles/opti.dir/build.make CMakeFiles/opti.dir/src/Scheduling.cpp.o
.PHONY : src/Scheduling.cpp.o

src/Scheduling.i: src/Scheduling.cpp.i

.PHONY : src/Scheduling.i

# target to preprocess a source file
src/Scheduling.cpp.i:
	$(MAKE) -f CMakeFiles/opti.dir/build.make CMakeFiles/opti.dir/src/Scheduling.cpp.i
.PHONY : src/Scheduling.cpp.i

src/Scheduling.s: src/Scheduling.cpp.s

.PHONY : src/Scheduling.s

# target to generate assembly for a file
src/Scheduling.cpp.s:
	$(MAKE) -f CMakeFiles/opti.dir/build.make CMakeFiles/opti.dir/src/Scheduling.cpp.s
.PHONY : src/Scheduling.cpp.s

src/Util.o: src/Util.cpp.o

.PHONY : src/Util.o

# target to build an object file
src/Util.cpp.o:
	$(MAKE) -f CMakeFiles/opti.dir/build.make CMakeFiles/opti.dir/src/Util.cpp.o
.PHONY : src/Util.cpp.o

src/Util.i: src/Util.cpp.i

.PHONY : src/Util.i

# target to preprocess a source file
src/Util.cpp.i:
	$(MAKE) -f CMakeFiles/opti.dir/build.make CMakeFiles/opti.dir/src/Util.cpp.i
.PHONY : src/Util.cpp.i

src/Util.s: src/Util.cpp.s

.PHONY : src/Util.s

# target to generate assembly for a file
src/Util.cpp.s:
	$(MAKE) -f CMakeFiles/opti.dir/build.make CMakeFiles/opti.dir/src/Util.cpp.s
.PHONY : src/Util.cpp.s

src/Workload.o: src/Workload.cpp.o

.PHONY : src/Workload.o

# target to build an object file
src/Workload.cpp.o:
	$(MAKE) -f CMakeFiles/opti.dir/build.make CMakeFiles/opti.dir/src/Workload.cpp.o
.PHONY : src/Workload.cpp.o

src/Workload.i: src/Workload.cpp.i

.PHONY : src/Workload.i

# target to preprocess a source file
src/Workload.cpp.i:
	$(MAKE) -f CMakeFiles/opti.dir/build.make CMakeFiles/opti.dir/src/Workload.cpp.i
.PHONY : src/Workload.cpp.i

src/Workload.s: src/Workload.cpp.s

.PHONY : src/Workload.s

# target to generate assembly for a file
src/Workload.cpp.s:
	$(MAKE) -f CMakeFiles/opti.dir/build.make CMakeFiles/opti.dir/src/Workload.cpp.s
.PHONY : src/Workload.cpp.s

src/Workload_LU.o: src/Workload_LU.cpp.o

.PHONY : src/Workload_LU.o

# target to build an object file
src/Workload_LU.cpp.o:
	$(MAKE) -f CMakeFiles/opti.dir/build.make CMakeFiles/opti.dir/src/Workload_LU.cpp.o
.PHONY : src/Workload_LU.cpp.o

src/Workload_LU.i: src/Workload_LU.cpp.i

.PHONY : src/Workload_LU.i

# target to preprocess a source file
src/Workload_LU.cpp.i:
	$(MAKE) -f CMakeFiles/opti.dir/build.make CMakeFiles/opti.dir/src/Workload_LU.cpp.i
.PHONY : src/Workload_LU.cpp.i

src/Workload_LU.s: src/Workload_LU.cpp.s

.PHONY : src/Workload_LU.s

# target to generate assembly for a file
src/Workload_LU.cpp.s:
	$(MAKE) -f CMakeFiles/opti.dir/build.make CMakeFiles/opti.dir/src/Workload_LU.cpp.s
.PHONY : src/Workload_LU.cpp.s

src/Workload_MM.o: src/Workload_MM.cpp.o

.PHONY : src/Workload_MM.o

# target to build an object file
src/Workload_MM.cpp.o:
	$(MAKE) -f CMakeFiles/opti.dir/build.make CMakeFiles/opti.dir/src/Workload_MM.cpp.o
.PHONY : src/Workload_MM.cpp.o

src/Workload_MM.i: src/Workload_MM.cpp.i

.PHONY : src/Workload_MM.i

# target to preprocess a source file
src/Workload_MM.cpp.i:
	$(MAKE) -f CMakeFiles/opti.dir/build.make CMakeFiles/opti.dir/src/Workload_MM.cpp.i
.PHONY : src/Workload_MM.cpp.i

src/Workload_MM.s: src/Workload_MM.cpp.s

.PHONY : src/Workload_MM.s

# target to generate assembly for a file
src/Workload_MM.cpp.s:
	$(MAKE) -f CMakeFiles/opti.dir/build.make CMakeFiles/opti.dir/src/Workload_MM.cpp.s
.PHONY : src/Workload_MM.cpp.s

src/Workload_QR.o: src/Workload_QR.cpp.o

.PHONY : src/Workload_QR.o

# target to build an object file
src/Workload_QR.cpp.o:
	$(MAKE) -f CMakeFiles/opti.dir/build.make CMakeFiles/opti.dir/src/Workload_QR.cpp.o
.PHONY : src/Workload_QR.cpp.o

src/Workload_QR.i: src/Workload_QR.cpp.i

.PHONY : src/Workload_QR.i

# target to preprocess a source file
src/Workload_QR.cpp.i:
	$(MAKE) -f CMakeFiles/opti.dir/build.make CMakeFiles/opti.dir/src/Workload_QR.cpp.i
.PHONY : src/Workload_QR.cpp.i

src/Workload_QR.s: src/Workload_QR.cpp.s

.PHONY : src/Workload_QR.s

# target to generate assembly for a file
src/Workload_QR.cpp.s:
	$(MAKE) -f CMakeFiles/opti.dir/build.make CMakeFiles/opti.dir/src/Workload_QR.cpp.s
.PHONY : src/Workload_QR.cpp.s

src/main.o: src/main.cpp.o

.PHONY : src/main.o

# target to build an object file
src/main.cpp.o:
	$(MAKE) -f CMakeFiles/opti.dir/build.make CMakeFiles/opti.dir/src/main.cpp.o
.PHONY : src/main.cpp.o

src/main.i: src/main.cpp.i

.PHONY : src/main.i

# target to preprocess a source file
src/main.cpp.i:
	$(MAKE) -f CMakeFiles/opti.dir/build.make CMakeFiles/opti.dir/src/main.cpp.i
.PHONY : src/main.cpp.i

src/main.s: src/main.cpp.s

.PHONY : src/main.s

# target to generate assembly for a file
src/main.cpp.s:
	$(MAKE) -f CMakeFiles/opti.dir/build.make CMakeFiles/opti.dir/src/main.cpp.s
.PHONY : src/main.cpp.s

# Help Target
help:
	@echo "The following are some of the valid targets for this Makefile:"
	@echo "... all (the default if no target is provided)"
	@echo "... clean"
	@echo "... depend"
	@echo "... rebuild_cache"
	@echo "... edit_cache"
	@echo "... opti"
	@echo "... src/CGScheduling.o"
	@echo "... src/CGScheduling.i"
	@echo "... src/CGScheduling.s"
	@echo "... src/ComputationGraph.o"
	@echo "... src/ComputationGraph.i"
	@echo "... src/ComputationGraph.s"
	@echo "... src/DRAMAccess.o"
	@echo "... src/DRAMAccess.i"
	@echo "... src/DRAMAccess.s"
	@echo "... src/Data.o"
	@echo "... src/Data.i"
	@echo "... src/Data.s"
	@echo "... src/DataBlock.o"
	@echo "... src/DataBlock.i"
	@echo "... src/DataBlock.s"
	@echo "... src/Hardware.o"
	@echo "... src/Hardware.i"
	@echo "... src/Hardware.s"
	@echo "... src/MacroNode.o"
	@echo "... src/MacroNode.i"
	@echo "... src/MacroNode.s"
	@echo "... src/MacroNodeTemplate.o"
	@echo "... src/MacroNodeTemplate.i"
	@echo "... src/MacroNodeTemplate.s"
	@echo "... src/Memory.o"
	@echo "... src/Memory.i"
	@echo "... src/Memory.s"
	@echo "... src/OptiMacroNode.o"
	@echo "... src/OptiMacroNode.i"
	@echo "... src/OptiMacroNode.s"
	@echo "... src/Pattern.o"
	@echo "... src/Pattern.i"
	@echo "... src/Pattern.s"
	@echo "... src/ProcessConfig.o"
	@echo "... src/ProcessConfig.i"
	@echo "... src/ProcessConfig.s"
	@echo "... src/Scheduling.o"
	@echo "... src/Scheduling.i"
	@echo "... src/Scheduling.s"
	@echo "... src/Util.o"
	@echo "... src/Util.i"
	@echo "... src/Util.s"
	@echo "... src/Workload.o"
	@echo "... src/Workload.i"
	@echo "... src/Workload.s"
	@echo "... src/Workload_LU.o"
	@echo "... src/Workload_LU.i"
	@echo "... src/Workload_LU.s"
	@echo "... src/Workload_MM.o"
	@echo "... src/Workload_MM.i"
	@echo "... src/Workload_MM.s"
	@echo "... src/Workload_QR.o"
	@echo "... src/Workload_QR.i"
	@echo "... src/Workload_QR.s"
	@echo "... src/main.o"
	@echo "... src/main.i"
	@echo "... src/main.s"
.PHONY : help



#=============================================================================
# Special targets to cleanup operation of make.

# Special rule to run CMake to check the build system integrity.
# No rule that depends on this can have commands that come from listfiles
# because they might be regenerated.
cmake_check_build_system:
	$(CMAKE_COMMAND) -H$(CMAKE_SOURCE_DIR) -B$(CMAKE_BINARY_DIR) --check-build-system CMakeFiles/Makefile.cmake 0
.PHONY : cmake_check_build_system

