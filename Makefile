CMAKE=cmake
BUILD_DIR=build
MAIN=gpol
MESH=3D_100

.PHONY: all clean build run

init:
	@mkdir -p $(BUILD_DIR)/
	$(CMAKE) -DCMAKE_BUILD_TYPE=$(TARGET) -B $(BUILD_DIR)/ .

build: init
	$(CMAKE) --build $(BUILD_DIR)/

run: build
	@./$(BUILD_DIR)/gpol $(MESH)

clean:
	rm -rf $(BUILD_DIR)
