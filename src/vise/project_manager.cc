#include "project_manager.h"

using namespace vise;

// the HTML page header of every HTML response
std::string vise::project_manager::PROJECT_HTML_PAGE_PREFIX = R"HTML(<!DOCTYPE html><html lang="en"><head><meta charset="UTF-8"><title>Project | VISE</title><link rel="shortcut icon" type="image/x-icon" href="favicon.ico"/><link rel="stylesheet" type="text/css" href="project.css" /></head>)HTML";

std::string vise::project_manager::PROJECT_HTML_PAGE_SUFFIX = R"HTML(\n</html>)HTML";

project_manager::project_manager(std::map<std::string, std::string> const &conf)
  : d_conf(conf)
{
  std::cout << "project_manager: showing configuration"
            << std::endl;
  vise::configuration_show(d_conf);
  d_projects.clear();
}

// WARNING: this method may be invoked by multiple threads simultaneously
// be careful of avoid race conditions
void project_manager::process_http_request(http_request const &request,
                                           http_response &response)
{
  std::ostringstream ss;
  ss << "project_manager: "
     << request.d_method << " " << request.d_uri;
  std::cout << ss.str() << std::endl;

  std::vector<std::string> uri;
  std::map<std::string, std::string> param;
  vise::decompose_uri(request.d_uri, uri, param);
  vise::print_vector("uri", uri);
  vise::print_map("param", param);

  if (request.d_method == "GET") {
    handle_get(request, uri, param, response);
    return;
  }
  if (request.d_method == "POST") {
    handle_post(request, uri, param, response);
    return;
  }
  if (request.d_method == "PUT") {
    handle_put(request, uri, param, response);
    return;
  }
  if (request.d_method == "DELETE") {
    handle_delete(request, uri, param, response);
    return;
  }

  // return default status
  response.set_status(404);
  return;
}

void project_manager::handle_get(http_request const &request,
                                 std::vector<std::string> const &uri,
                                 std::map<std::string, std::string> const &param,
                                 http_response &response)
{
  if ( uri.size() == 1 ) {
    response.set_status(404);
    return;
  }
  if (uri[0] != "") {
    response.set_status(404);
    return;
  }

  if (uri.size() == 2) {
    if (uri[1].front() == '_') {
      // a command (e.g. _create, _delete
      // @todo
      response.set_status(200);
      return;
    } else {
      if (uri[1] == "") {
        response.redirect_to(request.d_uri + "index.html");
        return;
      } else {
        // request for user interface HTML application files
        boost::filesystem::path file_loc(d_conf.at("www_root"));
        file_loc = file_loc / uri[1];
        file_send(file_loc, response);
        return;
      }
    }
  }

  if (uri.size() > 2) {
    // request for a project resource
    std::string pname = uri[1];
    if (!project_exists(pname)) {
      response.set_status(404);
      return;
    }

    if (uri[2].front() == '_') {
      // project command (e.g. _conf,)
      if (uri[2] == "_conf") {
        boost::filesystem::path fn = d_conf.at("vise_store");
        fn = fn / pname;
        fn = fn / "conf.txt";
        file_send(fn, response);
        return;
      }
    }

    if (uri.size() >= 3) {
      // serve static resources of a project (e.g. images)
      std::string asset = uri[2];
      if (uri.size() != 3) {
        std::size_t start = request.d_uri.find(pname);
        start = start + pname.size();
        asset = request.d_uri.substr(start);
      }
      boost::filesystem::path fn = d_conf.at("project_store");
      fn = fn / pname;
      fn = fn / asset;
      file_send(fn, response);
      return;
    }
  }

  // default response for unhandled cases
  response.set_status(400);
  return;
}

void project_manager::handle_post(http_request const &request,
                                  std::vector<std::string> const &uri,
                                  std::map<std::string, std::string> const &param,
                                  http_response &response)
{
  if ( uri.size() == 1 ) {
    response.set_status(404);
    return;
  }
  if (uri[0] != "") {
    response.set_status(404);
    return;
  }

  if (uri.size() == 2) {
    response.set_status(404);
    return;
  }

  if (uri.size() > 2) {
    std::string pname(uri[1]);
    if (uri[2].front() == '_') {
      // process command for a project
      if (uri[2] == "_conf") {
        std::string proj_conf_str = request.d_payload.str();
        project_conf_set(pname, proj_conf_str, response);
        return;
      }
      if (uri[2] == "_index_create") {
        project_index_create(pname, response);
        return;
      }
      if (uri[2] == "_index_load") {
        project_index_load(pname, response);
        return;
      }
      if (uri[2] == "_index_unload") {
        project_index_unload(pname, response);
        return;
      }
      if (uri[2] == "_index_search") {
        project_index_search(pname, param, response);
        return;
      }
      if (uri[2] == "_file_add") {
        std::cout << "adding file " << pname << std::endl;
        project_file_add(pname, param, response);
        return;
      }
    }
  }
  response.set_status(404);
}

