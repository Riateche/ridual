#ifndef MESSAGE_WIDGET_H
#define MESSAGE_WIDGET_H

#include <QFrame>
#include "types.h"

class Message_widget : public QFrame {
  Q_OBJECT
public:


  explicit Message_widget(QString message, Icon::Enum icon);
  
signals:
  
public slots:
  
};

#endif // MESSAGE_WIDGET_H
