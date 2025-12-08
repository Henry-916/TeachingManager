QT       += core gui sql

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

# 设置应用信息
TARGET = TeachingManager
TEMPLATE = app

SOURCES += \
    main.cpp \
    mainwindow.cpp \
    database.cpp

HEADERS += \
    common.h \
    mainwindow.h \
    database.h

# 添加资源文件（用于数据库配置）
RESOURCES +=
