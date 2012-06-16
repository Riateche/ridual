#include "Pane.h"
#include "ui_Pane.h"

Pane::Pane(QWidget *parent) :
  QWidget(parent),
  ui(new Ui::Pane)
{
  ui->setupUi(this);
}

Pane::~Pane()
{
  delete ui;
}
