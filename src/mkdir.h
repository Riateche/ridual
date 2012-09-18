#ifndef MKDIR_H
#define MKDIR_H

#include <QString>

bool ridual_mkdir(QString path, QString& error_string);
bool ridual_rmdir(QString path, QString& error_string);

void errno_to_string(QString& error_string);


#endif // MKDIR_H
