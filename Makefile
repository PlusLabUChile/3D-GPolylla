CMAKE=cmake
MAKE=make
BUILD_TYPE=Debug
BUILD_DIR=build
BIN=$(BUILD_DIR)/$(BUILD_TYPE)
MESH=socket

.PHONY: build init clean test run

all: run

init:
	@echo GENERATING BUILDSYSTEM \($(BUILD_TYPE)\)
	@$(CMAKE) --preset=default -DCMAKE_BUILD_TYPE=$(BUILD_TYPE) -S .

build.%: init
	@echo ====================================
	@echo BUILDING TARGET: $*
	@$(CMAKE) --build --preset=$*

test: build.default
	@echo ====================================
	@echo RUNNING TESTS
	@ctest --preset=default

clean: build.clean

run: build.demo
	@echo ====================================
	@echo RUNNING TARGET
	./build/demo/gpolylla_demo $(MESH) out/$(MESH)


