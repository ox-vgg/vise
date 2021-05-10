#include "metadata.h"

vise::metadata::metadata(std::string pname, boost::filesystem::path project_data_dir)
  : d_pname(pname),
    d_project_data_dir(project_data_dir),
    d_sqlite_db_status(SQLITE_ERROR),
    d_file_metadata_table_name("file_metadata"),
    d_region_metadata_table_name("region_metadata"),
    d_metadata_fts_vtable_name("metadata_vtable"),
    d_concatenated_file_metadata_col_name("concatenated_metadata"),
    d_is_metadata_available(false)
{
  std::cout << "initializing metadata for " << d_pname << std::endl;
  std::cout << "project_data_dir=" << d_project_data_dir.string() << std::endl;

  d_metadata_db_fn   = d_project_data_dir / "metadata_db.sqlite";
  d_metadata_conf_fn = d_project_data_dir / "metadata_conf.json";

  // initialize sqlite3 database
  d_sqlite_db_status = sqlite3_open_v2(d_metadata_db_fn.string().c_str(),
                                       &d_db,
                                       SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE,
                                       NULL);
  if( d_sqlite_db_status == SQLITE_OK ) {
    uint32_t tstart = vise::getmillisecs();
    init_metadata_db();
    init_metadata_fts_vtable();
    init_metadata_conf();

    // now open database in read only mode (to avoid misuse)
    sqlite3_close(d_db);
    d_sqlite_db_status = sqlite3_open_v2(d_metadata_db_fn.string().c_str(),
                                         &d_db,
                                         SQLITE_OPEN_READONLY,
                                         NULL);

    uint32_t tend = vise::getmillisecs();
    d_is_metadata_available = true;
    std::cout << "vise::metadata : init completed in " << (tend - tstart) << " ms"
              << std::endl;
  } else {
    d_metadata_conf = "{}";
    std::cout << "vise::metadata: failed to open database file "
              << d_metadata_db_fn.string() << std::endl;
  }
}

vise::metadata::~metadata() {
  if(d_sqlite_db_status == SQLITE_OK) {
    std::cout << "~metadata(): closing sqlite database" << std::endl;
    sqlite3_close(d_db);
  }
}

void vise::metadata::init_metadata_db() {
  create_metadata_db();
  init_file_attribute_name_list();
  if(sqlite_table_exists(d_region_metadata_table_name)) {
    init_region_attribute_name_list();
  }
}

bool vise::metadata::is_region_metadata_available() const {
  if(!d_is_metadata_available) {
    return false;
  }
  if(d_sqlite_db_status != SQLITE_OK) {
    return false;
  }
  if(!sqlite_table_exists(d_region_metadata_table_name)) {
    return false;
  }
  return true;
}

void vise::metadata::init_region_attribute_name_list() {
  d_region_attribute_name_list.clear();

  int rc;
  sqlite3_stmt *stmt;
  const char *tail;
  std::string sql = "PRAGMA table_info(" + d_region_metadata_table_name + ");";
  rc = sqlite3_prepare_v2(d_db, sql.c_str(), -1, &stmt, &tail);
  if(rc != SQLITE_OK) {
    std::cout << "vise::metadata : "
              << sqlite3_errmsg(d_db)
              << std::endl;
  }
  rc = sqlite3_step(stmt);
  int ncols = sqlite3_column_count(stmt);

  while(rc == SQLITE_ROW) {
    std::ostringstream column_name;
    column_name << sqlite3_column_text(stmt, 1);
    d_region_attribute_name_list.push_back(column_name.str());
    rc = sqlite3_step(stmt);
  }
  sqlite3_finalize(stmt);
  std::cout << "vise::metadata : read " << d_region_attribute_name_list.size()
            << " region attributes" << std::endl;
}

