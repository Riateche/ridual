#-------------------------------------------------
#
# Project created by QtCreator 2012-06-16T13:54:32
#
#-------------------------------------------------

QT       += core gui

TARGET = ridual
TEMPLATE = app


SOURCES += main.cpp\
        Main_window.cpp \
    Pane.cpp \
    Read_directory_thread.cpp \
    File_list_model.cpp

HEADERS  += Main_window.h \
    Pane.h \
    Read_directory_thread.h \
    File_list_model.h

FORMS    += Main_window.ui \
    Pane.ui

TRANSLATIONS += translations/ru.ts