void project_manager::handle_put(http_request const &request,
                                 std::vector<std::string> const &uri,
                                 std::map<std::string, std::string> const &param,
                                 http_response &response)
{
  if ( uri.size() == 1 ) {
    response.set_status(404);
    return;
  }
  if (uri[0] != "") {
    response.set_status(404);
    return;
  }

  if (uri.size() == 2) {
    // create a new project
    try {
      std::ostringstream json;
      std::string pname(uri[1]);
      if (project_exists(pname)) {
        json << "{\"pname\":\"" << pname << "\","
             << "\"uri\":\"/" << pname << "/\","
             << "\"message\":\"Project already exists\""
             << "}";
        response.set_payload(json.str());
        response.set_status(412);
        return;
      }
      project_create(pname);
      json << "{\"pname\":\"" << pname << "\","
           << "\"uri\":\"/" << pname << "/\"}";
      response.set_payload(json.str());
      response.set_field("Content-Type", "application/json");
      response.set_status(200);
    } catch(std::exception &e) {
      response.set_payload(e.what());
      response.set_field("Content-Type", "text/plain");
      response.set_status(400);
    }
    return;
  }

  if (uri.size() > 2) {
    std::string pname(uri[1]);
    if (uri[2] == "") {
      response.set_status(400);
      return;
    }

    std::string asset = uri[2];
    if (uri.size() != 3) {
      std::size_t start = request.d_uri.find(pname);
      start = start + pname.size();
      asset = request.d_uri.substr(start);
    }
    boost::filesystem::path fn = d_conf.at("project_store");
    fn = fn / pname;
    fn = fn / asset;
    std::string payload = request.d_payload.str();
    bool ok = vise::file_save(fn, payload);
    if(ok) {
      response.set_status(200);
      return;
    } else {
      response.set_status(412);
      return;
    }
    return;
  }

  // default response
  response.set_status(404);
  return;
}

void project_manager::handle_delete(http_request const &request,
                                    std::vector<std::string> const &uri,
                                    std::map<std::string, std::string> const &param,
                                    http_response &response)
{
  if ( uri.size() == 1 ) {
    response.set_status(404);
    return;
  }
  if (uri[0] != "") {
    response.set_status(404);
    return;
  }

  if (uri.size() == 2) {
    // delete a project
    try {
      std::string pname(uri[1]);
      if (project_exists(pname)) {
        project_delete(pname);
        response.set_status(200);
      } else {
        response.set_status(412);
      }
    } catch(std::exception &e) {
      response.set_status(400);
    }
    return;
  }

  if (uri.size() > 2) {
    // delete a project's resource (e.g. image)
    std::string pname(uri[1]);
    if (uri[2] == "") {
      response.set_status(400);
      return;
    }

    std::string asset = uri[2];
    if (uri.size() != 3) {
      std::size_t start = request.d_uri.find(pname);
      start = start + pname.size();
      asset = request.d_uri.substr(start);
    }
    boost::filesystem::path fn = d_conf.at("project_store");
    fn = fn / pname;
    fn = fn / asset;
    if (boost::filesystem::remove(fn)) {
      response.set_status(200);
    } else {
      response.set_status(412);
    }
    return;
  }

}

void project_manager::file_send(boost::filesystem::path fn,
                                http_response &response) {
  std::string file_content;
  std::cout << "Sending file " << fn.string() << std::endl;
  bool ok = vise::file_load(fn, file_content);
  if ( ok ) {
    response.set_status(200);
    response.set_payload( file_content );
    response.set_content_type_from_filename( fn.string() );
  } else {
    response.set_status(404);
    std::cout << "failed to send file in http response ["
              << fn.string() << "]" << std::endl;
  }
}

//
// POST /{PNAME}/_index_*
//

bool project_manager::project_exists(std::string pname) {
  boost::filesystem::path pdatadir = d_conf.at("project_store");
  pdatadir = pdatadir / pname;
  boost::filesystem::path vdatadir = d_conf.at("vise_store");
  vdatadir = vdatadir / pname;

  if (boost::filesystem::exists(pdatadir) &&
      boost::filesystem::exists(vdatadir)) {
    return true;
  } else {
    return false;
  }
}

