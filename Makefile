CMAKE=cmake
MAKE=make
BUILD_TYPE=Debug
BUILD_DIR=build
BIN_DIR:=$(BUILD_DIR)/$(BUILD_TYPE)
BUILD=build
WDIRS=src/ test/ include/

.PHONY: build init clean test run

init:
	@mkdir -p $(BUILD_DIR)/
	@$(CMAKE) -DCMAKE_BUILD_TYPE=$(BUILD_TYPE) -DGPOLYLLA_LIB=1 -DBUILD_TESTING=1 -S . -B $(BIN_DIR)/ 
	@cp $(BIN_DIR)/compile_commands.json .
build:
	cd $(BIN_DIR) && make
test: build
	@GTEST_COLOR=1 ctest --test-dir $(BIN_DIR) --output-on-failure
clean:
	cd $(BIN_DIR) && make clean
watch: init
	@find $(WDIRS) | entr -s "make build"
