#include "Directory_list_task.h"
#include <QDir>
#include <QDebug>
#include "File_system_engine.h"


#include "qt_gtk.h"


Directory_list_task::Directory_list_task(QString p_uri, File_system_engine *e) :
  uri(p_uri)
, fs_engine(e)
{
}

Directory_list_task::~Directory_list_task() {
  fs_engine->deleteLater();
  //todo: fix segfault here; fixed?
}

void Directory_list_task::run() {
  /*
    moveToThead() should be called from the thread that owns fs_engine (GUI thread actually),
    but in GUI thread we don't know yet
    which thread should own fs_engine object. We would create
    fs_engine right here, but Core::get_new_file_system_engine method is too
    far from being thread-safe. Now we will hope that the next statement will
    not crash the app. //todo: do something
  */
  fs_engine->moveToThread(thread());
  try {
    File_system_engine::Iterator* iterator = fs_engine->list(uri);
    File_info_list list;
    while(iterator->has_next()) {
      list << iterator->get_next();
      //todo: send by 100-file chunks
    }
    emit ready(list);
  } catch (File_system_engine::Exception e) {
    emit error(e.get_message());
  }

}
