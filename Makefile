# Make Settings
MAKEFLAGS += -j16						# Parallel build with 8 jobs -- change as needed
SHELL := powershell.exe
.SHELLFLAGS := -NoProfile -Command
.ONESHELL:

# Compiler Flags
CXX := g++ 
R_FLAGS := -O0 -DGLEW_STATIC 																				# Release Flags
D_FLAGS := -g -O0 -DGLEW_STATIC																				# Debug Flags																		# Debug Flags
INCLUDE := -Iinclude -isystem common/include 																# Headers
LIB := -Lcommon/lib -lglew32s -lSoil -lfreeglut -lopengl32 -lglu32 -lwinmm -lgdi32 -lirrKlang  			    # Libraries

# Directors
SRC_DIR := src
BIN_DIR := bin
BIN_R_DIR := bin/release
BIN_D_DIR := bin/debug
BUILD_DIR := build
BUILD_R_DIR := build\release
BUILD_D_DIR := build\debug
IMG_DIR := images
DLL_DIR := common\bin

# Files
MAIN_SRC = main.cpp
MAIN_BIN = main.o
OUTPUT = main.exe
SRCS := $(wildcard $(SRC_DIR)/*.cpp)												# List of source files
D_BINS := $(patsubst $(SRC_DIR)/%.cpp, $(BIN_D_DIR)/%.o, $(SRCS))					# List of binary files for debug
D_BINS += $(BIN_D_DIR)/$(MAIN_BIN)													# Adds main to binary list for debug

R_BINS := $(patsubst $(SRC_DIR)/%.cpp, $(BIN_R_DIR)/%.o, $(SRCS))					# List of binary files for release
R_BINS += $(BIN_R_DIR)/$(MAIN_BIN)													# Adds main to binary list for release


.PHONY: debug release clean

debug: $(BUILD_D_DIR)/$(OUTPUT)	
release: $(BUILD_R_DIR)/$(OUTPUT)												

# -- DEBUG -- #

# Debug Linking	
$(BUILD_D_DIR)/$(OUTPUT): $(D_BINS) | $(IMG_DIR) $(BUILD_D_DIR)
	$(CXX) $(D_BINS) $(LIB) -o $@
	@Copy-Item -Path "$(DLL_DIR)\irrKlang.dll" -Destination "$(BUILD_D_DIR)"
	@Copy-Item -Path "$(DLL_DIR)\ikpMP3.dll" -Destination "$(BUILD_D_DIR)"
	@Copy-Item -Path "$(DLL_DIR)\ikpFlac.dll" -Destination "$(BUILD_D_DIR)"
	@Write-Host " >>--------------------- DEBUG ------------------------<< "
	@Write-Host " >>           Binaries Linked Successfully!            << "
	@Write-Host " >>----------------------------------------------------<< "
	@Write-Host " >> Executable built at >> $(BUILD_D_DIR)\$(OUTPUT) << "
	@Write-Host " >>----------------------------------------------------<< "

# Debug Compilation with src
$(BIN_D_DIR)/%.o: $(SRC_DIR)/%.cpp | $(BIN_D_DIR)
	$(CXX) -c $< $(INCLUDE) $(D_FLAGS) -o $@
	@Write-Host "Compiled $<"

# Debug Compilation with main
$(BIN_D_DIR)/$(MAIN_BIN): $(MAIN_SRC) | $(BIN_D_DIR)
	$(CXX) -c $^ $(INCLUDE) $(D_FLAGS) -o $@
	@Write-Host "Compiled $^"

# -- RELEASE -- #

# Release Linking
$(BUILD_R_DIR)/$(OUTPUT): $(R_BINS) | $(IMG_DIR) $(BUILD_R_DIR)
	$(CXX) $(R_BINS) $(LIB) -mwindows -o $@
	@Copy-Item -Path "$(DLL_DIR)\irrKlang.dll" -Destination "$(BUILD_R_DIR)"
	@Copy-Item -Path "$(DLL_DIR)\ikpMP3.dll" -Destination "$(BUILD_R_DIR)"
	@Copy-Item -Path "$(DLL_DIR)\ikpFlac.dll" -Destination "$(BUILD_R_DIR)"
	@Write-Host " >>-------------------- RELEASE -----------------------<< "
	@Write-Host " >>           Binaries Linked Successfully!            << "
	@Write-Host " >>----------------------------------------------------<< "
	@Write-Host " >> Executable built at >> $(BUILD_R_DIR)\$(OUTPUT) << "
	@Write-Host " >>----------------------------------------------------<< "

# Release Compilation with src
$(BIN_R_DIR)/%.o: $(SRC_DIR)/%.cpp | $(BIN_R_DIR)
	$(CXX) -c $< $(INCLUDE) $(R_FLAGS) -o $@
	@Write-Host "Compiled $< Successfully!"

# Release Compilation with main
$(BIN_R_DIR)/$(MAIN_BIN): $(MAIN_SRC) | $(BIN_R_DIR)
	$(CXX) -c $^ $(INCLUDE) $(R_FLAGS) -o $@
	@Write-Host "Compiled $^ Successfully!"


# -- DIRECTORS -- #

# Makes bin directory
$(BIN_DIR):
	@echo "Missing $@ folder, creating a new one now . . . "
	New-Item -ItemType Directory -Force -Path $@ | Out-Null

# Makes bin/debug directory
$(BIN_D_DIR): | $(BIN_DIR)
	@echo "Missing $@ folder, creating a new one now . . . "
	New-Item -ItemType Directory -Force -Path $@ | Out-Null

# Makes bin/release directory
$(BIN_R_DIR): | $(BIN_DIR)
	@echo "Missing $@ folder, creating a new one now . . . "
	New-Item -ItemType Directory -Force -Path $@ | Out-Null

# Makes images directory
$(IMG_DIR):
	@echo "Missing $@ folder, creating a new one now . . . "
	New-Item -ItemType Directory -Force -Path $@ | Out-Null
 
# Makes build directory
$(BUILD_DIR):
	@echo "Missing $@ folder, creating a new one now . . . "
	New-Item -ItemType Directory -Force -Path $@ | Out-Null

# Makes debug directory inside of build
$(BUILD_D_DIR): | $(BUILD_DIR)
	@echo "Missing $@ folder, creating a new one now . . . "
	New-Item -ItemType Directory -Force -Path $@ | Out-Null

# Makes release directory inside of build
$(BUILD_R_DIR): | $(BUILD_DIR)
	@echo "Missing $@ folder, creating a new one now . . . "
	New-Item -ItemType Directory -Force -Path $@ | Out-Null

# Removes .o files
clean:
	@Remove-Item $(BUILD_D_DIR)\*
	@echo "-- Debug folder cleaned successfully -- "
	@Remove-Item $(BUILD_R_DIR)\*
	@echo "-- Release folder cleaned successfully -- "