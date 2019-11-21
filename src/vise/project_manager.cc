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
        boost::filesystem::path fn = d_conf.at("vise_store");
        fn = fn / pname;
        fn = fn / "conf.txt";
        file_save(request, fn, response);
        return;
      }
      if (uri[2] == "_index") {
        project_index(pname, response);
        return;
      }
      if (uri[2] == "_search") {
        response.set_status(200);
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
    file_save(request, fn, response);
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
  bool ok = vise::load_file(fn, file_content);
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

void project_manager::file_save(http_request const &request,
                                boost::filesystem::path fn,
                                http_response &response) {
  std::string file_content;
  std::cout << "Writing file " << fn.string() << std::endl;
  try {
    std::ofstream f(fn.string());
    f << request.d_payload.str();
    f.close();
    response.set_status(200);
    response.set_payload( file_content );
    response.set_content_type_from_filename( fn.string() );
  } catch(std::exception &e) {
    response.set_status(404);
    std::cout << "failed to send file in http response ["
              << fn.string() << "]" << std::endl;
  }
}

//
// Project
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

void project_manager::project_index(std::string pname,
                                    http_response &response) {
  if (project_load(pname)) {
    d_projects.at(pname)->index();
    std::ostringstream json;
    json << "{\"pname\":\"" << pname << "\","
         << "\"command\":\"_index\","
         << "\"result\":\"indexing started\"}";
    response.set_payload(json.str());
    response.set_field("Content-Type", "application/json");
    response.set_status(200);
  } else {
    response.set_status(412);
  }
  return;
}
