QT += core xml
QT -= gui

TARGET = xsdqt-runtime
TEMPLATE = lib
CONFIG += staticlib c++14

DEFINES += QT_DEPRECATED_WARNINGS

SOURCES += \
    runtime/XmlHelpers.cpp

HEADERS += \
    runtime/XmlSerializable.h \
    runtime/XmlHelpers.h \
    runtime/XmlDocument.h

# Installation
unix {
    target.path = /usr/local/lib
    headers.path = /usr/local/include/xsdqt
    headers.files = $$HEADERS
    INSTALLS += target headers
}

win32 {
    target.path = $$[QT_INSTALL_LIBS]
    headers.path = $$[QT_INSTALL_HEADERS]/xsdqt
    headers.files = $$HEADERS
    INSTALLS += target headers
}
