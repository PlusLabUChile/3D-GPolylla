CMAKE=cmake
BUILD_TYPE=Debug
BUILD_DIR=build
BIN_DIR:=$(BUILD_DIR)/$(BUILD_TYPE)
TARGET=gpol
MESH=3D_100

.PHONY: all clean build run

init:
	@mkdir -p $(BUILD_DIR)/
	$(CMAKE) -DCMAKE_BUILD_TYPE=$(BUILD_TYPE) -B $(BIN_DIR)/ .
	@cp $(BIN_DIR)/compile_commands.json .

build:
	$(CMAKE) --build $(BIN_DIR)/

run: build
	./$(BIN_DIR)/gpol $(MESH)

clean:
	rm -rf $(BUILD_DIR)
