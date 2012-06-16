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
    File_list_model.cpp \
    hotkeys/Hotkey_editor.cpp \
    hotkeys/Hotkey_editor_model.cpp \
    hotkeys/Hotkey_text_edit.cpp \
    hotkeys/Hotkey_item_delegate.cpp

HEADERS  += Main_window.h \
    Pane.h \
    Read_directory_thread.h \
    File_list_model.h \
    hotkeys/Hotkey_editor.h \
    hotkeys/Hotkey_editor_model.h \
    hotkeys/Hotkey_text_edit.h \
    hotkeys/Hotkey_item_delegate.h

FORMS    += Main_window.ui \
    Pane.ui \
    hotkeys/Hotkey_editor.ui

TRANSLATIONS += translations/ru.ts

