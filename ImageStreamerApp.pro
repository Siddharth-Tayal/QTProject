# Qt Modules Required
QT += core gui network widgets

# Project Configuration
CONFIG += c++11
TARGET = ZenQtApp

# --- Source Files ---
# List all .cpp files that need to be compiled.
SOURCES += \
    src/main.cpp \
    src/mainwindow.cpp \
    src/zen_worker.cpp \
    network/thread_stream.cpp \
    auth/serverDataProcess.cpp

# --- Header Files ---
# List all .h and .hpp files.
HEADERS += \
    src/mainwindow.h \
    src/zen_worker.h \
    network/thread_stream.hpp \
    auth/serverDataProcess.hpp \

# --- Link External Libraries ---
# Your thread_stream.cpp uses pthread_create, so we must link the pthread library.
# This is a crucial step for the build to succeed on Linux.
LIBS += -lpthread
