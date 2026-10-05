CC      ?= gcc
CFLAGS  += -std=c11 -Wall -g
ROOT    := ..
UNITY   := vendor/unity
BUILD   := build

INCLUDES := -I$(ROOT)/include -I$(UNITY)

TEST_FLASH_SRCS := test_flash_driver.c $(ROOT)/src/flash_driver.c $(UNITY)/unity.c

.PHONY: test clean

test: $(BUILD)/test_flash_driver
	./$(BUILD)/test_flash_driver

$(BUILD)/test_flash_driver: $(TEST_FLASH_SRCS) | $(BUILD)
	$(CC) $(CFLAGS) $(INCLUDES) $^ -o $@

$(BUILD):
	mkdir -p $@

clean:
	rm -rf $(BUILD)