void vise::metadata::init_file_attribute_name_list() {
  d_file_attribute_name_list.clear();

  int rc;
  sqlite3_stmt *stmt;
  const char *tail;
  std::string sql = "PRAGMA table_info(" + d_file_metadata_table_name + ");";
  rc = sqlite3_prepare_v2(d_db, sql.c_str(), -1, &stmt, &tail);
  if(rc != SQLITE_OK) {
    std::cout << "vise::metadata : "
              << sqlite3_errmsg(d_db)
              << std::endl;
  }
  rc = sqlite3_step(stmt);
  int ncols = sqlite3_column_count(stmt);

  while(rc == SQLITE_ROW) {
    std::ostringstream column_name;
    column_name << sqlite3_column_text(stmt, 1);
    d_file_attribute_name_list.push_back(column_name.str());
    rc = sqlite3_step(stmt);
  }
  sqlite3_finalize(stmt);
  std::cout << "vise::metadata : read " << d_file_attribute_name_list.size()
            << " file attributes" << std::endl;
}

void vise::metadata::create_metadata_db() {
  if(sqlite_table_exists(d_file_metadata_table_name)) {
    std::cout << "vise::metadata : loading file_metadata from "
              << d_metadata_db_fn.string() << std::endl;
    return ;
  }
  std::cout << "vise::metadata : creating file_metadata table ..." << std::endl;
  int rc;
  char *err_msg;
  std::string sql;

  sql = "BEGIN TRANSACTION";
  rc = sqlite3_exec(d_db, sql.c_str(), NULL, NULL, &err_msg);

  sql = "CREATE TABLE `" + d_file_metadata_table_name + "`(`file_id` INTEGER PRIMARY KEY, `filename` TEXT NOT NULL);";
  rc = sqlite3_exec(d_db, sql.c_str(), NULL, NULL, &err_msg);

  boost::filesystem::path filelist_fn = d_project_data_dir / "filelist.txt";
  std::ifstream filelist(filelist_fn.string(), std::ios::in);
  if(!filelist.is_open()) {
    std::cout << "failed to open [" << filelist_fn.string() << "] for reading file list"
              << std::endl;
  }
  uint32_t fid = 0;
  std::string filename;

  // ASSUMPTION: the line number of "filelist.txt" indicates the file-id
  // @todo: ensure that this assumption is always valid
  // (i.e. even when some files are discarded during feature extraction)
  std::ostringstream ss;
  while( !filelist.eof() ) {
    std::getline(filelist, filename);
    if(filename == "") {
      continue;
    }
    // sqlite requires text to be escaped such that all single quotes (') are replaced with ('')
    vise::escape_string(filename, '\'', "''");
    ss << "INSERT INTO `" << d_file_metadata_table_name << "` "
       << "VALUES(" << fid << ",'" << filename << "');";
    sql = ss.str();
    rc = sqlite3_exec(d_db, sql.c_str(), NULL, NULL, &err_msg);
    fid = fid + 1;
    ss.str("");
    ss.clear();
  }

  sql = "END TRANSACTION";
  rc = sqlite3_exec(d_db, sql.c_str(), NULL, NULL, &err_msg);

  if(rc != SQLITE_OK) {
    if(err_msg != NULL) {
      std::cout << "metadata::create_metadata_db() failed" << std::endl;
      sqlite3_free(err_msg);
    }
  }
}

