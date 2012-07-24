#-------------------------------------------------
#
# Project created by QtCreator 2012-06-16T13:54:32
#
#-------------------------------------------------

QT       += core gui

TARGET = ridual
target.path = /opt/extras.ubuntu.com/ridual
INSTALLS += target

data.path = /opt/extras.ubuntu.com/ridual/data
data.files = data/*
INSTALLS += data

install_desktop.files = extras-ridual.desktop
install_desktop.path = /usr/share/applications
INSTALLS += install_desktop

TEMPLATE = app


SOURCES += src/Main_window.cpp \
    src/Pane.cpp \
    src/File_list_model.cpp \
    src/hotkeys/Hotkey_editor.cpp \
    src/hotkeys/Hotkey_text_edit.cpp \
    src/hotkeys/Hotkey_item_delegate.cpp \
    src/hotkeys/Hotkeys.cpp \
    src/hotkeys/Hotkey.cpp \
    src/utils.cpp \
    src/File_info.cpp \
    src/gio/Volume.cpp \
    src/gio/Mount.cpp \
    src/Directory.cpp \
    src/Tasks_thread.cpp \
    src/Task.cpp \
    src/Path_button.cpp \
    src/Directory_list_task.cpp \
    src/Directory_watch_task.cpp \
    src/Path_widget.cpp \
    src/Special_uri.cpp \
    src/Columns.cpp \
    src/App_info.cpp \
    src/Settings_dialog.cpp \
    src/Copy_dialog.cpp \
    src/File_action_task.cpp \
    src/File_action_queue.cpp \
    src/Bookmarks_file_parser.cpp \
    src/Tasks_model.cpp \
    src/Directory_tree_item.cpp

HEADERS  += src/Main_window.h \
    src/Pane.h \
    src/File_list_model.h \
    src/hotkeys/Hotkey_editor.h \
    src/hotkeys/Hotkey_text_edit.h \
    src/hotkeys/Hotkey_item_delegate.h \
    src/hotkeys/Hotkeys.h \
    src/hotkeys/Hotkey.h \
    src/utils.h \
    src/File_info.h \
    src/gio/Volume.h \
    src/gio/Mount.h \
    src/Directory.h \
    src/Tasks_thread.h \
    src/Task.h \
    src/Path_button.h \
    src/qt_gtk.h \
    src/Directory_list_task.h \
    src/Directory_watch_task.h \
    src/Path_widget.h \
    src/Special_uri.h \
    src/Columns.h \
    src/App_info.h \
    src/Settings_dialog.h \
    src/Copy_dialog.h \
    src/File_action_task.h \
    src/File_action_queue.h \
    src/Bookmarks_file_parser.h \
    src/Tasks_model.h \
    src/Directory_tree_item.h

FORMS    += src/Main_window.ui \
    src/Pane.ui \
    src/hotkeys/Hotkey_editor.ui \
    src/Settings_dialog.ui \
    src/Copy_dialog.ui 


INCLUDEPATH += src

TRANSLATIONS += translations/ru.ts

#LIBS += -lmagic

CONFIG += link_pkgconfig
PKGCONFIG += gobject-2.0 gio-2.0 gtk+-2.0

enable_profiler { # gprof profiler
  QMAKE_CFLAGS+=-pg
  QMAKE_CXXFLAGS+=-pg
  QMAKE_LFLAGS+=-pg
}

testing {
  QT += testlib
  LIBS += -lgtest
  SOURCES += $$_PRO_FILE_PWD_/tests/*.cpp
  HEADERS += $$_PRO_FILE_PWD_/tests/*.h
  TARGET = ridual-test
  #DEFINES += "TEST_ENV_PATH=\\\"$$_PRO_FILE_PWD_/tests/env\\\""
} else {
  SOURCES += src/main.cpp
}



RESOURCES += \
    resources/1.qrc

OTHER_FILES += \
    README.md \
    doc/todo.txt \
    tests/run.sh
