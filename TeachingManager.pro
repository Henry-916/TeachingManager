QT       += core gui sql

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

TARGET = TeachingManager
TEMPLATE = app

# MySQL配置
INCLUDEPATH += "C:\Program Files\MySQL\MySQL Server 8.0\include"
LIBS += -L"C:\Program Files\MySQL\MySQL Server 8.0\lib" -llibmysql

SOURCES += \
    basewindow.cpp \
    configmanager.cpp \
    database.cpp \
    main.cpp \
    mainwindow.cpp \
    user.cpp \
    logindialog.cpp \
    studentwindow.cpp \
    teacherwindow.cpp

HEADERS += \
    basewindow.h \
    configmanager.h \
    database.h \
    mainwindow.h \
    user.h \
    logindialog.h \
    studentwindow.h \
    teacherwindow.h

FORMS += \
    logindialog.ui

RESOURCES +=
