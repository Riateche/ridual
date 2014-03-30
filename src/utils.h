#ifndef UTILS_H
#define UTILS_H

#include <QString>
#include <QIcon>

QString get_mime_type(const QString &filename);

QIcon get_file_icon(const QString& content_type);

#endif // UTILS_H
