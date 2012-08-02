#ifndef QUEUE_DETAILS_H
#define QUEUE_DETAILS_H

#include <QWidget>

namespace Ui {
  class Task_details;
}

class Task_details : public QWidget
{
  Q_OBJECT
  
public:
  explicit Task_details(QWidget *parent = 0);
  ~Task_details();
  
private:
  Ui::Task_details *ui;
};

#endif // QUEUE_DETAILS_H