//
// file metadata full text search
//
void vise::metadata::init_metadata_fts_vtable() {
  if(sqlite_table_exists(d_metadata_fts_vtable_name)) {
    std::cout << "vise::metadata : " << d_metadata_fts_vtable_name
              << " already exists" << std::endl;
    return ;
  }

  // create virtual table for full text search
  int rc;
  char *err_msg;
  std::string sql;

  // gather column names from file_metadata table

  // create temporary virtual table containing FTS3 index
  sql = "BEGIN TRANSACTION";
  rc = sqlite3_exec(d_db, sql.c_str(), NULL, NULL, &err_msg);


  sql = "CREATE VIRTUAL TABLE `" + d_metadata_fts_vtable_name + "` USING FTS3(`rowid` INTEGER PRIMARY KEY, `" + d_concatenated_file_metadata_col_name + "` TEXT);";
  rc = sqlite3_exec(d_db, sql.c_str(), NULL, NULL, &err_msg);

  // sanity check
  if(d_file_attribute_name_list.size() < 2) {
    std::cout << "vise::metadata : init_metadata_fts_vtable() failed; "
              << "d_file_attribute_name_list=" << d_file_attribute_name_list.size()
              << " (should be >= 2)" << std::endl;
    return ;
  }

  // for each row, concatenate all the columns and put in the virtual table to allow full text search
  std::ostringstream ss;

  // insert region metadata
  // region_metadata(row_id, file_id, region_id, ...)
  if(d_region_attribute_name_list.size() > 3) {
    // insert both file metadata and region metadata
    // insert file metadata
    ss << "INSERT INTO `" << d_metadata_fts_vtable_name << "` "
       << "SELECT FM.file_id, IFNULL(FM." << d_file_attribute_name_list.at(1) << " || ' ', '')"; // (file_id, filename, ...)
    for(std::size_t i=2; i<d_file_attribute_name_list.size(); ++i) {
      ss << " || IFNULL(FM." << d_file_attribute_name_list.at(i) << " || ' ', '')";
    }
    ss << " || IFNULL(RM." << d_region_attribute_name_list.at(3) << " || ' ', '')";
    for(std::size_t i=4; i<d_region_attribute_name_list.size(); ++i) {
      ss << " || IFNULL(`" << d_region_attribute_name_list.at(i) << "` || ' ', '')";
    }
    ss << " AS `" << d_concatenated_file_metadata_col_name
       << "` FROM `" << d_file_metadata_table_name << "` AS FM"
       << " LEFT JOIN `" << d_region_metadata_table_name << "` AS RM"
       << " USING(file_id);";
    sql = ss.str();
  } else {
    // insert file metadata only
    ss << "INSERT INTO `" << d_metadata_fts_vtable_name << "` "
       << "SELECT file_id, IFNULL(" << d_file_attribute_name_list.at(1) << " || ' ', '')"; // (file_id, filename, ...)
    for(std::size_t i=2; i<d_file_attribute_name_list.size(); ++i) {
      ss << " || IFNULL(" << d_file_attribute_name_list.at(i) << " || ' ', '')";
    }
    ss << " AS `" << d_concatenated_file_metadata_col_name
       << "` FROM `" << d_file_metadata_table_name << "`;";
    sql = ss.str();
  }
  rc = sqlite3_exec(d_db, sql.c_str(), NULL, NULL, &err_msg);

  ss.clear();
  ss.str("");
  ss << "INSERT INTO " << d_metadata_fts_vtable_name << "("
     << d_metadata_fts_vtable_name << ") VALUES('optimize');";
  sql = ss.str();
  rc = sqlite3_exec(d_db, sql.c_str(), NULL, NULL, &err_msg);

  sql = "END TRANSACTION";
  rc = sqlite3_exec(d_db, sql.c_str(), NULL, NULL, &err_msg);

  if(rc != SQLITE_OK) {
    if(err_msg != NULL) {
      std::cout << "vise::metadata : init_metadata_fts_vtable() failed : "
                << err_msg << std::endl;
      sqlite3_free(err_msg);
    }
  } else {
    std::cout << "vise::metadata : init_metadata_fts_vtable() done " << std::endl;
  }
}

