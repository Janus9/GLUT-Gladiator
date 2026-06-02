# Make Settings
MAKEFLAGS += -j16						# Parallel build with 8 jobs -- change as needed
SHELL := cmd.exe
.ONESHELL:

# Compiler Flags
CXX := g++ 
R_FLAGS := -std=c++20 -O0  																							# Release Flags
D_FLAGS := -std=c++20 -g -O0 -Wall -Wextra -D_GLIBCXX_DEBUG 		 												# Debug Flags																		
INCLUDE := -Iinclude -isystem common/include -isystem C:/msys64/uctr64/include										# Headers
LIB := -LC:/msys64/ucrt64/lib -Lcommon/lib -lglew32 -lfreeglut -lopengl32 -lglu32 -lwinmm -lgdi32  					# Libraries	    

# Directors
SRC_DIR := src
BIN_DIR := bin
BIN_R_DIR := bin/release
BIN_D_DIR := bin/debug
BUILD_DIR := build
BUILD_R_DIR := build\release
BUILD_D_DIR := build\debug
IMG_DIR := images
SHD_DIR := shaders
SND_DIR := sounds
DLL_DIR := common\bin
PUB_DIR ?= publish
SAV_DIR := saves
CUR_DIR := cursor

# Directory Groups 
DIRS_COMMON := $(BIN_DIR) $(BIN_D_DIR) $(BIN_R_DIR) $(BUILD_DIR) $(BUILD_D_DIR) $(BUILD_R_DIR)

# Files
MAIN_SRC = main.cpp
MAIN_BIN = main.o
OUTPUT = main.exe
PUBLISH_OUTPUT = Glut Gladiator.exe
SRCS := $(wildcard $(SRC_DIR)/*.cpp)												# List of source files
D_BINS := $(patsubst $(SRC_DIR)/%.cpp, $(BIN_D_DIR)/%.o, $(SRCS))					# List of binary files for debug
D_BINS += $(BIN_D_DIR)/$(MAIN_BIN)													# Adds main to binary list for debug

R_BINS := $(patsubst $(SRC_DIR)/%.cpp, $(BIN_R_DIR)/%.o, $(SRCS))					# List of binary files for release
R_BINS += $(BIN_R_DIR)/$(MAIN_BIN)													# Adds main to binary list for release


.PHONY: debug release clean publish

debug: | $(DIRS_COMMON)
debug: $(BUILD_D_DIR)/$(OUTPUT)	
release: | $(DIRS_COMMON)
release: $(BUILD_R_DIR)/$(OUTPUT)
publish: release | make_publish_dir
	@cmd /c "if exist $(PUB_DIR) rd /s /q $(PUB_DIR) && md $(PUB_DIR)"
	@copy "$(BUILD_R_DIR)\$(OUTPUT)" "$(PUB_DIR)\$(PUBLISH_OUTPUT)"
	@xcopy "$(DLL_DIR)\*" "$(PUB_DIR)" /Y
	@xcopy "$(IMG_DIR)" "$(PUB_DIR)" /E /I /Y
	@xcopy "$(SHD_DIR)" "$(PUB_DIR)" /E /I /Y
	@xcopy "$(SND_DIR)" "$(PUB_DIR)" /E /I /Y
	@xcopy "$(CUR_DIR)" "$(PUB_DIR)" /E /I /Y
	@md "$(PUB_DIR)\$(SAV_DIR)"
	@echo --------------------- PUBLISH ------------------------
	@echo            GLUT Gladiator Published Successfully!     
	@echo ------------------------------------------------------
	@echo  Executable built at $(PUB_DIR)\$(PUBLISH_OUTPUT) 
	@echo -----------------------------------------------------

# -- DEBUG -- #

# Debug Linking	
$(BUILD_D_DIR)/$(OUTPUT): $(D_BINS) | $(IMG_DIR) $(BUILD_D_DIR)
	$(CXX) $(D_BINS) $(LIB) -o $@
	@echo --------------------- DEBUG ------------------------
	@echo            Binaries Linked Successfully!            
	@echo ----------------------------------------------------
	@echo  Executable built at $(BUILD_D_DIR)\$(OUTPUT) 
	@echo ----------------------------------------------------

# Debug Compilation with src
$(BIN_D_DIR)/%.o: $(SRC_DIR)/%.cpp | $(BIN_D_DIR)
	$(CXX) -c $< $(INCLUDE) $(D_FLAGS) -o $@
	@echo Compiled $<

# Debug Compilation with main
$(BIN_D_DIR)/$(MAIN_BIN): $(MAIN_SRC) | $(BIN_D_DIR)
	$(CXX) -c $^ $(INCLUDE) $(D_FLAGS) -o $@
	@echo Compiled $^

# -- RELEASE -- #

# Release Linking
$(BUILD_R_DIR)/$(OUTPUT): $(R_BINS) | $(IMG_DIR) $(BUILD_R_DIR)
	$(CXX) $(R_BINS) $(LIB) -mwindows -o $@
	@echo -------------------- RELEASE -----------------------
	@echo            Binaries Linked Successfully!            
	@echo ----------------------------------------------------
	@echo  Executable built at $(BUILD_R_DIR)\$(OUTPUT) 
	@echo ----------------------------------------------------

# Release Compilation with src
$(BIN_R_DIR)/%.o: $(SRC_DIR)/%.cpp | $(BIN_R_DIR)
	$(CXX) -c $< $(INCLUDE) $(R_FLAGS) -o $@
	@echo Compiled $< Successfully!

# Release Compilation with main
$(BIN_R_DIR)/$(MAIN_BIN): $(MAIN_SRC) | $(BIN_R_DIR)
	$(CXX) -c $^ $(INCLUDE) $(R_FLAGS) -o $@
	@echo Compiled $^ Successfully!

# -- DIRECTORS -- #

# Makes bin directory
$(BIN_DIR):
	@if not exist "$@" md "$@"

# Makes bin/debug directory
$(BIN_D_DIR): | $(BIN_DIR)
	@if not exist "$@" md "$@"

# Makes bin/release directory
$(BIN_R_DIR): | $(BIN_DIR)
	@if not exist "$@" md "$@"

# Makes images directory
$(IMG_DIR):
	@if not exist "$@" md "$@"
 
# Makes shader directory
$(SHD_DIR):
	@if not exist "$@" md "$@"

# Makes build directory
$(BUILD_DIR):
	@if not exist "$@" md "$@"

# Makes publish directory
make_publish_dir:
	@if not exist "$(PUB_DIR)" md "$(PUB_DIR)"

# Makes debug directory inside of build
$(BUILD_D_DIR): | $(BUILD_DIR)
	@if not exist "$@" md "$@"

# Makes release directory inside of build
$(BUILD_R_DIR): | $(BUILD_DIR)
	@if not exist "$@" md "$@"

# Removes .o files
clean: | $(DIRS_COMMON)			# Force folders if missing
clean:
	@del /q "bin\debug\*.*"
	@del /q "bin\release\*.*"
	@del /q "build\debug\*.*"
	@del /q "build\release\*.*"
	@echo "-- Build Artifacts Cleaned Successfully --"