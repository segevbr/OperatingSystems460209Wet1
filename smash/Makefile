# Change the C compiler (CC) to the C++ compiler (CXX)
CXX = g++

# Update the compiler flags (CXXFLAGS) to be C++-appropriate
# Using -std=c++11 or higher is common for modern C++ projects.
CXXFLAGS = -std=c++17 -Wall -Werror -pedantic-errors -pthread -DNDEBUG

# Use wildcard to find all .cpp source files
SRCS = $(wildcard *.cpp)

# Generate the list of object files (.o) from the list of source files (.cpp)
# The substitution must change .cpp to .o
OBJS = $(SRCS:.cpp=.o)

TARGET = smash

# --- Build Targets ---

# Default target: builds the executable
all: $(TARGET)

# Rule to link the object files into the final executable
$(TARGET): $(OBJS)
	# Use CXX for linking
	$(CXX) $(CXXFLAGS) $(OBJS) -o $(TARGET)

# Rule to compile each .cpp file into a .o object file (Pattern Rule)
# This rule will be used for files like commands.o, signals.o, smash.o, etc.
%.o: %.cpp
	# Use CXX for compilation
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Rule to clean up the generated files
clean:
	# Use -f option for rm to prevent errors if files don't exist
	rm -f $(TARGET) $(OBJS)
	# The original list of files also includes a my_system_call.o, which is not in the OBJS list.
	# It's good practice to ensure all generated files are cleaned.
	rm -f my_system_call.o