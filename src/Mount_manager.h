#ifndef MOUNT_MANAGER_H
#define MOUNT_MANAGER_H

#include <QObject>
#include "gio/Mount.h"
#include "gio/Volume.h"
#include <QMutex>
#include "qt_gtk.h"

typedef struct _GVolumeMonitor GVolumeMonitor;
typedef struct _GDrive GDrive;

/*! This class is responsible for fetching list of mounts and volumnes
  and updating it on changes.
  */
class Mount_manager : public QObject {
  Q_OBJECT
public:
  explicit Mount_manager(QObject *parent = 0);
  ~Mount_manager();

  inline QList<Gio_mount> get_mounts() { return mounts; }
  inline QList<Gio_volume*> get_volumes() { return volumes; }

  
signals:
  void mounts_changed();

private:
  QList<Gio_volume*> volumes;
  QList<Gio_mount> mounts;
//  QMutex mounts_mutex;
  GVolumeMonitor* volume_monitor;
  QList<gulong> connects;

  void init_connects();
  void fetch();
  static void gio_callback(GVolumeMonitor *volume_monitor, GDrive *drive, Mount_manager* _this);

};

#endif // MOUNT_MANAGER_H