// query: cat AND dog
// query: name=ABC
// query: year=(1400,1500)
bool vise::metadata::full_text_search_query_to_sql(const std::string query,
                                                   std::string &sql) const {

  std::ostringstream ss;
  std::size_t eq_pos = query.find("=");
  if(eq_pos == (query.size() - 1)) {
    return false;
  }

  if(eq_pos == std::string::npos) {
    ss << "SELECT DISTINCT rowid AS file_id from `" << d_metadata_fts_vtable_name << "` WHERE "
       << "`" << d_metadata_fts_vtable_name << "` MATCH '"
       << query << "'";
    sql.assign(ss.str());
    return true;
  }

  std::string col_name( query.substr(0, eq_pos) );
  std::string table_name = table_name_from_attribute_name(col_name);

  if(table_name == "") {
    return false; // non-existing column
  }

  if(query.at(eq_pos + 1) == '(') {
    // range search, e.g. year=(1400,1480)
    std::size_t bracket_pos = query.find(')', eq_pos + 1);
    if(bracket_pos == std::string::npos) {
      return false;
    }
    if(bracket_pos == (eq_pos + 2)) {
      return false; // discard year=()
    }
    std::size_t comma_pos = query.find(',', eq_pos + 2);
    if(comma_pos == std::string::npos) {
      return false;
    }
    if((bracket_pos - comma_pos) < 2) {
      return false;
    }
    std::string value1_str( query.substr(eq_pos+2, comma_pos - eq_pos - 2) );
    std::string value2_str( query.substr(comma_pos + 1, bracket_pos - comma_pos - 1) );
    ss << "SELECT DISTINCT file_id from `" << table_name << "` WHERE `"
       << col_name << "` BETWEEN '" << value1_str << "' AND '" << value2_str << "'";
    sql.assign(ss.str());
    return true;
  } else {
    // exact search
    std::string value_str( query.substr(eq_pos + 1) );
    ss << "SELECT DISTINCT file_id from `" << table_name << "` WHERE `"
       << col_name << "` = '" << value_str << "'";
    sql.assign(ss.str());
  }

  // default is full text search
  ss << "SELECT DISTINCT rowid AS file_id from `" << d_metadata_fts_vtable_name << "` WHERE "
     << "`" << d_metadata_fts_vtable_name << "` MATCH '"
      << query << "'";
  return true;
}
void vise::metadata::file_metadata_full_text_search(const std::string query,
                                                    std::vector<uint32_t> &flist) {
  uint32_t tstart = vise::getmillisecs();
  std::string sql;
  bool success = full_text_search_query_to_sql(query, sql);
  if(!success) {
    return;
  }

  int rc;
  sqlite3_stmt *stmt;
  const char *tail;
  rc = sqlite3_prepare_v2(d_db, sql.c_str(), -1, &stmt, &tail);
  if(rc != SQLITE_OK) {
    std::cout << "metadata::filter_on_filename() failed : "
              << sqlite3_errmsg(d_db)
              << std::endl;
  }
  rc = sqlite3_step(stmt);
  while(rc == SQLITE_ROW) {
    flist.push_back(sqlite3_column_int(stmt, 0)); // we know that there will be only one column (i.e. file_id)
    rc = sqlite3_step(stmt);
  }
  sqlite3_finalize(stmt);
  uint32_t tend = vise::getmillisecs();
  //std::cout << "vise::metadata : query=" << query << ", result size=" << flist.size()
  //          << ", completed in " << (tend - tstart) << " ms" << std::endl;
}

