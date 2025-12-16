QT       += core gui sql

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

TARGET = TeachingManager
TEMPLATE = app

# 只保留实际使用的文件
SOURCES += \
    basewindow.cpp \
    database.cpp \
    main.cpp \
    mainwindow.cpp \
    user.cpp \
    logindialog.cpp \
    studentwindow.cpp \
    teacherwindow.cpp

HEADERS += \
    basewindow.h \
    database.h \
    mainwindow.h \
    user.h \
    logindialog.h \
    studentwindow.h \
    teacherwindow.h

FORMS += \
    logindialog.ui

RESOURCES +=
