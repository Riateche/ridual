#-------------------------------------------------
#
# Project created by QtCreator 2012-06-16T13:54:32
#
#-------------------------------------------------

QT       += core gui
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

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


SOURCES += \
    src/gio/Gio_mounter.cpp \
    src/gui/Pane.cpp \
    src/gui/Action_state_widget.cpp \
    src/gui/Question_widget.cpp \
    src/gui/Main_window.cpp \
    src/gui/Task_details.cpp \
    src/gui/Settings_dialog.cpp \
    src/gui/Path_widget.cpp \
    src/gui/Path_button.cpp \
    src/gui/Message_widget.cpp \
    src/gui/Action_answerer.cpp \
    src/gio/Gio_file_system_engine.cpp \
    src/gio/Mount.cpp \
    src/gio/Volume.cpp \
    src/hotkeys/Hotkey_editor.cpp \
    src/hotkeys/Hotkey_item_delegate.cpp \
    src/hotkeys/Hotkey_text_edit.cpp \
    src/hotkeys/Hotkey.cpp \
    src/hotkeys/Hotkeys.cpp \
    src/Action_queue.cpp \
    src/Action.cpp \
    src/Actions_manager.cpp \
    src/Bookmarks_file_parser.cpp \
    src/Core_ally.cpp \
    src/Core.cpp \
    src/Directory_list_task.cpp \
    src/Directory_watcher.cpp \
    src/Directory.cpp \
    src/Elapsed_timer.cpp \
    src/File_info.cpp \
    src/File_leaf.cpp \
    src/File_list_model.cpp \
    src/File_system_engine.cpp \
    src/Mount_manager.cpp \
    src/Real_file_system_engine.cpp \
    src/Special_uri.cpp \
    src/utils.cpp \
    src/gui/App_info.cpp \
    src/gui/Current_queue_question.cpp \
    src/gui/Error_reaction.cpp \
    src/Columns.cpp

HEADERS  += \
    src/gio/Gio_mounter.h \
    src/gui/Task_details.h \
    src/gui/Settings_dialog.h \
    src/gui/Question_widget.h \
    src/gui/Path_widget.h \
    src/gui/Path_button.h \
    src/gui/Pane.h \
    src/gui/Message_widget.h \
    src/gui/Main_window.h \
    src/gui/Action_state_widget.h \
    src/gui/Action_answerer.h \
    src/gio/Gio_file_system_engine.h \
    src/gio/Mount.h \
    src/gio/Volume.h \
    src/hotkeys/Hotkey_editor.h \
    src/hotkeys/Hotkey_item_delegate.h \
    src/hotkeys/Hotkey_text_edit.h \
    src/hotkeys/Hotkey.h \
    src/hotkeys/Hotkeys.h \
    src/Action_queue.h \
    src/Action.h \
    src/Actions_manager.h \
    src/Bookmarks_file_parser.h \
    src/Core_ally.h \
    src/Core.h \
    src/Directory_list_task.h \
    src/Directory_watcher.h \
    src/Directory.h \
    src/Elapsed_timer.h \
    src/File_info.h \
    src/File_leaf.h \
    src/File_list_model.h \
    src/File_system_engine.h \
    src/Mount_manager.h \
    src/qt_gtk.h \
    src/Real_file_system_engine.h \
    src/Special_uri.h \
    src/types.h \
    src/utils.h \
    src/gui/App_info.h \
    src/gui/Current_queue_question.h \
    src/gui/Error_reaction.h \
    src/Columns.h

FORMS    += src/gui/Main_window.ui \
    src/gui/Pane.ui \
    src/gui/Settings_dialog.ui \
    src/gui/Task_details.ui \
    src/gui/Question_widget.ui \
    src/gui/Action_state_widget.ui \
    src/hotkeys/Hotkey_editor.ui


INCLUDEPATH += src src/gui

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
