CC = gcc
CFLAGS = -Wall -Wextra -O3 -fopenmp -Isrc
LDFLAGS = -Lsrc/external -l:libraylib.a -lm -fopenmp -ldl -lpthread

BUILD_DIR = build
SRC_DIR = src

TARGET = $(BUILD_DIR)/main
THREAD_NUM = 50

SOURCES = $(wildcard $(SRC_DIR)/*.c) $(wildcard $(SRC_DIR)/simulation/*.c)

OBJECTS = $(patsubst $(SRC_DIR)/%.c,$(BUILD_DIR)/%.o,$(SOURCES))

all: $(TARGET)

# Create build directory structure
$(BUILD_DIR):
	@echo "Creating build directory structure..."
	@mkdir -p $(BUILD_DIR)
	@mkdir -p $(BUILD_DIR)/simulation

# Link object files to create executable
$(TARGET): $(OBJECTS) | $(BUILD_DIR)
	@echo "Linking executable: $(TARGET)"
	$(CC) $(OBJECTS) -o $(TARGET) $(LDFLAGS)
	@echo "Build successful! Executable created: $(TARGET)"

# Compile source files to object files (main src directory)
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c | $(BUILD_DIR)
	@echo "Compiling: $<"
	$(CC) $(CFLAGS) -c $< -o $@

# Compile source files in simulation subdirectory
$(BUILD_DIR)/simulation/%.o: $(SRC_DIR)/simulation/%.c | $(BUILD_DIR)
	@echo "Compiling: $<"
	@mkdir -p $(BUILD_DIR)/simulation
	$(CC) $(CFLAGS) -c $< -o $@

# Run the program with specified thread count
run: $(TARGET)
	@echo "Running $(TARGET) with $(THREAD_NUM) threads..."
	@export OMP_NUM_THREADS=$(THREAD_NUM) && $(TARGET)

# Clean up generated files
clean:
	@echo "Cleaning build directory..."
	@rm -rf $(BUILD_DIR)
	@echo "Clean complete."

# Rebuild everything from scratch
rebuild: clean all

# Show configuration
info:
	@echo "Configuration:"
	@echo "  Source directory:     $(SRC_DIR)"
	@echo "  Build directory:      $(BUILD_DIR)"
	@echo "  Target:               $(TARGET)"
	@echo "  Compiler:             $(CC)"
	@echo "  Compile Flags:        $(CFLAGS)"
	@echo "  Link Flags:           $(LDFLAGS)"
	@echo "  Thread Count:         $(THREAD_NUM)"
	@echo ""
	@echo "  Source files found:"
	@for file in $(SOURCES); do echo "    $$file"; done
	@echo ""
	@echo "  Static libraries:"
	@if [ -f "$(SRC_DIR)/libs/libraylib.a" ]; then \
		echo "    $(SRC_DIR)/libs/libraylib.a (found)"; \
	else \
		echo "    $(SRC_DIR)/libs/libraylib.a (missing)"; \
	fi

# Phony targets
.PHONY: all clean rebuild info run
