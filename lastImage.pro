#-------------------------------------------------
#
# Project created by QtCreator 2015-06-29T17:17:26
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = lastImage
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    imgabout.cpp \
    imgwin.cpp \
    sliderdialog.cpp \
    zoomdialog.cpp \
    imageeditlabel.cpp

HEADERS  += mainwindow.h \
    imageeditlabel.h \
    imgabout.h \
    imgwin.h \
    sliderdialog.h \
    zoomdialog.h

FORMS    += mainwindow.ui \
    imgabout.ui \
    imgresizedialog.ui \
    imgwin.ui \
    sliderdialog.ui \
    zoomdialog.ui

RESOURCES += \
    source.qrc \
    mysource.qrc \
    sour.qrc
