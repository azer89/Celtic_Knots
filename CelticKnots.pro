#-------------------------------------------------
#
# Project created by QtCreator 2015-06-18T14:04:57
#
#-------------------------------------------------

QT       += core gui opengl svg

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = CelticKnots
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    GLContainer.cpp \
    GLWidget.cpp \
    TilePainter.cpp

HEADERS  += mainwindow.h \
    GLContainer.h \
    GLWidget.h \
    AVector.h \
    VertexData.h \
    ALine.h \
    CCell.h \
    AnIndex.h \
    TileType.h \
    DirectionType.h \
    TilePainter.h

FORMS    += mainwindow.ui

QMAKE_CXXFLAGS += -frounding-math -O3

QMAKE_CXXFLAGS += -std=gnu++1y
