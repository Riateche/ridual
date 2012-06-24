#ifndef TASK_H
#define TASK_H

#include <QVariant>

enum Task_type {
  task_directory_list
};

class Task {
public:
  Task(QObject* receiver,
       const char* slot,
       Task_type type,
       QVariant arg1 = QVariant(),
       QVariant arg2 = QVariant(),
       QVariant arg3 = QVariant());
  Task_type type;
  QVariantList arguments;
  QObject* receiver;
  const char* slot;
};

class Task_error {
public:
  Task_error(QString p_message = QString()): message(p_message) {}
  QString message;
};

Q_DECLARE_METATYPE(Task_error)

#endif // TASK_H
