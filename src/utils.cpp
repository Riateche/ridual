#include "utils.h"
#include "qt_gtk.h"
#include <QDebug>
#include <QCache>

QIcon get_file_icon(const QString& content_type) {
  static QCache<QString, QIcon> cache;
  cache.setMaxCost(100);
  if (cache.contains(content_type)) return QIcon(*(cache.object(content_type)));
  QByteArray ba = content_type.toLocal8Bit();
  GIcon* gicon = g_content_type_get_icon(ba.constData());
  if (G_IS_THEMED_ICON(gicon)) {
    const gchar * const * names = g_themed_icon_get_names(reinterpret_cast<GThemedIcon*>(gicon));
    if (names != 0) {
      int i = 0;
      while(names[i] != 0) {
        QString name = QString::fromLocal8Bit(names[i]);
        if (name != "(null)") { // hack for ubuntu 12.10
          QIcon r = QIcon::fromTheme(name);
          if (r.isNull()) {
            qDebug() << "get_file_icon: no icon from theme for name: " << name;
          } else {
            cache.insert(content_type, new QIcon(r));
            g_object_unref(gicon);
            return r;
          }
        }
        i++;
      }
    } else {
      qDebug() << "get_file_icon: empty or invalid result of g_themed_icon_get_names";
    }
  } else {
    qDebug() << "get_file_icon: gicon is not themed icon";
  }
  qDebug() << "get_file_icon: returning null icon";
  g_object_unref(gicon);
  return QIcon();
}
