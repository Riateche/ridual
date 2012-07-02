#include "App_info.h"
#include <QDebug>

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
  qDebug() << "launch app" << name() << "with file" << filename;
  GList* list = 0;
  GFile* file = g_file_new_for_path(filename.toLocal8Bit());
  GError* error = 0;
  list = g_list_append(list, file);
  g_app_info_launch(object, list, 0, &error);
  g_object_unref(file);
  g_list_free(list);
  if (error) {
    qDebug() << "error: " << error->message;
    //todo: display message to user
  }
}
