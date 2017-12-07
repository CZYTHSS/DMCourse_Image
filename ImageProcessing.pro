#-------------------------------------------------
#
# Project created by QtCreator 2017-10-11T15:55:00
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = ImageProcessing
TEMPLATE = app

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

INCLUDEPATH += $$(OPENCV_HOME)/include/

LIBS += $$(OPENCV_HOME)/x64/vc14/lib/opencv_world320d.lib
LIBS += $$(OPENCV_HOME)/x64/vc14/lib/opencv_world320.lib

SOURCES += \
        main.cpp \
        mainwindow.cpp \
    dialog.cpp \
    lineardialog.cpp \
    nonlineardialog.cpp \
    histogramdialog.cpp \
    utils.cpp

HEADERS += \
        mainwindow.h \
    dialog.h \
    lineardialog.h \
    nonlineardialog.h \
    histogramdialog.h \
    utils.h

FORMS += \
        mainwindow.ui \
    dialog.ui \
    lineardialog.ui \
    nonlineardialog.ui \
    histogramdialog.ui