void vise::metadata::file_metadata_full_text_search_group_stat(const std::string query,
                                                               const std::string groupby,
                                                               std::map<std::string, uint32_t> &group_stat) const {
  uint32_t tstart = vise::getmillisecs();
  std::string table_name = table_name_from_attribute_name(groupby);

  std::string query_sql;
  bool success = full_text_search_query_to_sql(query, query_sql);
  if(!success) {
    return;
  }

  group_stat.clear();
  std::ostringstream ss;
  ss << "SELECT " << groupby << ", COUNT(file_id) FROM `" << table_name << "`"
     << " WHERE " << groupby << " IS NOT NULL AND file_id IN "
     << " (" << query_sql << ") "
     << "GROUP BY (" << groupby << ");";
  std::string sql(ss.str());
  int rc;
  sqlite3_stmt *stmt;
  const char *tail;
  rc = sqlite3_prepare_v2(d_db, sql.c_str(), -1, &stmt, &tail);
  if(rc != SQLITE_OK) {
    std::cout << "metadata::file_metadata_full_text_search_group_stat() failed : "
              << sqlite3_errmsg(d_db)
              << std::endl;
  }
  rc = sqlite3_step(stmt);
  if(rc == SQLITE_ROW) {
    std::stringstream ss;
    std::string group;
    uint32_t group_size;
    while(rc == SQLITE_ROW) {
      ss << sqlite3_column_text(stmt, 0);
      group.assign(ss.str());
      group_size = sqlite3_column_int(stmt, 1);
      group_stat[group] = group_size;
      ss.clear();
      ss.str("");
      rc = sqlite3_step(stmt);
    }
  }
  sqlite3_finalize(stmt);
  uint32_t tend = vise::getmillisecs();
  std::cout << "vise::metadata : query=" << query << ", groupby=" << groupby
            << ", group_stat size=" << group_stat.size()
            << ", completed in " << (tend - tstart) << " ms" << std::endl;
}

void vise::metadata::select_fid_with_filename_like(const std::string filename_pattern,
                                                   std::vector<std::size_t> &file_id_list) const {
  file_id_list.clear();
  std::ostringstream ss;
  ss << "SELECT `file_id` from `" << d_file_metadata_table_name << "`"
     << " WHERE `filename` LIKE '" << filename_pattern << "';";
  std::string sql(ss.str());
  int rc;
  sqlite3_stmt *stmt;
  const char *tail;
  rc = sqlite3_prepare_v2(d_db, sql.c_str(), -1, &stmt, &tail);
  if(rc != SQLITE_OK) {
    std::cout << "vise::metadata : filename LIKE `" << filename_pattern << "` failed, "
              << sqlite3_errmsg(d_db)
              << std::endl;
    return;
  }
  rc = sqlite3_step(stmt);
  int ncols = sqlite3_column_count(stmt);
  if(rc == SQLITE_ROW && ncols > 0) {
    while(rc == SQLITE_ROW) {
      file_id_list.push_back(sqlite3_column_int(stmt, 0));
      rc = sqlite3_step(stmt);
    }
  }
  sqlite3_finalize(stmt);
}

void vise::metadata::select_rid_with_filename_like(const std::string filename_pattern,
                                                   std::vector<std::size_t> &region_id_list) const {
  region_id_list.clear();
  std::ostringstream ss;
  ss << "SELECT `region_id` from `" << d_region_metadata_table_name << "`"
     << " WHERE `file_id` IN "
     << "(SELECT `file_id` from `" << d_file_metadata_table_name << "` "
     << "WHERE `filename` LIKE '" << filename_pattern << "');";
  std::string sql(ss.str());
  int rc;
  sqlite3_stmt *stmt;
  const char *tail;
  rc = sqlite3_prepare_v2(d_db, sql.c_str(), -1, &stmt, &tail);
  if(rc != SQLITE_OK) {
    std::cout << "vise::metadata : file_id IN filename LIKE `" << filename_pattern << "` failed, "
              << sqlite3_errmsg(d_db)
              << std::endl;
    return;
  }
  rc = sqlite3_step(stmt);
  int ncols = sqlite3_column_count(stmt);
  if(rc == SQLITE_ROW && ncols > 0) {
    while(rc == SQLITE_ROW) {
      region_id_list.push_back(sqlite3_column_int(stmt, 0));
      rc = sqlite3_step(stmt);
    }
  }
  sqlite3_finalize(stmt);
}

