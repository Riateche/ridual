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
    hotkeys/Hotkey_text_edit.cpp \
    hotkeys/Hotkey_item_delegate.cpp \
    hotkeys/Hotkeys.cpp \
    hotkeys/Hotkey.cpp \
    utils.cpp \
    File_info.cpp \
    Gio.cpp

HEADERS  += Main_window.h \
    Pane.h \
    Read_directory_thread.h \
    File_list_model.h \
    hotkeys/Hotkey_editor.h \
    hotkeys/Hotkey_text_edit.h \
    hotkeys/Hotkey_item_delegate.h \
    hotkeys/Hotkeys.h \
    hotkeys/Hotkey.h \
    utils.h \
    File_info.h \
    Gio.h

FORMS    += Main_window.ui \
    Pane.ui \
    hotkeys/Hotkey_editor.ui

TRANSLATIONS += translations/ru.ts

LIBS += -lmagic

CONFIG += link_pkgconfig
PKGCONFIG += gobject-2.0 gio-2.0 gtk+-2.0

#QMAKE_CXXFLAGS += `pkg-config --libs gobject-2.0`
#LIBS += `pkg-config --libs gobject-2.0`
#QMAKE_CXXFLAGS += `pkg-config --libs glib-2.0 gio-2.0 gtk+-2.0`
#LIBS += -lgio-2.0 -lgobject-2.0 -lgtk+-2.0

RESOURCES += \
    resources/1.qrc
