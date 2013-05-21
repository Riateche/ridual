#-------------------------------------------------
#
# Project created by QtCreator 2012-06-16T13:54:32
#
#-------------------------------------------------

QT       += core gui

TARGET = ridual
target.path = /usr/bin
INSTALLS += target

data.path = /usr/share/ridual/data
data.files = data/*
INSTALLS += data

install_desktop.files = ridual.desktop
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
    src/Task.cpp \
    src/Path_button.cpp \
    src/Directory_list_task.cpp \
    src/Path_widget.cpp \
    src/Special_uri.cpp \
    src/Columns.cpp \
    src/App_info.cpp \
    src/Settings_dialog.cpp \
    src/Bookmarks_file_parser.cpp \
    src/Task_details.cpp \
    src/Error_reaction.cpp \
    src/File_leaf.cpp \
    src/Directory_watcher.cpp \
    src/Action_queue.cpp \
    src/Action.cpp \
    src/Action_answerer.cpp \
    src/mkdir.cpp \
    src/Question_widget.cpp \
    src/Current_queue_question.cpp \
    src/Core.cpp \
    src/Mount_manager.cpp \
    src/Core_ally.cpp \
    src/Message_widget.cpp \
    src/Action_state_widget.cpp \
    src/Elapsed_timer.cpp \
    src/Actions_manager.cpp \
    src/File_system_engine.cpp \
    src/Real_file_system_engine.cpp \
    src/gio/Gio_file_system_engine.cpp \
    src/gio/Gio_mounter.cpp

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
    src/Task.h \
    src/Path_button.h \
    src/qt_gtk.h \
    src/Directory_list_task.h \
    src/Path_widget.h \
    src/Special_uri.h \
    src/Columns.h \
    src/App_info.h \
    src/Settings_dialog.h \
    src/Bookmarks_file_parser.h \
    src/Task_details.h \
    src/Error_reaction.h \
    src/File_leaf.h \
    src/Directory_watcher.h \
    src/Action.h \
    src/Action_queue.h \
    src/Action_answerer.h \
    src/types.h \
    src/mkdir.h \
    src/Question_widget.h \
    src/Current_queue_question.h \
    src/Core.h \
    src/Mount_manager.h \
    src/Core_ally.h \
    src/Message_widget.h \
    src/Action_state_widget.h \
    src/Elapsed_timer.h \
    src/Actions_manager.h \
    src/File_system_engine.h \
    src/Real_file_system_engine.h \
    src/gio/Gio_file_system_engine.h \
    src/gio/Gio_mounter.h

FORMS    += src/Main_window.ui \
    src/Pane.ui \
    src/hotkeys/Hotkey_editor.ui \
    src/Settings_dialog.ui \
    src/Task_details.ui \
    src/Question_widget.ui \
    src/Action_state_widget.ui


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
#  LIBS += -lgtest
  INCLUDEPATH += $$_PRO_FILE_PWD_/tests/gtest-1.6.0
  INCLUDEPATH += $$_PRO_FILE_PWD_/tests/gtest-1.6.0/include
  SOURCES += $$_PRO_FILE_PWD_/tests/gtest-1.6.0/src/gtest-all.cc
  SOURCES += $$_PRO_FILE_PWD_/tests/*.cpp
  HEADERS += $$_PRO_FILE_PWD_/tests/*.h
  TARGET = ridual-test
  DEFINES += "TESTS_MODE"
  #DEFINES += "TEST_ENV_PATH=\\\"$$_PRO_FILE_PWD_/tests/env\\\""
} else {
  SOURCES += src/main.cpp
}

RESOURCES += \
    resources.qrc

OTHER_FILES += \
    README.md \
    doc/todo.txt \
    tests/run.sh
