#include "Task_details.h"
#include "ui_Task_details.h"

Task_details::Task_details(QWidget *parent) :
  QWidget(parent),
  ui(new Ui::Task_details)
{
  ui->setupUi(this);
}

Task_details::~Task_details() {
  delete ui;
}
