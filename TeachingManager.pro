QT       += core gui sql

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

# 设置应用信息
TARGET = TeachingManager
TEMPLATE = app

# 添加新的源文件和头文件
SOURCES += \
    basewindow.cpp \
    utils.cpp \
    database.cpp \
    main.cpp \
    mainwindow.cpp \
    user.cpp \
    logindialog.cpp \
    studentwindow.cpp \
    teacherwindow.cpp

HEADERS += \
    basewindow.h \
    utils.h \
    database.h \
    mainwindow.h \
    user.h \
    logindialog.h \
    studentwindow.h \
    teacherwindow.h

FORMS += \
    logindialog.ui

# 添加资源文件
RESOURCES +=
