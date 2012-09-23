#include "File_list_proxy_model.h"
#include <QSettings>
#include "File_list_model.h"
#include <QDebug>

File_list_proxy_model::File_list_proxy_model()
{
}

bool File_list_proxy_model::lessThan(const QModelIndex &left, const QModelIndex &right) const {
  static File_list_model* source_model = 0;
  if (source_model == 0) source_model = dynamic_cast<File_list_model*>(sourceModel());
  QSettings s;
  bool show_folders_before_files = s.value("show_folders_before_files").toBool();
  if (show_folders_before_files) {
    bool f1 = source_model->get_file_info(left).is_folder;
    bool f2 = source_model->get_file_info(right).is_folder;
    if (!f1 && f2) {
      return sortOrder() == Qt::DescendingOrder;
    } else if (f1 && !f2) {
      return sortOrder() == Qt::AscendingOrder;
    }
  }
  return QSortFilterProxyModel::lessThan(left, right);
}
