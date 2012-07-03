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
    File_list_model.cpp \
    hotkeys/Hotkey_editor.cpp \
    hotkeys/Hotkey_text_edit.cpp \
    hotkeys/Hotkey_item_delegate.cpp \
    hotkeys/Hotkeys.cpp \
    hotkeys/Hotkey.cpp \
    utils.cpp \
    File_info.cpp \
    gio/Volume.cpp \
    gio/Mount.cpp \
    Directory.cpp \
    Tasks_thread.cpp \
    Task.cpp \
    Path_button.cpp \
    Directory_list_task.cpp \
    Directory_watch_task.cpp \
    Path_widget.cpp \
    Bookmarks.cpp \
    Special_uri.cpp \
    Columns.cpp \
    App_info.cpp \
    Settings_dialog.cpp

HEADERS  += Main_window.h \
    Pane.h \
    File_list_model.h \
    hotkeys/Hotkey_editor.h \
    hotkeys/Hotkey_text_edit.h \
    hotkeys/Hotkey_item_delegate.h \
    hotkeys/Hotkeys.h \
    hotkeys/Hotkey.h \
    utils.h \
    File_info.h \
    gio/Volume.h \
    gio/Mount.h \
    Directory.h \
    Tasks_thread.h \
    Task.h \
    Path_button.h \
    qt_gtk.h \
    Directory_list_task.h \
    Directory_watch_task.h \
    Path_widget.h \
    Bookmarks.h \
    Special_uri.h \
    Columns.h \
    App_info.h \
    Settings_dialog.h

FORMS    += Main_window.ui \
    Pane.ui \
    hotkeys/Hotkey_editor.ui \
    Settings_dialog.ui

TRANSLATIONS += translations/ru.ts

LIBS += -lmagic

CONFIG += link_pkgconfig
PKGCONFIG += gobject-2.0 gio-2.0 gtk+-2.0

#QMAKE_CXXFLAGS += `pkg-config --libs gobject-2.0`
#LIBS += `pkg-config --libs gobject-2.0`
#QMAKE_CXXFLAGS += `pkg-config --libs glib-2.0 gio-2.0 gtk+-2.0`
#LIBS += -lgio-2.0 -lgobject-2.0 -lgtk+-2.0

enable_profiler { # gprof profiler
  QMAKE_CFLAGS+=-pg
  QMAKE_CXXFLAGS+=-pg
  QMAKE_LFLAGS+=-pg
}



RESOURCES += \
    resources/1.qrc

OTHER_FILES += \
    ../notes.txt
