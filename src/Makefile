# Makefile for XSD to C++ Mapping Library
# Alternative to qmake-based build

# Qt configuration
QT_PATH ?= /usr
QT_INCPATH = $(QT_PATH)/include/qt5
QT_LIBPATH = $(QT_PATH)/lib

# Compiler and flags
CXX ?= g++
CXXFLAGS = -std=c++14 -Wall -Wextra -O2 -fPIC
INCLUDES = -I$(QT_INCPATH) -I$(QT_INCPATH)/QtCore -I$(QT_INCPATH)/QtXml -Iruntime
LIBS = -L$(QT_LIBPATH) -lQt5Core -lQt5Xml -lpthread

# Directories
BUILD_DIR = build
RUNTIME_DIR = runtime
GENERATOR_DIR = generator
TEST_DIR = tests

# Output files
RUNTIME_LIB = $(BUILD_DIR)/libxsdqt-runtime.a
GENERATOR_BIN = $(BUILD_DIR)/xsd2cpp
TEST_BIN = $(BUILD_DIR)/test_xsdqt

# Runtime library sources
RUNTIME_SRCS = $(RUNTIME_DIR)/XmlHelpers.cpp
RUNTIME_OBJS = $(patsubst $(RUNTIME_DIR)/%.cpp,$(BUILD_DIR)/runtime/%.o,$(RUNTIME_SRCS))

# Generator sources
GENERATOR_SRCS = \
	$(GENERATOR_DIR)/main.cpp \
	$(GENERATOR_DIR)/XsdParser.cpp \
	$(GENERATOR_DIR)/CodeGenerator.cpp
GENERATOR_OBJS = $(patsubst $(GENERATOR_DIR)/%.cpp,$(BUILD_DIR)/generator/%.o,$(GENERATOR_SRCS))

# Test sources (would need actual test files)
TEST_SRCS = $(TEST_DIR)/TestXmlSerialization.cpp
TEST_OBJS = $(patsubst $(TEST_DIR)/%.cpp,$(BUILD_DIR)/tests/%.o,$(TEST_SRCS))

# MOC files (Qt Meta-Object Compiler)
MOC = moc-qt5
TEST_MOC = $(BUILD_DIR)/tests/TestXmlSerialization.moc

# Default target
all: directories $(RUNTIME_LIB) $(GENERATOR_BIN)

# Create build directories
directories:
	@mkdir -p $(BUILD_DIR)/runtime
	@mkdir -p $(BUILD_DIR)/generator
	@mkdir -p $(BUILD_DIR)/tests

# Runtime library
$(RUNTIME_LIB): $(RUNTIME_OBJS)
	@echo "Creating runtime library..."
	ar rcs $@ $^
	@echo "Runtime library created: $@"

$(BUILD_DIR)/runtime/%.o: $(RUNTIME_DIR)/%.cpp
	@echo "Compiling $<..."
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

# Generator executable
$(GENERATOR_BIN): $(GENERATOR_OBJS)
	@echo "Linking generator..."
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LIBS)
	@echo "Generator created: $@"

$(BUILD_DIR)/generator/%.o: $(GENERATOR_DIR)/%.cpp
	@echo "Compiling $<..."
	$(CXX) $(CXXFLAGS) $(INCLUDES) -I$(GENERATOR_DIR) -c $< -o $@

# Tests
tests: $(TEST_BIN)

$(TEST_BIN): $(TEST_OBJS) $(RUNTIME_LIB)
	@echo "Linking tests..."
	$(CXX) $(CXXFLAGS) -o $@ $(TEST_OBJS) $(RUNTIME_LIB) $(LIBS) -lQt5Test
	@echo "Tests created: $@"

$(BUILD_DIR)/tests/%.o: $(TEST_DIR)/%.cpp $(TEST_MOC)
	@echo "Compiling $<..."
	$(CXX) $(CXXFLAGS) $(INCLUDES) -I$(TEST_DIR) -c $< -o $@

$(TEST_MOC): $(TEST_DIR)/TestXmlSerialization.cpp
	@echo "Generating MOC file..."
	$(MOC) $(TEST_DIR)/TestXmlSerialization.cpp -o $@

# Run tests
check: tests
	@echo "Running tests..."
	@$(TEST_BIN)

# Install
PREFIX ?= /usr/local
INSTALL_BIN = $(PREFIX)/bin
INSTALL_LIB = $(PREFIX)/lib
INSTALL_INC = $(PREFIX)/include/xsdqt

install: all
	@echo "Installing to $(PREFIX)..."
	install -d $(INSTALL_BIN)
	install -d $(INSTALL_LIB)
	install -d $(INSTALL_INC)
	install -m 755 $(GENERATOR_BIN) $(INSTALL_BIN)/
	install -m 644 $(RUNTIME_LIB) $(INSTALL_LIB)/
	install -m 644 $(RUNTIME_DIR)/*.h $(INSTALL_INC)/
	@echo "Installation complete!"

# Uninstall
uninstall:
	@echo "Uninstalling..."
	rm -f $(INSTALL_BIN)/xsd2cpp
	rm -f $(INSTALL_LIB)/libxsdqt-runtime.a
	rm -rf $(INSTALL_INC)
	@echo "Uninstall complete!"

# Clean
clean:
	@echo "Cleaning build files..."
	rm -rf $(BUILD_DIR)

distclean: clean
	@echo "Cleaning all generated files..."
	rm -f Makefile.bak

# Example: Generate code from sample XSD
example: $(GENERATOR_BIN)
	@echo "Generating code from vehicle.xsd..."
	@mkdir -p generated
	$(GENERATOR_BIN) -i tests/vehicle.xsd -o generated/ -n Example
	@echo "Generated code in generated/"

# Help
help:
	@echo "XSD to C++ Mapping Library - Makefile"
	@echo ""
	@echo "Targets:"
	@echo "  all (default) - Build runtime library and generator"
	@echo "  tests         - Build and link test suite"
	@echo "  check         - Build and run tests"
	@echo "  install       - Install library and tool (requires root)"
	@echo "  uninstall     - Remove installed files"
	@echo "  clean         - Remove build files"
	@echo "  distclean     - Remove all generated files"
	@echo "  example       - Generate code from sample XSD"
	@echo "  help          - Show this help"
	@echo ""
	@echo "Variables:"
	@echo "  QT_PATH=$(QT_PATH)"
	@echo "  PREFIX=$(PREFIX)"
	@echo "  CXX=$(CXX)"
	@echo ""
	@echo "Examples:"
	@echo "  make
