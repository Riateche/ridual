#include "Directory_list_task.h"
#include <QDir>
#include <QDebug>
#include "File_system_engine.h"
#include "Core.h"
#include "qt_gtk.h"
#include "Special_uri.h"
#include "Directory.h"
#include "Mount_manager.h"
#include "Bookmarks_file_parser.h"
#include "utils.h"

Directory_list_task::Directory_list_task(QString p_uri, Core *p_core) :
  uri(p_uri)
, core(p_core)
, fs_engine(p_core->get_file_system_engine())
{
}

Directory_list_task::~Directory_list_task() {

}

void Directory_list_task::run() {
  try {
    if (Special_uri(uri).name() == Special_uri::places) {
      //the root of our virtual directory tree
      fetch_places();
      return;
    }
    File_system_engine::Iterator* iterator = fs_engine->list(uri);
    File_info_list list;
    while(iterator->has_next()) {
      list << iterator->get_next();
      //todo: send by 100-file chunks
    }
    emit ready(list);
  } catch (File_system_engine::Exception e) {
    if (e.get_cause() == File_system_engine::not_found && !uri.startsWith("/")) {
      emit location_not_found();
    } else {
      emit error(e.get_message());
    }
  }
}

void Directory_list_task::fetch_places() {
  File_info_list r;
  File_info fi;
  fi.name = tr("Filesystem root");
  fi.uri = "/";
  fi.is_folder = true;
  r << fi;
  fi = File_info();
  fi.name = tr("Home");
  fi.uri = QDir::homePath();
  fi.is_folder = true;
  r << fi;

  fi = File_info();
  fi.name = tr("Trash");
  fi.uri = "trash:///";
  fi.mime_type = "inode/directory";
  fi.is_folder = true;
  r << fi;

  QMap<QString, File_info> mounts;
  foreach (Gio_mount m, core->get_mount_manager()->get_mounts()) {
    File_info i;
    i.name = m.name;
    i.uri = m.default_location;
    i.is_folder = true;
    i.mime_type = "inode/directory";
    mounts[i.name] = i;
  }
  if (!mounts.isEmpty()) {
    File_info fi;
    fi.is_header_entry = true;
    fi.name = tr("Mounted filesystems");
    r << fi;
    r << mounts.values();
  }
  QMap<QString, File_info> volumes;
  int id = 0;
  foreach (Gio_volume* v, core->get_mount_manager()->get_volumes()) {
    //we must show only unmounted volumes because
    //mounted volumes have been listed as gio::Mount's
    if (!v->mounted) {
      File_info i;
      i.name = v->name;
      i.is_folder = true;
      i.uri = QString("places/mounts/%1").arg(id); //use number of volume in list as id
      volumes[i.name] = i;
    }
    id++;
  }
  if (!volumes.isEmpty()) {
    File_info fi;
    fi.is_header_entry = true;
    fi.name = tr("Unmounted volumes");
    r << fi;
    r << volumes.values();
  }

  QStringList uris;
  foreach(File_info fi, r) {
    fi.uri = Directory::canonize(fi.uri);
    uris << fi.uri;
  }

  QMap<QString, File_info> bookmarks;
  foreach(File_info i, core->get_bookmarks()->get_all()) {
    i.uri = Directory::canonize(i.uri);
    if (!uris.contains(i.uri)) {
      uris << i.uri;
      bookmarks [i.name] = i;
    }
  }
  if (!bookmarks.isEmpty()) {
    File_info fi;
    fi.is_header_entry = true;
    fi.name = tr("Bookmarks");
    r << fi;
    r << bookmarks.values();
  }

  QMap<QString, File_info> user_dirs;
  foreach(File_info i, core->get_user_dirs()->get_all()) {
    i.uri = Directory::canonize(i.uri);
    if (!uris.contains(i.uri)) {
      uris << i.uri;
      user_dirs[i.name] = i;
    }
  }
  if (!user_dirs.isEmpty()) {
    File_info fi;
    fi.is_header_entry = true;
    fi.name = tr("Standard places");
    r << fi;
    r << user_dirs.values();
  }
  for(int i = 0; i < r.count(); i++) {
    if (!r[i].uri.isEmpty() && r[i].mime_type.isEmpty()) {
      r[i].mime_type = get_mime_type(r[i].uri);
    }
  }

  r.columns << Column::name << Column::uri;
  r.disable_sort = true;
  emit ready(r);
}
