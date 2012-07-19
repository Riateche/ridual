#include "gtest/gtest.h"
#include <QApplication>
#include <QTimer>

class Gtest_runner : public QObject {
  Q_OBJECT
public:
  Gtest_runner() {}

public slots:
  void run() {
    QApplication::exit(RUN_ALL_TESTS());
  }

};

#include "main.moc"


int main(int argc, char** argv) {
  ::testing::InitGoogleTest(&argc, argv);
  QApplication a(argc, argv);
  Gtest_runner r;
  QTimer::singleShot(0, &r, SLOT(run()));
  return a.exec();
}
