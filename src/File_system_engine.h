#ifndef FILE_OPERATIONS_H
#define FILE_OPERATIONS_H

#include "File_info.h"

#define FS_ENGINE_BUFFER_SIZE 65535

class File_system_engine {
public:

  class Iterator {
  public:
    virtual bool has_next() = 0;
    virtual File_info get_next() = 0;
    virtual ~Iterator() {}

  protected:
    Iterator() {}

  };

  class Operation {
  public:
    virtual bool is_finished() = 0;
    virtual double get_progress() = 0;
    virtual void run_iteration() = 0;
    virtual ~Operation() {}

  protected:
    Operation() {}
  };



  enum error_type {
    unknown_error,
    directory_list_failed,
    source_file_read_failed,
    destination_file_write_failed,
    file_remove_failed
  };

  enum error_cause {
    unknown_cause,
    not_found,
    permission_denied,
    already_exists,
    too_many_entries,
    filesystem_full,
    readonly_filesystem,
    busy,
    invalid_path,
    not_directory,
    path_too_long,
    symbolic_links_loop,
    io_error
  };

  class Exception {
  public:
    Exception(error_type type, error_cause cause);
    inline error_type get_type() { return type; }
    inline error_cause get_cause() { return cause; }

  private:
    error_type type;
    error_cause cause;

  };


  File_system_engine();
  virtual bool is_responsible_for(const QString& uri) = 0;
  virtual Iterator* list(const QString& uri) = 0;
  virtual Operation* copy(const QString& source, const QString& destination) = 0;
  virtual Operation* move(const QString& source, const QString& destination) = 0;
  virtual void remove(const QString& uri) = 0;
  virtual void make_directory(const QString& uri) = 0;
  virtual QString get_real_file_name(const QString& uri) = 0;
  virtual ~File_system_engine() {}


protected:
  static error_cause get_cause_from_errno();


};

#endif // FILE_OPERATIONS_H
