TARGET = gitsync
SOURCES = src/github_scanner.c

# Compiler detection
ifeq ($(shell which clang >/dev/null 2>&1; echo $$?), 0)
    CC = clang
    CFLAGS = -Werror -Wall -Wextra -std=c99 -D_GNU_SOURCE
else
    CC = gcc
    CFLAGS = -Wall -Wextra -std=c99 -D_GNU_SOURCE
endif

# Performance and optimization flags
OPTIMIZE ?= false
LTO ?= false
STATIC ?= false

ifeq ($(OPTIMIZE),true)
    CFLAGS += -O2 -march=native
endif

ifeq ($(LTO),true)
    CFLAGS += -flto
    LDFLAGS += -flto
endif

ifeq ($(STATIC),true)
    LDFLAGS += -static
endif

# Default build
$(TARGET): $(SOURCES)
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS)

# Static analysis targets
clang-tidy:
	@echo "Running clang-tidy analysis..."
	@if ! command -v clang-tidy >/dev/null 2>&1; then \
		echo "clang-tidy not found. Install with: apt install clang-tidy or brew install llvm"; \
		exit 1; \
	fi
	clang-tidy src/github_scanner.c --checks='*,-clang-analyzer-security.insecureAPI.*' 2>&1 || true

cppcheck:
	@echo "Running cppcheck analysis..."
	@if ! command -v cppcheck >/dev/null 2>&1; then \
		echo "cppcheck not found. Install with: apt install cppcheck or brew install cppcheck"; \
		exit 1; \
	fi
	cppcheck --enable=all --std=c99 $(SOURCES)

analyze: clang-tidy cppcheck

# Development builds
debug: CFLAGS += -g -fsanitize=address
debug: LDFLAGS += -fsanitize=address
debug: $(TARGET)

test: debug
	@echo "Running tests..."
	./$(TARGET) --help || true

format:
	@echo "Formatting source code..."
	@if ! command -v clang-format >/dev/null 2>&1; then \
		echo "clang-format not found. Install with: apt install clang-format or brew install clang-format"; \
		exit 1; \
	fi
	clang-format -i $(SOURCES)

# Optimization builds
optimized: OPTIMIZE=true
optimized: LTO=true
optimized: $(TARGET)

# Static build
static-build: STATIC=true
static-build: $(TARGET)

# Installation targets
PREFIX ?= /usr/local
BINDIR = $(PREFIX)/bin

install: $(TARGET)
	install -D $(TARGET) $(BINDIR)/$(TARGET)

uninstall:
	rm -f $(BINDIR)/$(TARGET)

# Distribution
VERSION ?= 1.0.0
DIST_NAME = $(TARGET)-$(VERSION)

dist: clean
	mkdir -p $(DIST_NAME)
	cp -r $(SOURCES) Makefile README.md $(DIST_NAME)/
	tar czf $(DIST_NAME).tar.gz $(DIST_NAME)
	rm -rf $(DIST_NAME)

# Cleanup
clean:
	rm -f $(TARGET) $(TARGET)-*.tar.gz

# Help target
help:
	@echo "Available targets:"
	@echo "  (default)     - Build $(TARGET)"
	@echo "  debug         - Build with debug flags and address sanitizer"
	@echo "  optimized     - Build with -O2 -march=native and LTO"
	@echo "  static-build  - Build with static linking"
	@echo "  test          - Run debug build and basic tests"
	@echo "  format        - Format source code with clang-format"
	@echo "  clang-tidy    - Run static analysis with clang-tidy"
	@echo "  cppcheck      - Run static analysis with cppcheck"
	@echo "  analyze       - Run both clang-tidy and cppcheck"
	@echo "  install       - Install to $(BINDIR)"
	@echo "  uninstall     - Remove from $(BINDIR)"
	@echo "  dist          - Create distribution package"
	@echo "  clean         - Remove build artifacts"
	@echo "  help          - Show this help message"
	@echo ""
	@echo "Variables:"
	@echo "  CC            - Compiler (auto-detected: $(CC))"
	@echo "  OPTIMIZE      - Enable optimizations (false/true, default: $(OPTIMIZE))"
	@echo "  LTO           - Enable Link Time Optimization (false/true, default: $(LTO))"
	@echo "  STATIC        - Enable static linking (false/true, default: $(STATIC))"
	@echo "  PREFIX        - Install prefix (default: $(PREFIX))"

.PHONY: clean test debug format clang-tidy cppcheck analyze optimized static-build install uninstall dist help