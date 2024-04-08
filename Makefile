# CMAKE=cmake
# BUILD_TYPE=Debug
# BUILD_DIR=build
# BIN_DIR:=$(BUILD_DIR)/$(BUILD_TYPE)
# TARGET=gpol
# MESH=socket.1
# OUT=test.off
#
# .PHONY: all clean build run autoload

# init:
# 	@mkdir -p $(BUILD_DIR)/
# 	$(CMAKE) -DCMAKE_BUILD_TYPE=$(BUILD_TYPE) -B $(BIN_DIR)/ .
# 	@cp $(BIN_DIR)/compile_commands.json .
#
# build:
# 	$(CMAKE) --build $(BIN_DIR) -j 10
#
# run: 
# 	./$(BIN_DIR)/src/$(TARGET) data/$(MESH) $(OUT)
#
# clean:
# 	rm -rf $(BUILD_DIR)
#
# autoload: 
# 	find src/ | entr -s "make build"
CMAKE=cmake
BUILD_TYPE=Debug
BUILD_DIR=build
BIN_DIR:=$(BUILD_DIR)/$(BUILD_TYPE)
BUILD=build
WDIRS=src/ test/ include/

.PHONY: build init clean test run

init:
	@mkdir -p $(BUILD_DIR)/
	$(CMAKE) -DCMAKE_BUILD_TYPE=$(BUILD_TYPE) -S . -B $(BIN_DIR)/ 
	@cp $(BIN_DIR)/compile_commands.json .

build:
	cmake --build $(BIN_DIR) -j 10

test:
	@GTEST_COLOR=1 ctest --test-dir $(BIN_DIR) --output-on-failure

clean:
	rm -rf .cache Testing build

watch: init
	@find $(WDIRS) | entr -s "make build"
