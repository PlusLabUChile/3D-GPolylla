CMAKE=cmake
MAKE=make
BUILD_TYPE=Debug
BUILD_DIR=build
BIN=$(BUILD_DIR)/$(BUILD_TYPE)
MESH=socket

.PHONY: build init clean test run

all: gpolylla.build

gpolylla_tests.init:
	@mkdir -p $(BIN)/
	@echo GENERATING BUILDSYSTEM \($(BUILD_TYPE)\)
	@$(CMAKE) -DCMAKE_BUILD_TYPE=$(BUILD_TYPE) -DBUILD_TESTING=1 -S . -B $(BIN)/
	@cp $(BIN)/compile_commands.json .

gpolylla_demo.init:
	@mkdir -p $(BIN)/
	@echo GENERATING BUILDSYSTEM \($(BUILD_TYPE)\) FOR DEMO
	$(CMAKE) -DCMAKE_BUILD_TYPE=$(BUILD_TYPE) -DGPOLYLLA_DEMO=1 -DBUILD_TESTING=0 -S . -B $(BIN)/
	@cp $(BIN)/compile_commands.json .

gpolylla.init:
	@mkdir -p $(BIN)/
	@echo GENERATING BUILDSYSTEM \($(BUILD_TYPE)\)
	@$(CMAKE) -DCMAKE_BUILD_TYPE=$(BUILD_TYPE) -DBUILD_TESTING=0 -S . -B $(BIN)/
	@cp $(BIN)/compile_commands.json .

%.build: %.init
	@echo ====================================
	@echo BUILDING TARGET: $*
	@$(CMAKE) --build $(BIN)/ -j 10 --target $*

test: gpolylla_tests.build
	@echo ====================================
	@echo RUNNING TESTS
	@ctest --test-dir $(BIN) --output-on-failure

clean:
	@$(CMAKE) --build $(BIN)/ -j 10 --target clean

run: gpolylla_demo.build
	@echo ====================================
	@echo RUNNING TARGET
	./$(BIN)/demo/gpolylla_demo $(MESH) out/$(MESH)


