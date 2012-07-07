#include "utils.h"
/*
#include <QDebug>
#include <magic.h>

QString get_mime_type(const QString &filename) {
  QString result("application/octet-stream");
  magic_t magicMimePredictor;
  magicMimePredictor = magic_open(MAGIC_MIME_TYPE); // Open predictor
  if (!magicMimePredictor) {
    qDebug() << "libmagic: Unable to initialize magic library";
  } else if (magic_load(magicMimePredictor, 0)) { // if not 0 - error
    qDebug() << "libmagic: Can't load magic database - " +
                QString(magic_error(magicMimePredictor));
    magic_close(magicMimePredictor); // Close predictor
  } else {
    char *file = filename.toLocal8Bit().data();
    const char *mime;
    mime = magic_file(magicMimePredictor, file); // getting mime-type
    result = QString(mime);
    magic_close(magicMimePredictor); // Close predictor
  }

  //qDebug() << "get_mime_type(" << filename << ") = " << result;
  return result;
}
*/
