#-------------------------------------------------
#
# Project created by QtCreator 2015-09-03T13:32:24
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = MApRX_Qt
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    ../KssuFile.cpp \
    ../NitroLz.cpp \
    blockstore.cpp \
    dialogaboutme.cpp \
    dialogmakerom.cpp \
    dialogproperties.cpp \
    ../Nitro.cpp \
    ../cprs_lz.cpp \
    mainwindow_mapoperation.cpp \
    ../itemcatagory.cpp \
    dialogscripts.cpp \
    dialogresizemap.cpp \
    itemdictionary.cpp \
    mapview.cpp \
    itemimages.cpp

TRANSLATIONS = maprx_zh.ts

HEADERS  += mainwindow.h \
    ../KssuFile.h \
    ../Nitro.h \
    ../NitroLz.h \
    ../cprs_lz.h \
    blockstore.h \
    dialogaboutme.h \
    dialogmakerom.h \
    dialogproperties.h \
    dialogscripts.h \
    dialogresizemap.h \
    main.h \
    itemdictionary.h \
    mapview.h \
    itemimages.h \
    render_transit.h

FORMS    += mainwindow.ui \
    dialogaboutme.ui \
    dialogmakerom.ui \
    dialogproperties.ui \
    dialogscripts.ui \
    dialogresizemap.ui

RESOURCES += \
    resource.qrc

CONFIG += c++11 warn_on rtti_off

RC_FILE = winres.rc
