#ifndef GIO_MOUNTER_H
#define GIO_MOUNTER_H

#include <QObject>

typedef struct _GObject GObject;
typedef struct _GAsyncResult GAsyncResult;
typedef struct _GCancellable GCancellable;
typedef struct _GFile GFile;
typedef void* gpointer;


class Gio_mounter : public QObject {
  Q_OBJECT
public:
  explicit Gio_mounter();
  void interrupt();
  void start(const QString& uri);

signals:
  void finished(bool success);
  void error(QString message);

private:
  GCancellable* gcancellable;
  GFile* file;
  static void async_result(GObject *source_object, GAsyncResult *res, gpointer p_this);
  ~Gio_mounter();

  
};

#endif // GIO_MOUNTER_H
