#ifndef GIO_H
#define GIO_H

#include <QThread>
#include "Volume.h"
#include "Mount.h"

typedef struct _GVolumeMonitor GVolumeMonitor;

class Gio_main : public QThread {
  Q_OBJECT  
public:
  explicit Gio_main(QObject *parent = 0);
  void fetch_all();

signals:
  void list_changed(QList<gio::Volume> volumes, QList<gio::Mount> mounts);


private:
  GVolumeMonitor *monitor;
  virtual void run();

  QList<gio::Volume> volumes;
  QList<gio::Mount> mounts;

};

#endif // GIO_H
