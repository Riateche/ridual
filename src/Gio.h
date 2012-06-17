#ifndef GIO_H
#define GIO_H

#include <QThread>

#include <QtCore/QCoreApplication>
#undef signals // Collides with GTK symbols
#include "gio/gio.h"
#include "gtk/gtk.h"

class Gio : public QThread {
  Q_OBJECT
public:
  explicit Gio(QObject *parent = 0);



private:
  GVolumeMonitor *monitor;
  virtual void run();
  
};

#endif // GIO_H
