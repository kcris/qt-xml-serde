TEMPLATE = subdirs

SUBDIRS += \
    runtime \
    generator \
    tests

runtime.file = xsdqt-runtime.pro
generator.file = xsdqt-generator.pro
tests.file = tests/xsdqt-tests.pro

# Build order
generator.depends = runtime
tests.depends = runtime

CONFIG += ordered
