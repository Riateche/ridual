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
  delete fs_engine;
  //todo: fix segfault here
}

void Directory_list_task::run() {
  fs_engine->moveToThread(thread()); //dangerous! need to test
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
