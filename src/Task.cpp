#include "Task.h"
/*
Task::Task(QObject *p_receiver,
           const char *p_slot,
           Task_type p_type,
           QVariant arg1,
           QVariant arg2,
           QVariant arg3)
{
  receiver = p_receiver;
  slot = p_slot;
  type = p_type;
  arguments << arg1 << arg2 << arg3;
}*/

Task::Task(QObject *parent): QObject(parent) {
}
