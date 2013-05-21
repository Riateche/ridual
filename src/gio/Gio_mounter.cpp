#include "Gio_mounter.h"
#include "qt_gtk.h"
#include <QDebug>


Gio_mounter::Gio_mounter() {

}

Gio_mounter::~Gio_mounter() {
  //interrupt();
}

void Gio_mounter::interrupt() {
  if (gcancellable) {
    g_cancellable_cancel(gcancellable);
    gcancellable = 0;
  }
}

void Gio_mounter::start(const QString &uri) {
  qDebug() << "Trying to mount location" << uri;
  file = g_file_new_for_uri(uri.toLocal8Bit());
  gcancellable = g_cancellable_new();
  g_file_mount_enclosing_volume(file, GMountMountFlags(), 0, gcancellable, async_result, this);
}


void Gio_mounter::async_result(GObject *source_object, GAsyncResult *res, gpointer p_this) {
  Gio_mounter* _this = reinterpret_cast<Gio_mounter*>(p_this);
  GError* e = 0;
  g_file_mount_enclosing_volume_finish(reinterpret_cast<GFile*>(source_object), res, &e);
  if (e) {
    if (e->code == G_IO_ERROR_CANCELLED) {
      qDebug() << "Operation was cancelled";
    } else if (e->code == G_IO_ERROR_NOT_SUPPORTED) {
      //error "volume doesn't implement mount"  occurs on invalid address
      emit _this->error(tr("Address not recognized"));
    } else {
      emit _this->error( tr("Error %1: %2").arg(e->code)
                         .arg(QString::fromLocal8Bit(e->message)) );
    }
    g_error_free(e);
    emit _this->finished(false);
    return;
  } else {
    emit _this->finished(true);
  }
  g_object_unref(source_object); // source_object == file
  _this->deleteLater();
}
