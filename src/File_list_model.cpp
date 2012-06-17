#include "File_list_model.h"
#include <QIcon>


File_list_model::File_list_model() {
}

void File_list_model::set_data(QList<File_info> p_list) {
  emit layoutAboutToBeChanged();
  list = p_list;
  emit layoutChanged();

}

int File_list_model::rowCount(const QModelIndex &parent) const {
  return list.count();
  Q_UNUSED(parent);
}

QVariant File_list_model::data(const QModelIndex &index, int role) const {
  if (index.row() < 0 || index.row() >= list.count()) return QVariant();
  if (role == Qt::DisplayRole) {
    return list.at(index.row()).i.fileName();
  }
  if (role == Qt::DecorationRole) {
    /*QImage image(5, 5, QImage::Format_ARGB32);
    image.fill(Qt::green);
    image.setPixel(2, 2, Qt::white);
    return QPixmap::fromImage(image); */
    return icon_provider.icon(list.at(index.row()).i);
  }
  return QVariant();
}

File_info File_list_model::info(const QModelIndex &index) {
  if (index.row() < 0 || index.row() >= list.count()) return File_info();
  return list[index.row()];
}