void vise::metadata::get_region_file_info(const uint32_t region_id,
                                          std::size_t &file_id,
                                          std::size_t &region_index) const {
  std::ostringstream ss;
  ss << "SELECT file_id,region_index "
     << "FROM `" << d_region_metadata_table_name << "` "
     << "WHERE `region_id` = " << region_id << " LIMIT 1;";
  std::string sql(ss.str());
  int rc;
  sqlite3_stmt *stmt;
  const char *tail;
  rc = sqlite3_prepare_v2(d_db, sql.c_str(), -1, &stmt, &tail);
  if(rc != SQLITE_OK) {
    std::cout << "vise::metadata : failed to fetch region_id = " << region_id << ", "
              << sqlite3_errmsg(d_db)
              << std::endl;
    return;
  }
  rc = sqlite3_step(stmt);
  int ncols = sqlite3_column_count(stmt);
  if(rc == SQLITE_ROW && ncols == 2) {
    file_id = sqlite3_column_int(stmt, 0);
    region_index = sqlite3_column_int(stmt, 1);
  }
  sqlite3_finalize(stmt);
}

void vise::metadata::get_region_shape(const uint32_t region_id,
                                      std::size_t &file_id,
                                      std::size_t &region_index,
                                      std::string &region_shape,
                                      std::string &region_points) const {
  std::ostringstream ss;
  ss << "SELECT file_id, region_index, region_shape, region_points "
     << "FROM `" << d_region_metadata_table_name << "` "
     << "WHERE `region_id` = " << region_id << " LIMIT 1;";
  std::string sql(ss.str());
  int rc;
  sqlite3_stmt *stmt;
  const char *tail;
  rc = sqlite3_prepare_v2(d_db, sql.c_str(), -1, &stmt, &tail);
  if(rc != SQLITE_OK) {
    std::cout << "vise::metadata : failed to fetch region_id = " << region_id << ", "
              << sqlite3_errmsg(d_db)
              << std::endl;
    return;
  }
  rc = sqlite3_step(stmt);
  int ncols = sqlite3_column_count(stmt);
  if(rc == SQLITE_ROW && ncols == 4) {
    file_id = sqlite3_column_int(stmt, 0);
    region_index = sqlite3_column_int(stmt, 1);
    std::ostringstream ss;
    ss << sqlite3_column_text(stmt, 2);
    region_shape = ss.str();
    ss.str("");
    ss << sqlite3_column_text(stmt, 3);
    region_points = ss.str();
  }
  sqlite3_finalize(stmt);
}

//
// util
//
bool vise::metadata::sqlite_table_exists(const std::string table_name) const {
  std::string sql("SELECT COUNT(type) from sqlite_master where type='table' and name='" + table_name + "';");
  int rc;
  sqlite3_stmt *stmt;
  const char *tail;
  rc = sqlite3_prepare_v2(d_db, sql.c_str(), -1, &stmt, &tail);
  if(rc != SQLITE_OK) {
    std::cout << "metadata::sqlite_table_exists() failed" << std::endl;
    return false;
  }
  rc = sqlite3_step(stmt);
  int ncols = sqlite3_column_count(stmt);
  if(ncols != 1) {
    std::cout << "metadata::sqlite_table_exists() malformed result from SQL" << std::endl;
    return false;
  }

  unsigned int table_count = sqlite3_column_int(stmt, 0);
  sqlite3_finalize(stmt);
  if(table_count == 1) {
    return true;
  } else {
    return false;
  }
}

