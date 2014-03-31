#include "App_info.h"
#include <QDebug>
#include <QStringList>
#include "Main_window.h"



App_info_data::App_info_data(Main_window* mw, GAppInfo* obj): main_window(mw), object(obj) {

}

App_info_data::~App_info_data() {
  g_object_unref(object);
}

QString App_info_data::name() {
  if (!object) {
    qWarning("unitialized App_info used");
    return QString();
  }
  const char* b = g_app_info_get_name(object);
  return QString::fromLocal8Bit(b);
}

QString App_info_data::command() const {
  if (!object) {
    qWarning("unitialized App_info used");
    return QString();
  }
  const char* b = g_app_info_get_commandline(object);
  return QString::fromLocal8Bit(b);
}

/*bool App_info::operator ==(const App_info &other) {
  if (other.object == object) return true;
  if (other.command() == command()) return true;
  return false;
}*/

void App_info_data::launch(QString filename) {
  launch( QStringList() << filename );
}

void App_info_data::launch_uris(QStringList filenames) {
  GList* list = 0;
  GError* error = 0;
  foreach(QString filename, filenames) {
    list = g_list_append(list, filename.toLocal8Bit().data());
  }
  g_app_info_launch_uris(object, list, 0, &error);
  g_list_free(list);
  if (error) {
    main_window->show_error(QObject::tr("Failed to launch an application: %1").arg(error->message));
    g_error_free(error);
  }
}

void App_info_data::launch(QStringList filenames) {
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
    main_window->show_error(QObject::tr("Failed to launch an application: %1").arg(error->message));
  }
}
