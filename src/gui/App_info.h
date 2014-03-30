#ifndef APP_INFO_H
#define APP_INFO_H

#include "qt_gtk.h"

#include <QList>
#include <QVariant>

class Main_window;

/*! This class extracts information about an application from GAppInfo object
  and provides a way to launch it.

  Objects are passed by value. Used Main_window object must not be deleted during
  life time of App_info objects.

  todo: GAppInfo shoule be deleted somewhere; check copying

  */
class App_info {
public:
  App_info(Main_window* mw = 0, GAppInfo* obj = 0);
  QString name();
  QString command() const;
  bool operator==(const App_info& other);

  void launch(QStringList filenames);
  void launch(QString filename);

  void launch_uris(QStringList filenames); //not recommended

private:
  Main_window* main_window;
  GAppInfo* object;
};

Q_DECLARE_METATYPE(App_info)

class App_info_list: public QList<App_info> {
public:
  App_info default_app;
};


#endif // APP_INFO_H
