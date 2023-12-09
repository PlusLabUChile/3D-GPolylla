CMAKE=cmake
BUILD_DIR=build
BUILD_TYPE=Debug
BIN_DIR:=$(BUILD_DIR)/$(BUILD_TYPE)
TARGET=gpol
MESH=3D_100

.PHONY: all clean build run

init:
	@mkdir -p $(BIN_DIR)/
	$(CMAKE) -DCMAKE_BUILD_TYPE=$(BUILD_TYPE) -B $(BIN_DIR) .
	@cp $(BIN_DIR)/compile_commands.json .

build: 
	$(CMAKE) --build $(BIN_DIR)/

run: build
	./$(BIN_DIR)/$(TARGET) $(MESH)

clean:
	rm -rf $(BUILD_DIR)
