#include "App_info.h"
#include <QDebug>
#include <QStringList>

App_info::App_info(GAppInfo* obj): object(obj) {

}

QString App_info::name() {
  if (!object) {
    qWarning("unitialized App_info used");
    return QString();
  }
  const char* b = g_app_info_get_name(object);
  return QString::fromLocal8Bit(b);
}

QString App_info::command() const {
  if (!object) {
    qWarning("unitialized App_info used");
    return QString();
  }
  const char* b = g_app_info_get_commandline(object);
  return QString::fromLocal8Bit(b);
}

bool App_info::operator ==(const App_info &other) {
  if (other.object == object) return true;
  if (other.command() == command()) return true;
  return false;
}

void App_info::launch(QString filename) {
  launch( QStringList() << filename );
}

void App_info::launch(QStringList filenames) {
  GList* list = 0;
  GError* error = 0;
  foreach(QString filename, filenames) {
    GFile* file = g_file_new_for_path(filename.toLocal8Bit());
    list = g_list_append(list, file);
  }
  g_app_info_launch(object, list, 0, &error);
  for(; list; list = list->next) {
    g_object_unref(static_cast<GFile*>(list->data));
  }
  g_list_free(list);
  if (error) {
    qDebug() << "error: " << error->message;
    g_error_free(error);
    //todo: display message to user
  }
}
