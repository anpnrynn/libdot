PREFIX ?= /usr/local
BUILD_DIR ?= build

.PHONY: all test clean install

all:
	cmake -S . -B $(BUILD_DIR) -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=$(PREFIX)
	cmake --build $(BUILD_DIR)

test: all
	ctest --test-dir $(BUILD_DIR) --output-on-failure

install: all
	cmake --install $(BUILD_DIR)

clean:
	rm -rf $(BUILD_DIR)
