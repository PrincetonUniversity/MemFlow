CCC = g++

OBJDIR = ../obj
OBJ = $(addprefix $(OBJDIR)/, main.o OptiMacroNode.o Memory.o CGScheduling.o MacroNode.o DataBlock.o MacroNodeTemplate.o Scheduling.o Pattern.o ComputationGraph.o Hardware.o ProcessConfig.o)


opti: $(OBJ)
	$(CCC) -pg -g -Wall -o $@ $(OBJ) -I/usr/include/python2.7 -lpython2.7

$(OBJDIR)/main.o: main.cpp OptiMacroNode.hpp Pattern.hpp ComputationGraph.hpp Hardware.hpp CGScheduling.hpp Scheduling.hpp Util.hpp Setting.hpp ProcessConfig.hpp Memory.hpp
	$(CCC) -pg -g -Wall -std=c++0x -c $< -o $@
$(OBJDIR)/OptiMacroNode.o: OptiMacroNode.cpp OptiMacroNode.hpp
	$(CCC) -pg -g -Wall -std=c++0x -c $< -o $@
$(OBJDIR)/Memory.o: Memory.cpp Memory.hpp Setting.hpp
	$(CCC) -pg -g -Wall -std=c++0x -c $< -o $@
$(OBJDIR)/CGScheduling.o: CGScheduling.cpp CGScheduling.hpp MacroNode.hpp Pattern.hpp Memory.hpp
	$(CCC) -pg -g -Wall -std=c++0x -c $< -o $@
$(OBJDIR)/MacroNode.o: MacroNode.cpp MacroNode.hpp
	$(CCC) -pg -g -Wall -std=c++0x -c $< -o $@
$(OBJDIR)/DataBlock.o: DataBlock.cpp DataBlock.hpp
	$(CCC) -pg -g -Wall -std=c++0x -c $< -o $@
$(OBJDIR)/MacroNodeTemplate.o: MacroNodeTemplate.cpp MacroNodeTemplate.hpp Scheduling.hpp ComputationGraph.hpp
	$(CCC) -pg -g -Wall -std=c++0x -c $< -o $@
$(OBJDIR)/Scheduling.o: Scheduling.cpp Scheduling.hpp Hardware.hpp Setting.hpp
	$(CCC) -pg -g -Wall -std=c++0x -c $< -o $@
$(OBJDIR)/Pattern.o: Pattern.cpp Pattern.hpp Setting.hpp DataBlock.hpp MacroNode.hpp
	$(CCC) -pg -g -Wall -std=c++0x -c $< -o $@
$(OBJDIR)/ComputationGraph.o: ComputationGraph.cpp ComputationGraph.hpp
	$(CCC) -pg -g -Wall -std=c++0x -c $< -o $@
$(OBJDIR)/Hardware.o: Hardware.cpp Hardware.hpp
	$(CCC) -pg -g -Wall -std=c++0x -c $< -o $@
$(OBJDIR)/ProcessConfig.o: ProcessConfig.cpp ProcessConfig.hpp
	$(CCC) -pg -g -Wall -std=c++0x -c $< -o $@

clean:
	rm $(OBJ)