void project_manager::project_create(std::string pname) {
  boost::filesystem::path pdatadir = d_conf.at("project_store");
  pdatadir = pdatadir / pname;
  boost::filesystem::create_directory(pdatadir);

  boost::filesystem::path vdatadir = d_conf.at("vise_store");
  vdatadir = vdatadir / pname;
  boost::filesystem::create_directory(vdatadir);
}

bool project_manager::project_load(std::string pname) {
  std::lock_guard<std::mutex> lock(d_project_load_mutex);
  if (!project_exists(pname)) {
    return false;
  }
  if (d_projects.count(pname) == 1) {
    return true;
  }
  try {
    d_projects[pname] = std::unique_ptr<vise::project>(new vise::project(pname, d_conf));
    return true;
  } catch(std::exception &e) {
    return false;
  }
}

void project_manager::project_delete(std::string pname) {
  boost::filesystem::path pdatadir = d_conf.at("project_store");
  pdatadir = pdatadir / pname;
  boost::filesystem::remove_all(pdatadir);

  boost::filesystem::path vdatadir = d_conf.at("vise_store");
  vdatadir = vdatadir / pname;
  boost::filesystem::remove_all(vdatadir);
}

void project_manager::project_index_create(std::string pname,
                                           http_response &response) {
  if (project_load(pname)) {
    bool success;
    std::string message;
    d_projects.at(pname)->index_create(success, message);
    std::ostringstream json;
    json << "{\"pname\":\"" << pname << "\","
         << "\"command\":\"_index_create\","
         << "\"result\":\"" << message << "\"}";
    response.set_payload(json.str());
    response.set_field("Content-Type", "application/json");
    response.set_status(200);
  } else {
    response.set_status(412);
  }
  return;
}

void project_manager::project_index_load(std::string pname,
                                         http_response &response) {
  if (project_load(pname)) {
    bool success;
    std::string message;
    d_projects.at(pname)->index_load(success, message);
    std::ostringstream json;
    json << "{\"pname\":\"" << pname << "\","
         << "\"command\":\"_index_load\","
         << "\"result\":\"" << message << "\"}";
    response.set_payload(json.str());
    response.set_field("Content-Type", "application/json");
    response.set_status(200);
  } else {
    response.set_status(412);
  }
  return;
}

void project_manager::project_index_unload(std::string pname,
                                           http_response &response) {
  if (project_load(pname)) {
    bool success;
    std::string message;
    d_projects.at(pname)->index_unload(success, message);
    std::ostringstream json;
    json << "{\"pname\":\"" << pname << "\","
         << "\"command\":\"_index_unload\","
         << "\"result\":\"" << message << "\"}";
    response.set_payload(json.str());
    response.set_field("Content-Type", "application/json");
    response.set_status(200);
  } else {
    response.set_status(412);
  }
  return;
}

void project_manager::project_index_search(std::string pname,
                                           std::map<std::string, std::string> const &param,
                                           http_response &response) {
  if( !project_load(pname) ) {
    response.set_status(412);
    return;
  }

  if( !d_projects.at(pname)->index_is_loaded() ) {
    response.set_status(412);
    return;
  }

  if (param.count("file_id") == 0 ||
      param.count("x") == 0 ||
      param.count("y") == 0 ||
      param.count("width") == 0 ||
      param.count("height") == 0) {
    response.set_status(412);
    return;
  }

  vise::search_query query(param);
  std::vector<vise::search_result> result;
  d_projects.at(pname)->index_search(query, result);

  std::ostringstream json;
  json << "{\"query\":{\"file_id\":" << query.d_file_id << ","
       << "\"x\":" << query.d_x << ",\"y\":" << query.d_y
       << ",\"width\":" << query.d_width << ",\"height\":" << query.d_height
       << "},\"result\":[";

  std::vector<vise::search_result>::iterator it;
  for (it = result.begin(); it != result.end(); ++it) {
    json << "{\"file_id\":" << it->d_file_id << ","
         << "\"filename\":\"" << it->d_filename << "\","
         << "\"score\":" << it->d_score << ","
         << "\"H\":[" << it->d_H[0] << "," << it->d_H[1] << "," << it->d_H[2]
         << "," << it->d_H[3] << "," << it->d_H[4] << "," << it->d_H[5]
         << "," << it->d_H[6] << "," << it->d_H[7] << "," << it->d_H[8]
         << "]}";
    if ((it+1) != result.end()) {
      json << ",";
    }
  }
  json << "]}";
  response.set_payload(json.str());
  response.set_field("Content-Type", "application/json");
  response.set_status(200);
}