void vise::metadata::file_metadata_as_json(const uint32_t file_id,
                                           std::ostringstream &json) const {
  std::ostringstream ss;
  ss << "SELECT * from `" << d_file_metadata_table_name
     << "` WHERE `file_id` = " << file_id << " LIMIT 1;";
  std::string sql(ss.str());
  int rc;
  sqlite3_stmt *stmt;
  const char *tail;
  rc = sqlite3_prepare_v2(d_db, sql.c_str(), -1, &stmt, &tail);
  if(rc != SQLITE_OK) {
    json << "{}";
    return;
  }
  rc = sqlite3_step(stmt);
  int ncols = sqlite3_column_count(stmt);
  json << "{";
  if(rc == SQLITE_ROW && ncols > 0) {
    std::ostringstream col_value_stream;
    col_value_stream << sqlite3_column_text(stmt, 0);
    std::string col_value(col_value_stream.str());
    vise::escape_string(col_value, '"', "\\\"");
    json << "\"" << sqlite3_column_name(stmt, 0)
         << "\":\"" << col_value << "\"";
    for(std::size_t i=1; i<ncols; ++i) {
      col_value_stream.clear();
      col_value_stream.str("");
      col_value_stream << sqlite3_column_text(stmt, i);
      col_value.assign(col_value_stream.str());
      vise::escape_string(col_value, '"', "\\\"");
      json << ",\"" << sqlite3_column_name(stmt, i)
           << "\":\"" << col_value << "\"";
    }
  }
  json << "}";
  sqlite3_finalize(stmt);
}

void vise::metadata::region_metadata_as_json(const uint32_t file_id,
                                             std::ostringstream &json) const {
  std::ostringstream ss;
  ss << "SELECT * from `" << d_region_metadata_table_name
     << "` WHERE `file_id` = " << file_id << ";";
  std::string sql(ss.str());
  int rc;
  sqlite3_stmt *stmt;
  const char *tail;
  rc = sqlite3_prepare_v2(d_db, sql.c_str(), -1, &stmt, &tail);
  if(rc != SQLITE_OK) {
    json << "[]";
    return;
  }
  rc = sqlite3_step(stmt);
  int ncols = sqlite3_column_count(stmt);
  bool first_row = true;
  json << "[";
  while(rc == SQLITE_ROW) {
    if(!first_row) {
      json << ",";
    } else {
      first_row = false;
    }
    json << "{";
    for(std::size_t i=0; i<ncols; ++i) {
      if(i!=0) {
        json << ",";
      }

      ss.clear();
      ss.str("");
      ss << sqlite3_column_name(stmt, i);
      std::string col_name(ss.str());
      json << "\"" << col_name << "\":";

      ss.clear();
      ss.str("");
      ss << sqlite3_column_text(stmt,i);
      std::string col_value(ss.str());
      vise::escape_string(col_value, '"', "\\\"");
      if(col_name == "region_points") {
        json << "[" << col_value << "]";
      } else {
        json << "\"" << col_value << "\"";
      }
    }
    json << "}";
    rc = sqlite3_step(stmt);
  }
  json << "]";
  sqlite3_finalize(stmt);
}

void vise::metadata::metadata_conf_as_json(std::ostringstream &json) const {
  json << d_metadata_conf;
}

void vise::metadata::file_attribute_name_list(std::vector<std::string> &file_attribute_name_list) const {
  file_attribute_name_list = d_file_attribute_name_list;
}

void vise::metadata::init_metadata_conf() {
  std::ifstream f(d_metadata_conf_fn.string());
  if(f.is_open()) {
    d_metadata_conf.assign( (std::istreambuf_iterator<char>(f)),
                            (std::istreambuf_iterator<char>()) );
  } else {
    d_metadata_conf = "{}";
    std::cout << "vise::metadata : failed to load metadata_conf" << std::endl;
  }
  f.close();
}

std::string vise::metadata::table_name_from_attribute_name(const std::string aname) const {
  std::vector<std::string>::const_iterator itr;
  itr = std::find(d_file_attribute_name_list.begin(),
                  d_file_attribute_name_list.end(),
                  aname);
  if(itr != d_file_attribute_name_list.end()) {
    return d_file_metadata_table_name;
  } else {
    itr = std::find(d_region_attribute_name_list.begin(),
                    d_file_attribute_name_list.end(),
                    aname);
    if(itr != d_region_attribute_name_list.end()) {
      return d_region_metadata_table_name;
    } else {
      return "";
    }
  }
}

