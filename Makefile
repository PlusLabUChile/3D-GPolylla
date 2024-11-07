CMAKE=cmake
MAKE=make
BUILD_TYPE=Debug
BUILD_DIR=build
BIN=$(BUILD_DIR)/$(BUILD_TYPE)

.PHONY: build init clean test run

all: run

3D-GPolyllaTest.init:
	@mkdir -p $(BIN)/
	@echo GENERATING BUILDSYSTEM \($(BUILD_TYPE)\) FOR LIBRARY
	@$(CMAKE) -DCMAKE_BUILD_TYPE=$(BUILD_TYPE) -DGPOLYLLA_LIB=1 -DBUILD_TESTING=1 -S . -B $(BIN)/ 
	@cp $(BIN)/compile_commands.json .

3D-GPolylla.init:
	@mkdir -p $(BIN)/
	@echo GENERATING BUILDSYSTEM \($(BUILD_TYPE)\) FOR EXECUTABLE
	$(CMAKE) -DCMAKE_BUILD_TYPE=$(BUILD_TYPE) -DGPOLYLLA_LIB=0 -DBUILD_TESTING=0 -S . -B $(BIN)/ 
	@cp $(BIN)/compile_commands.json .

%.build: %.init
	@echo ====================================
	@echo BUILDING TARGET: $*
	@$(CMAKE) --build $(BIN)/ -j 10 --target $*

test: 3D-GPolyllaTest.build
	@echo ====================================
	@echo RUNNING TESTS
	@ctest --test-dir $(BIN) --output-on-failure

clean:
	@$(CMAKE) --build $(BIN)/ -j 10 --target clean

run: 3D-GPolylla.build
	@echo ====================================
	@echo RUNNING TARGET
	./$(BIN)/src/3D-GPolylla data/1000points.1 minimal