bool project_manager::project_index_is_loaded(std::string pname) {
  if( d_projects.count(pname)==0) {
    return false;
  }

  if( d_projects.at(pname)->index_is_loaded() ) {
    return true;
  } else {
    return false;
  }
}

bool project_manager::project_index_is_done(std::string pname) {
  if( d_projects.count(pname)==0) {
    return false;
  }

  if(d_projects.at(pname)->index_is_done() ) {
    return true;
  } else {
    return false;
  }
}

//
// POST /{PNAME}/_file_*
//
void project_manager::project_file_add(std::string pname,
                                       std::map<std::string, std::string> const &param,
                                       http_response &response) {
  std::cout << param.at("source_type") << std::endl;
  std::cout << param.at("source_loc") << std::endl;
  if(param.count("source_type") == 0 ||
     param.count("source_loc") == 0 ) {
    response.set_status(412);
    return;
  }

  if(param.at("source_type") == "local_folder") {
    boost::filesystem::path src(param.at("source_loc"));
    if(!boost::filesystem::exists(src) ||
       !boost::filesystem::is_directory(src)) {
      response.set_status(412);
      return;
    }

    boost::filesystem::path dst_prefix = d_conf.at("project_store");
    dst_prefix = dst_prefix / pname;
    boost::filesystem::directory_iterator end_itr;
    std::size_t file_count = 0;
    for(boost::filesystem::directory_iterator it(src); it!=end_itr; ++it) {
      if(boost::filesystem::is_regular_file(it->path())) {
        if(it->path().extension() == ".jpg" ||
           it->path().extension() == ".JPG" ||
           it->path().extension() == ".jpeg" ||
           it->path().extension() == ".JPEG" ||
           it->path().extension() == ".png" ||
           it->path().extension() == ".PNG"
           ) {
          boost::filesystem::path dst(dst_prefix);
          dst = dst / it->path().filename();
          boost::filesystem::copy_file(it->path(), dst);
          file_count += 1;
        }
      }
    }
    std::ostringstream json;
    json << "{\"pname\":\"" << pname << "\","
         << "\"command\":\"_file_add\","
         << "\"source_type\":\"" << param.at("source_type") << "\","
         << "\"source_loc\":\"" << param.at("source_loc") << "\","
         << "\"result\":\"" << file_count << "\"}";
    response.set_payload(json.str());
    response.set_field("Content-Type", "application/json");
    response.set_status(200);
    return;
  } else {
    response.set_status(412);
    return;
  }
}

//
// POST /{PNAME}/_conf
//
void project_manager::project_conf_set(std::string pname,
                                       std::string &conf_str,
                                       http_response &response) {
  boost::filesystem::path fn = d_conf.at("vise_store");
  fn = fn / pname;
  fn = fn / "conf.txt";

  bool ok = vise::file_save(fn, conf_str);
  if(ok) {
    response.set_status(200);
    return;
  } else {
    response.set_status(412);
    return;
  }
}

//
// DEBUG
//
void project_manager::debug() {
  std::string pname("ox5k");
  project_load(pname);

  bool success;
  std::string message;
  d_projects.at(pname)->index_load(success, message);

  std::map<std::string, std::string> param;
  param["file_id"] = "12";
  param["x"] = "110";
  param["y"] = "410";
  param["width"] = "400";
  param["height"] = "260";

  vise::search_query query(param);
  std::vector<vise::search_result> result;
  d_projects.at(pname)->index_search(query, result);

  std::ostringstream json;
  json << "{\"query\":{\"file_id\":" << query.d_file_id << ","
       << "\"x\":" << query.d_x << ",\"y\":" << query.d_y
       << ",\"width\":" << query.d_width << ",\"height\":" << query.d_height
       << "},\"result\":[";

  std::vector<vise::search_result>::iterator it;
  for (it = result.begin(); it != result.end(); ++it) {
    json << "{\"file_id\":" << it->d_file_id << ","
         << "\"filename\":\"" << it->d_filename << "\","
         << "\"score\":" << it->d_score << ","
         << "\"H\":[" << it->d_H[0] << "," << it->d_H[1] << "," << it->d_H[2]
         << "," << it->d_H[3] << "," << it->d_H[4] << "," << it->d_H[5]
         << "," << it->d_H[6] << "," << it->d_H[7] << "," << it->d_H[8]
         << "]}";
    if ((it+1) != result.end()) {
      json << ",";
    }
  }
  json << "]}";
  std::cout << "Result:" << std::endl << json.str() << std::endl;
}
