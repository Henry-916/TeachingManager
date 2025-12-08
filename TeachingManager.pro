QT       += core gui sql

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

# 设置应用信息
TARGET = TeachingManager
TEMPLATE = app

# 添加新的源文件和头文件
SOURCES += \
    main.cpp \
    mainwindow.cpp \
    database.cpp \
    user.cpp \
    logindialog.cpp

HEADERS += \
    common.h \
    mainwindow.h \
    database.h \
    user.h \
    logindialog.h

FORMS += \
    logindialog.ui  # 去掉重复的行

# 添加资源文件（用于数据库配置）
RESOURCES +=