void vise::metadata::metadata_group_stat(const std::string groupby,
                                         std::map<std::string, uint32_t> &group_stat) const {
  group_stat.clear();
  std::string table_name = table_name_from_attribute_name(groupby);

  std::ostringstream ss;
  ss << "SELECT `" << groupby << "`, COUNT(file_id) from `" << table_name
     << "` WHERE `" << groupby << "` IS NOT NULL GROUP BY `" << groupby << "` LIMIT 512;"; // @todo max number of groups = 512
  std::string sql(ss.str());
  int rc;
  sqlite3_stmt *stmt;
  const char *tail;
  rc = sqlite3_prepare_v2(d_db, sql.c_str(), -1, &stmt, &tail);
  if(rc != SQLITE_OK) {
    std::cout << "vise::metadata : group_stat() failed, "
              << sqlite3_errmsg(d_db)
              << std::endl;
    return;
  }
  rc = sqlite3_step(stmt);
  int ncols = sqlite3_column_count(stmt);
  if(rc == SQLITE_ROW && ncols > 0) {
    std::stringstream ss;
    std::string group;
    uint32_t group_size;
    while(rc == SQLITE_ROW) {
      ss << sqlite3_column_text(stmt, 0);
      group.assign(ss.str());
      group_size = sqlite3_column_int(stmt, 1);
      group_stat[group] = group_size;
      ss.clear();
      ss.str("");
      rc = sqlite3_step(stmt);
    }
  }
  sqlite3_finalize(stmt);
}

void vise::metadata::metadata_groupby(const std::string groupby,
                                      const std::string group,
                                      std::vector<uint32_t> &flist) const {
  flist.clear();
  std::string table_name = table_name_from_attribute_name(groupby);

  std::ostringstream ss;
  ss << "SELECT `file_id` from `" << table_name
     << "` WHERE `" << groupby << "`='" << group << "';";
  std::string sql(ss.str());
  int rc;
  sqlite3_stmt *stmt;
  const char *tail;
  rc = sqlite3_prepare_v2(d_db, sql.c_str(), -1, &stmt, &tail);
  if(rc != SQLITE_OK) {
    std::cout << "vise::metadata : groupby() failed, "
              << sqlite3_errmsg(d_db)
              << std::endl;
    return;
  }
  rc = sqlite3_step(stmt);
  int ncols = sqlite3_column_count(stmt);
  if(rc == SQLITE_ROW && ncols > 0) {
    while(rc == SQLITE_ROW) {
      flist.push_back(sqlite3_column_int(stmt, 0));
      rc = sqlite3_step(stmt);
    }
  }
  sqlite3_finalize(stmt);
}

void vise::metadata::get_copy_of_metadata(const std::string source_table_name,
                                          const std::string destination_db_filename,
                                          const std::string destination_table_name,
                                          bool &success,
                                          std::string &message) const {
  int rc;
  char *err_msg;
  std::string sql;
  std::ostringstream ss;
  ss << "BEGIN TRANSACTION;"
     << "ATTACH DATABASE '" << destination_db_filename << "' AS DEST;"
     << "CREATE TABLE 'DEST." << destination_table_name << "' AS "
     << "SELECT * FROM " << source_table_name << ";"
     << "END TRANSACTION;";
  sql = ss.str();
  rc = sqlite3_exec(d_db, sql.c_str(), NULL, NULL, &err_msg);
  if(rc != SQLITE_OK) {
    message = "failed to execute get_copy_of_metadata: source_table=" + source_table_name + ", destination_db=" + destination_db_filename + ", destination_table=" + destination_table_name + ".";
    std::cout << "sqlite3 error:: " << err_msg << std::endl;
    success = false;
    if(err_msg != NULL) {
      sqlite3_free(err_msg);
    }
  } else {
    success = true;
  }
}
