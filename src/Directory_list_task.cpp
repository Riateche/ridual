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
  //fs_engine->deleteLater();
}

void Directory_list_task::run() {
  try {
    File_system_engine::Iterator* iterator = fs_engine->list(uri);
    File_info_list list;
    while(iterator->has_next()) {
      list << iterator->get_next();
      //todo: send by 100-file chunks
    }
    emit ready(list);
  } catch (File_system_engine::Exception e) {
    if (e.get_cause() == File_system_engine::not_found && !uri.startsWith("/")) {
      emit location_not_found();
    } else {
      emit error(e.get_message());
    }
  }
}
