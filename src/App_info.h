#ifndef APP_INFO_H
#define APP_INFO_H

#include "qt_gtk.h"

#include <QList>
#include <QVariant>


/*! This class extracts information about an application from GAppInfo object
  and provides a way to launch it.
  */
class App_info {
public:
  App_info(GAppInfo* obj = 0);
  QString name();
  QString command() const;
  bool operator==(const App_info& other);

  void launch(QStringList filenames);
  void launch(QString filename);

  void launch_uris(QStringList filenames); //not recommended

private:
  GAppInfo* object;
};

Q_DECLARE_METATYPE(App_info)

class App_info_list: public QList<App_info> {
public:
  App_info default_app;
};


#endif // APP_INFO_H
