# ==========================================
# Makefile for Aether Motor Control Testing
# ==========================================

# Compiler to use
CXX = g++

# Compiler flags
CXXFLAGS = -Wall -Wextra -std=c++17 -O2

# Include directories (Current directory '.' and STServo)
INCLUDES = -I. -ISTServo

# Define the source files (Removed the 'motor_control/' prefix)
SRCS = main.cpp \
       motor_control.cpp \
       STServo/SMS_STS.cpp \
       STServo/SCS.cpp \
       STServo/SCSerial.cpp

# Define the object files
OBJS = $(SRCS:.cpp=.o)

# Name of the final executable file
TARGET = test_motor

# Default target
all: $(TARGET)

# Rule to link the object files and create the executable
$(TARGET): $(OBJS)
	@echo "Linking $(TARGET)..."
	$(CXX) $(CXXFLAGS) $(INCLUDES) -o $(TARGET) $(OBJS)
	@echo "Build successful! Run with ./$(TARGET)"

# Rule to compile the .cpp files into .o object files
%.o: %.cpp
	@echo "Compiling $<..."
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

# Clean rule to remove generated files
clean:
	@echo "Cleaning up..."
	rm -f $(OBJS) $(TARGET)