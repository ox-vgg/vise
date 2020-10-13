#include "project_manager.h"

using namespace vise;

project_manager::project_manager(std::map<std::string, std::string> const &conf)
  : d_conf(conf), is_serve_only_active(false)
{
  std::cout << "project_manager: showing configuration"
            << std::endl;
  vise::configuration_show(d_conf);
  d_projects.clear();
}

project_manager::~project_manager() {
  std::cout << "~project_manager()" << std::endl;
}

// WARNING: this method may be invoked by multiple threads simultaneously
// be careful of avoid race conditions
void project_manager::process_http_request(http_request const &request,
                                           http_response &response)
{
  //std::ostringstream ss;
  //ss << "project_manager: " << request.d_method << " " << request.d_uri;

  // if http_uri_namespace is defined,
  // we only respond to queries under that namespace
  std::string request_uri_without_ns(request.d_uri);
  if(d_conf.count("http_uri_namespace")) {
    if(d_conf.at("http_uri_namespace") != "") {
      std::string ns = d_conf.at("http_uri_namespace");
      if(request.d_uri.size() < ns.size()) {
        uri_namespace_mismatch_4xx_response(response);
        return;
      }
      if(request.d_uri.substr(0,ns.size()) != ns) {
        uri_namespace_mismatch_4xx_response(response);
        return;
      }
      request_uri_without_ns = request.d_uri.substr(ns.size()-1); // retain the trailing slash
    }
  }

  std::vector<std::string> uri;
  std::map<std::string, std::string> param;
  vise::decompose_uri(request_uri_without_ns, uri, param);
  //ss << " [without-ns=" << request_uri_without_ns << "]";
  //std::cout << ss.str() << std::endl;

  //request.parse_urlencoded_form_data();
  //vise::print_vector("uri", uri);
  //vise::print_map("param", param);
  //vise::print_map("d_fields", request.d_fields);
  //vise::print_map("d_multipart_data", request.d_multipart_data);
  //std::cout << "payload=" << request.d_payload.str() << std::endl;

  if(is_serve_only_active) {
    // only respond to GET requests for a limited number of projects
    if (request.d_method != "GET") {
      serve_only_4xx_response(response);
      return;
    }

    if (request.d_method == "GET") {
      if (uri.size() == 1) {
        serve_only_4xx_response(response);
        return;
      }
      if (uri.size() == 2) {
        if( uri[1] == "home" ||
            uri[1] == "settings" ||
            uri[1] == "about" ||
            uri[1] == "index.html" ||
            uri[1] == "") {
          serve_only_4xx_response(response);
        } else {
          serve_from_www_store(uri[1], response);
        }
        return;
      }

      if(uri.size() > 2) {
        // request for a project resource
        std::string pname = uri[1];
        if( !project_is_loaded(pname) ) {
          response.set_status(404);
          return;
        }
        handle_project_get_request(pname, request, uri, param, response);
        return;
      }
    }
    response.set_status(404);
    return;
  }

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
  if(uri[1] == "" || uri[1] == "index.html") {
    // redirect localhost:port/ to localhost:port/_ui/index.html
    response.redirect_to(d_conf.at("http_uri_namespace") + "home");
    return;
  }

  if (uri.size() == 2) {
    if (uri[1] == "home") {
      vise_home(param, response);
      return;
    }
    if (uri[1] == "settings") {
      vise_settings(param, response);
      return;
    }

    if (uri[1] == "about") {
      vise_about(param, response);
      return;
    }

    // all other requests are treated as request for static resources from www_store
    serve_from_www_store(uri[1], response);
    return;
  }

  if (uri.size() > 2) {
    // request for a project resource
    std::string pname = uri[1];
    if (!project_exists(pname)) {
      std::ostringstream ss;
      ss << "<h1>Error</h1><p>Project [" << pname << "] does not exist! Visit <a href=\"/home\">Home</a> page to view a list of available projects.</p>";
      response.set_html_payload(ss.str());
      return;
    }

    if (uri[2].front() == '_') {
      // project command that do not trigger loading of a project
      if (uri[2] == "_cover_image") {
        boost::filesystem::path image_dir(d_conf.at("vise_store"));
        image_dir = image_dir / pname;
        image_dir = image_dir / "image";
        if(!boost::filesystem::exists(image_dir)) {
          response.set_status(404);
          return;
        }

        boost::filesystem::directory_iterator end_itr;
        std::vector<std::string> img_fn_list;
        for (boost::filesystem::directory_iterator it(image_dir); it!=end_itr; ++it) {
          if (boost::filesystem::is_regular_file(it->path())) {
            img_fn_list.push_back(it->path().string());
            if(img_fn_list.size() > 3) {
              break;
            }
          }
        }
        if(img_fn_list.size()) {
          file_send(img_fn_list.at(img_fn_list.size() - 1), response);
          return;
        } else {
          response.set_status(404);
          return;
        }
      }
    }

    // all projects must be loaded before a get request
    // can be fulfilled
    if(!project_is_loaded(pname)) {
      std::cout << "GET request triggered load of project=" << pname << std::endl;
      if(!project_load(pname)) {
        std::string response_format = "html";
        if(param.count("response_format") == 1) {
          if(param.at("response_format") == "json") {
            response_format = "json";
          }
        }
        std::string message = "project [" + pname + "] could not be loaded";
        vise_error_page(message, response_format, response);
        return;
      }
    }

    // serve project resource
    handle_project_get_request(pname, request, uri, param, response);
    return;
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
    if (uri[1].front() == '_') {
      if(uri[1] == "_project_create") {
        std::map<std::string, std::string> formdata;
        vise::parse_urlencoded_form(request.d_payload.str(), formdata);
        vise_project_create(formdata, response);
        return;
      }
      if(uri[1] == "_project_delete") {
        std::map<std::string, std::string> formdata;
        vise::parse_urlencoded_form(request.d_payload.str(), formdata);
        vise_project_delete(formdata, response);
        return;
      }
      if (uri[1] == "_project_unload") {
          std::map<std::string, std::string> formdata;
          vise::parse_urlencoded_form(request.d_payload.str(), formdata);
          vise_project_unload(formdata, response);
          return;
      }
      if(uri[1] == "_settings_update") {
        std::string settings_formdata = request.d_payload.str();
        vise_settings_save(settings_formdata, response);
        return;
      }
    }
    response.set_status(404);
    return;
  }

  if (uri.size() > 2) {
    std::string pname(uri[1]);
    if (uri[2].front() == '_') {
      // process command for a project
      if (uri[2] == "_config_save") {
        std::string config_formdata = request.d_payload.str();
        project_config_save(pname, config_formdata, response);
        return;
      }
      if (uri[2] == "_config_use_preset") {
        std::string preset_id = request.d_payload.str();
        project_config_use_preset(pname, preset_id, response);
        return;
      }
      if (uri[2] == "_index_create") {
        bool block_until_done = false;
        project_index_create(pname, response, block_until_done);
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
      if (uri[2] == "_file_add") {
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
  if (uri[0] != "") {
    response.set_status(404);
    return;
  }

  if ( uri.size() == 1 || uri.size() == 2 ) {
    response.set_status(404);
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

    boost::filesystem::path fn(d_projects.at(pname)->pconf("image_src_dir"));
    fn = fn / asset;
    std::string payload = request.d_payload.str();
    bool ok = vise::file_save_binary(fn, payload);
    if(ok) {
      response.set_status(200);
      response.set_payload("");
      response.set_field("Content-Type", "text/plain");
      return;
    } else {
      response.set_status(412);
      response.set_payload("");
      response.set_field("Content-Type", "text/plain");
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
    boost::filesystem::path fn = d_conf.at("vise_store");
    fn = fn / pname;
    fn = fn / "image";
    fn = fn / asset;
    if (boost::filesystem::remove(fn)) {
      response.set_status(200);
    } else {
      response.set_status(412);
    }
    return;
  }

}

void project_manager::serve_from_www_store(std::string res_uri,
                                           http_response &response) const {
  boost::filesystem::path file_loc(d_conf.at("www_store"));
  file_loc = file_loc / res_uri;
  file_send(file_loc, response);
}

void project_manager::file_send(boost::filesystem::path fn,
                                http_response &response) const {
  std::string file_content;
  //std::cout << "Sending file " << fn.string() << std::endl;
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

void project_manager::handle_project_get_request(std::string const pname,
                                                 http_request const &request,
                                                 std::vector<std::string> const &uri,
                                                 std::map<std::string, std::string> const &param,
                                                 http_response& response) const {
  if(uri[2].size() == 0) {
    // GET /PNAME/ redirects to project home
    project_home(pname, response);
    return;
  }

  if(uri[2] == "filelist") {
    if(d_projects.at(pname)->index_is_loaded()) {
      project_filelist(pname, param, response);
    } else {
      if(d_projects.at(pname)->index_load_is_ongoing()) {
        vise_wait_page("Project load operation is ongoing. This page will automatically refresh and redirect to project home page after completion of the load operation.", "html", response);
      } else {
        vise_error_page("Project has not been loaded yet!", "html", response);
      }
    }
    return;
  }

  if(uri[2] == "file") {
    project_file(pname, param, response);
    return;
  }

  if(uri[2] == "search") {
    project_index_search(pname, param, response);
    return;
  }

  if(uri[2] == "showmatch") {
    project_show_match(pname, param, response);
    return;
  }

  if(uri[2] == "register") {
    project_register_image(pname, param, response);
    return;
  }

  if(uri[2] == "configure") {
    project_configure(pname, param, response);
    return;
  }

  if(uri[2] == "index_status") {
    project_index_status(pname, param, response);
    return;
  }

  if (uri[2].front() == '_') {
    // project command (e.g. _conf,)
    if (uri[2] == "_image_src_count") {
      uint32_t count = project_image_src_count(pname);
      response.set_text_payload( std::to_string(count) );
      response.set_status(200);
      return;
    }
    if (uri[2] == "_conf") {
      std::ostringstream json;
      d_projects.at(pname)->conf_to_json(json);
      response.set_json_payload(json.str());
      response.set_status(200);
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
    std::string asset_uri_decoded;
    bool decode_result = vise::url_decode(asset, asset_uri_decoded);
    if(decode_result) {
      std::string image_dir = d_projects.at(pname)->pconf("image_dir");
      boost::filesystem::path fn(image_dir);
      fn = fn / asset_uri_decoded;
      file_send(fn, response);
    } else {
      response.set_status(400);
    }
    return;
  }

  response.set_status(400);
  return;
}
//
// POST /{PNAME}/_index_*
//

bool project_manager::project_exists(std::string pname) const {
  boost::filesystem::path project_dir = d_conf.at("vise_store");
  project_dir = project_dir / pname;

  if (boost::filesystem::exists(project_dir)) {
    return true;
  } else {
    return false;
  }
}

bool project_manager::project_is_loaded(std::string pname) const {
  if (d_projects.count(pname) == 1) {
    return true;
  } else {
    return false;
  }
}

bool project_manager::project_create(std::string pname) {
  try {
    boost::filesystem::path vise_store = d_conf.at("vise_store");
    boost::filesystem::path project_dir = vise_store / pname;
    boost::filesystem::create_directory(project_dir);
    return true;
  } catch(std::exception &ex) {
    std::cout << "project_manager::project_create() : " << ex.what() << std::endl;
    return false;
  }
}

bool project_manager::project_load(std::string pname) {
  std::lock_guard<std::mutex> lock(d_project_load_mutex);

  if (d_projects.count(pname) == 1) {
    return true;
  }

  try {
    d_projects[pname] = std::unique_ptr<vise::project>(new vise::project(pname, d_conf));
    std::cout << "project_manager::project_load(): done" << std::endl;
    return true;
  } catch(std::exception &e) {
    std::cout << "project_manager::project_load(): "
              << e.what() << std::endl;
    return false;
  }
}

void project_manager::project_index_create(std::string pname,
                                           http_response &response,
                                           bool block_until_done) {
  if (project_load(pname)) {
    bool success;
    std::string message;
    d_projects.at(pname)->conf_reload();
    d_projects.at(pname)->index_create(success, message, block_until_done);
    std::ostringstream redirect_url;
    redirect_url << d_conf.at("http_uri_namespace") << pname << "/index_status";
    response.redirect_to(redirect_url.str());
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

void project_manager::project_index_search(std::string pname,
                                           std::map<std::string, std::string> const &param,
                                           http_response &response) const {
  if( !project_is_loaded(pname) ) {
    response.set_status(412);
    response.set_payload("project not loaded yet");
    return;
  }

  if( !d_projects.at(pname)->index_is_loaded() ) {
    response.set_status(412);
    response.set_payload("project index not loaded");
    return;
  }

  if (param.count("file_id") == 0) {
    response.set_status(412);
    response.set_payload("search query must contain file_id parameter.");
    return;
  }

  uint32_t file_id = std::atoi(param.at("file_id").c_str());

  vise::search_query query(param);
  if(query.d_max_result_count == 0) {
    query.d_max_result_count = 1024;
  }

  query.d_filename = d_projects.at(pname)->filename(file_id);
  std::vector<vise::search_result> result;
  d_projects.at(pname)->index_search(query, result);

  std::ostringstream json;
  json << "{\"PNAME\":\"" << pname << "\""
       << ",\"QUERY\":" << query.to_json()
       << ",\"RESULT_SIZE\":" << result.size();
  if(result.size()) {
    json<< ",\"RESULT\":[" << result.at(0).to_json();
    for(uint32_t i=1; i < result.size(); ++i) {
      json << "," << result.at(i).to_json();
    }
    json << "]";
  }
  json << "}";

  if(param.count("response_format") == 1 &&
     param.at("response_format") == "json" ) {
    response.set_json_payload(json.str());
  } else {
    // default response format is HTML
    std::ostringstream html;
    html << vise::PROJECT_HTML_HEAD
         << "<body>\n"
         << HTML_SVG_ASSETS
         << "<script>\n"
         << "// JS code generated automatically by src/vise/project_manager.cc::project_index_search()\n"
         << "var _vise_data = " << json.str() << ";\n"
         << "</script>\n"
         << "<script src=\"../vise_common.js\"></script>\n"
         << "<script src=\"../project_search.js\"></script>\n"
         << vise::HTML_TAIL;
    response.set_html_payload(html.str());
  }
}


void project_manager::project_show_match(std::string pname,
                                         std::map<std::string, std::string> const &param,
                                         http_response &response) const {
  if( !project_is_loaded(pname) ) {
    response.set_status(412);
    response.set_payload("project not loaded yet");
    return;
  }

  if( !d_projects.at(pname)->index_is_loaded() ) {
    response.set_status(412);
    response.set_payload("project index not loaded");
    return;
  }

  if (param.count("file_id") == 0 ||
      param.count("match_file_id") == 0 ||
      param.count("x") == 0 ||
      param.count("y") == 0 ||
      param.count("width") == 0 ||
      param.count("height") == 0
      ) {
    response.set_status(412);
    response.set_payload("showmatch requests must contain: qfile_id, mfile_id, x, y, width, height, H");
    return;
  }

  uint32_t file_id = std::atoi(param.at("file_id").c_str());
  uint32_t match_file_id = std::atoi(param.at("match_file_id").c_str());
  vise::search_query query(param);
  query.d_filename = d_projects.at(pname)->filename(file_id);

  std::ostringstream match;
  match << "{'file_id':" << param.at("match_file_id") << ","
        << "'filename':\"" << d_projects.at(pname)->filename(match_file_id) << "\"}";

  std::ostringstream match_details;
  d_projects.at(pname)->index_internal_match(query, match_file_id, match_details);

  std::ostringstream json;
  json << "{\"PNAME\":\"" << pname << "\""
       << ",\"QUERY\":" << query.to_json()
       << ",\"MATCH\":" << match.str()
       << ",\"MATCH_DETAILS\":" << match_details.str()
       << "}";

  if(param.count("response_format") == 1 &&
     param.at("response_format") == "json" ) {
    response.set_json_payload(json.str());
  } else {
    // default response format is HTML
    std::ostringstream html;
    html << vise::PROJECT_HTML_HEAD
         << "<body>\n"
         << HTML_SVG_ASSETS
         << "<script>\n"
         << "// JS code generated automatically by src/vise/project_manager.cc::project_index_search()\n"
         << "var _vise_data = " << json.str() << ";\n"
         << "</script>\n"
         << "<script src=\"../vise_common.js\"></script>\n"
         << "<script src=\"../project_showmatch.js\"></script>\n"
         << vise::HTML_TAIL;
    response.set_html_payload(html.str());
  }
}

void project_manager::project_register_image(std::string pname,
                                             std::map<std::string, std::string> const &param,
                                             http_response &response) const {
  if( !project_is_loaded(pname) ) {
    response.set_status(412);
    response.set_payload("project not loaded yet");
    return;
  }

  if (!d_projects.at(pname)->index_is_loaded()) {
    response.set_status(412);
    response.set_payload("project index not loaded");
    return;
  }

  if (param.count("file1_id") == 0 ||
      param.count("x") == 0 ||
      param.count("y") == 0 ||
      param.count("width") == 0 ||
      param.count("height") == 0 ||
      param.count("file2_id") == 0 ||
      param.count("H0") == 0
      ) {
    response.set_status(412);
    response.set_payload("register requests must contain: file1_id, file2_id, x, y, width, height [ and optionally H0 ]");
    return;
  }

  std::string H0_str = param.at("H0");
  if (H0_str.front() != '[' && H0_str.back() != ']') {
    response.set_status(412);
    response.set_payload("H0 must be a JSON array like H0 = [0.92, 0, 21.92, ..., ]");
    return;
  }

  uint32_t file1_id = std::atoi(param.at("file1_id").c_str());
  uint32_t file2_id = std::atoi(param.at("file2_id").c_str());
  double x = std::atoi(param.at("x").c_str());
  double y = std::atoi(param.at("y").c_str());
  double width = std::atoi(param.at("width").c_str());
  double height = std::atoi(param.at("height").c_str());

  std::array<double, 9> H0;
  H0_str = H0_str.substr(1, H0_str.length() - 2);
  std::stringstream ss(H0_str); // csv
  char comma;
  // parse the comma separated values
  ss >> H0[0] >> comma >> H0[1] >> comma >> H0[2] >> comma
     >> H0[3] >> comma >> H0[4] >> comma >> H0[5] >> comma
     >> H0[6] >> comma >> H0[7] >> comma >> H0[8];
  std::array<double, 9> H = H0;

  // @todo
  // image registration module should be detached from relja_retrival search engine
  d_projects.at(pname)->register_image(file1_id, file2_id, x, y, width, height, H);
  std::ostringstream json;
  json << "{\"H\":["
       << H[0] << "," << H[1] << "," << H[2] << ","
       << H[3] << "," << H[4] << "," << H[5] << ","
       << H[6] << "," << H[7] << "," << H[8] << "]}";
  response.set_status(200);
  response.set_payload(json.str());
  response.set_field("Content-Type", "application/json");
  return;
}

void project_manager::project_home(std::string pname,
                                   http_response& response) const {
  std::ostringstream ss;

  switch (d_projects.at(pname)->state()) {
  case vise::project_state::SET_CONFIG:
    {
      std::ostringstream redirect_url;
      redirect_url << d_conf.at("http_uri_namespace") << pname << "/configure";
      response.redirect_to(redirect_url.str());
      break;
    }
  case vise::project_state::INDEX_ONGOING:
    {
      std::ostringstream redirect_url;
      redirect_url << d_conf.at("http_uri_namespace") << pname << "/index_status";
      response.redirect_to(redirect_url.str());
      break;
    }
  case vise::project_state::BROKEN_INDEX:
    ss << "<h1>Incomplete Visual Index</h1><p>Visual index files for project [" << pname << "] are broken and therefore cannot be loaded. This can happen if the indexing process gets terminated prematurely.</p>";
    ss << "<p><form method=\"post\" action=\"" << d_conf.at("http_uri_namespace") << pname << "/_index_create\"><input type=\"submit\" value=\"Resume Process to Fix Broken Index\"></form></p><p>If you do not need this project any more, you can delete it from VISE settings panel in the home page.</p>";
    response.set_html_payload(ss.str());
    break;
  case vise::project_state::SEARCH_READY:
    {
      std::ostringstream redirect_url;
      redirect_url << d_conf.at("http_uri_namespace") << pname << "/filelist";
      response.redirect_to(redirect_url.str());
      break;
    }
  case vise::project_state::INIT_FAILED:
    {
      ss << "<h1>Failed to Initialize Project</h1><p>Failed to initialize project [" << pname << "]. This usually happens if the project data folder and its files are manually updated or the configuration file is invalid.</p>";
      response.set_html_payload(ss.str());
      break;
    }

  default:
    ss << "unknown project state: "
       << d_projects.at(pname)->state_name() << std::endl;
    response.set_html_payload(ss.str());
  }
}

void project_manager::project_filelist(std::string pname,
                                       std::map<std::string, std::string> const& param,
                                       http_response& response) const {
  // check if project is loaded
  if(!d_projects.at(pname)->index_is_loaded()) {
    std::ostringstream ss;
    ss << "Indexing has not been loaded yet. Some large indexes may take a bit longer (e.g. 10 seconds) to load. ";
    ss << "<br/>Please referesh this page using your browsers \"Reload\" or \"Refresh\" button.";
    response.set_html_payload(ss.str());
  }

  uint32_t start = 0;
  uint32_t end;
  if(param.count("start") == 1) {
    start = std::atoi(param.at("start").c_str());
    if(start >= d_projects.at(pname)->fid_count()) {
      start = 0;
    }
  }
  if(param.count("end") == 1) {
    end = std::atoi(param.at("end").c_str());
    if(end == start || end > d_projects.at(pname)->fid_count()) {
      end = fminl(50, d_projects.at(pname)->fid_count());
    }
  } else {
    end = fminl(start + 50, d_projects.at(pname)->fid_count());
  }

  std::ostringstream json;
  json << "{\"PNAME\":\"" << pname << "\""
       << ",\"FLIST_SIZE\":" << d_projects.at(pname)->fid_count()
       << ",\"FLIST_START\":" << start
       << ",\"FLIST_END\":" << end;
  if(start != end) {
    json << ",\"FLIST\":["
         << "\"" << d_projects.at(pname)->filename(start) << "\"";
    for(uint32_t fid=start+1; fid < end; ++fid) {
      json << ",\"" << d_projects.at(pname)->filename(fid) << "\"";
    }
    json << "]";
  }
  json << "}";

  if(param.count("response_format") == 1 &&
     param.at("response_format") == "json" ) {
    response.set_json_payload(json.str());
  } else {
    // default response format is HTML
    std::ostringstream html;
    html << vise::PROJECT_HTML_HEAD
         << "<body>\n"
         << HTML_SVG_ASSETS
         << "<script>\n"
         << "// JS code generated automatically by src/vise/project_manager.cc::project_filelist()\n"
         << "var _vise_data = " << json.str() << ";\n"
         << "</script>\n"
         << "<script src=\"../vise_common.js\"></script>\n"
         << "<script src=\"../project_filelist.js\"></script>\n"
         << vise::HTML_TAIL;
    response.set_html_payload(html.str());
  }
}

void project_manager::project_file(std::string pname,
                                   std::map<std::string, std::string> const &param,
                                   http_response &response) const {
  if(param.count("file_id") == 0) {
    response.set_payload("file_id missing");
    response.set_field("Content-Type", "text/plain");
    response.set_status(412);
    return;
  }

  uint32_t file_id = std::atoi(param.at("file_id").c_str());
  if(file_id >= d_projects.at(pname)->fid_count()) {
    response.set_payload("invalid file_id");
    response.set_field("Content-Type", "text/plain");
    response.set_status(412);
    return;
  }

  std::ostringstream json;
  json << "{\"PNAME\":\"" << pname << "\""
       << ",\"FILE_ID\":" << file_id
       << ",\"FILENAME\":\"" << d_projects.at(pname)->filename(file_id) << "\""
       << ",\"FLIST_SIZE\":" << d_projects.at(pname)->fid_count()
       << "}";

  if(param.count("response_format") == 1 &&
     param.at("response_format") == "json" ) {
    response.set_json_payload(json.str());
  } else {
    // default response format is HTML
    std::ostringstream html;
    html << vise::PROJECT_HTML_HEAD
         << "<body>\n"
         << HTML_SVG_ASSETS
         << "<script>\n"
         << "// JS code generated automatically by src/vise/project_manager.cc::project_file()\n"
         << "var _vise_data = " << json.str() << ";\n"
         << "</script>\n"
         << "<script src=\"../vise_common.js\"></script>\n"
         << "<script src=\"../via0.js\"></script>\n"
         << "<script src=\"../project_file.js\"></script>\n"
         << vise::HTML_TAIL;
    response.set_html_payload(html.str());
  }
}

void project_manager::project_configure(std::string pname,
                                        std::map<std::string, std::string> const &param,
                                        http_response &response) const {
  if(d_projects.at(pname)->index_is_done()) {
    std::ostringstream uri;
    uri << d_conf.at("http_uri_namespace") << pname << "/";
    response.redirect_to(uri.str());
    return;
  }

  std::ostringstream json;
  json << "{\"PNAME\":\"" << pname << "\""
       << ",\"PCONF_PRESET_LIST\":";
  d_projects.at(pname)->preset_conf_to_json(json);
  json << ",\"PRESET_CONF_ID\":\"" << d_projects.at(pname)->pconf("preset_conf_id") << "\","
       << "\"IMAGE_SRC_COUNT\":" << d_projects.at(pname)->image_src_count()
       << "}";

  // default response format is HTML
  std::ostringstream html;
  html << vise::PROJECT_HTML_HEAD
       << "<body>\n"
       << HTML_SVG_ASSETS
       << "<script>\n"
       << "// JS code generated automatically by src/vise/project_manager.cc::project_configure()\n"
       << "var _vise_data = " << json.str() << ";\n"
       << "</script>\n"
       << "<script src=\"../vise_common.js\"></script>\n"
       << "<script src=\"../project_configure.js\"></script>\n"
       << vise::HTML_TAIL;
  response.set_html_payload(html.str());
}

void project_manager::project_index_status(std::string pname,
                                           std::map<std::string, std::string> const &param,
                                           http_response &response) const {
  std::ostringstream json;
  json << "{\"PNAME\":\"" << pname << "\""
       << ",\"INDEX_STATUS\":" << d_projects.at(pname)->index_status_to_json()
       << "}";

  if(param.count("response_format") == 1 &&
     param.at("response_format") == "json" ) {
    response.set_json_payload(json.str());
  } else {
    if(d_projects.at(pname)->index_is_done()) {
      std::ostringstream uri;
      uri << d_conf.at("http_uri_namespace") << pname << "/";
      response.redirect_to(uri.str());
      return;
    } else {
      // default response format is HTML
      std::ostringstream html;
      html << vise::PROJECT_HTML_HEAD
           << "<body>\n"
           << HTML_SVG_ASSETS
           << "<script>\n"
           << "// JS code generated automatically by src/vise/project_manager.cc::project_index_status()\n"
           << "var _vise_data = " << json.str() << ";\n"
           << "</script>\n"
           << "<script src=\"../vise_common.js\"></script>\n"
           << "<script src=\"../project_index_status.js\"></script>\n"
           << vise::HTML_TAIL;
      response.set_html_payload(html.str());
    }
  }

}

//
// Handlers for VISE main user interface via GET /{home,...}, PUT /
//
void project_manager::vise_home(std::map<std::string, std::string> const &param,
                                http_response &response) const {
  std::vector<std::string> pname_list;
  project_pname_list(pname_list);

  std::ostringstream json;
  json << "{\"VISE_FULLNAME\":\"" << VISE_FULLNAME << "\""
       << ",\"VISE_NAME\":\"" << VISE_NAME << "\""
       << ",\"VERSION\":\"" << VISE_VERSION_MAJOR << "." << VISE_VERSION_MINOR << "." << VISE_VERSION_PATCH << "\""
       << ",\"PROJECT_LIST\":{";
  if(pname_list.size()) {
    json << "\"" << pname_list.at(0) << "\":{}";
    for(uint32_t i=1; i<pname_list.size(); ++i) {
      json << ",\"" << pname_list.at(i) << "\":{}";
    }
  }
  json << "}}";

  if(param.count("response_format") == 1 &&
     param.at("response_format") == "json" ) {
    response.set_json_payload(json.str());
  } else {
    // default response format is HTML
    std::ostringstream html;
    html << vise::VISE_HTML_HEAD
         << "<body>\n"
         << vise::HTML_SVG_ASSETS
         << "<script>\n"
         << "// JS code generated automatically by src/vise/project_manager.cc::vise_home()\n"
         << "var _vise_data = " << json.str() << ";\n"
         << "</script>\n"
         << "<script src=\"vise_common.js\"></script>\n"
         << "<script src=\"vise_home.js\"></script>\n"
         << vise::HTML_EMPTY_TAIL;
    response.set_html_payload(html.str());
  }
  return;
}

void project_manager::project_pname_list(std::vector<std::string> &pname_list) const {
  pname_list.clear();

  boost::filesystem::path vise_store(d_conf.at("vise_store"));
  boost::filesystem::directory_iterator end_itr;
  for (boost::filesystem::directory_iterator it(vise_store); it!=end_itr; ++it) {
    if (boost::filesystem::is_directory(it->path())) {
      pname_list.push_back(boost::filesystem::relative(it->path(), vise_store).string());
    }
  }
}

uint32_t project_manager::project_image_src_count(std::string pname) const {
  return d_projects.at(pname)->image_src_count();
}

void project_manager::vise_project_create(std::map<std::string, std::string> const &param,
                                          http_response &response) {
  std::string response_format = "html";
  if(param.count("response_format") == 1) {
    if(param.at("response_format") == "json") {
      response_format = "json";
    }
  }

  if(param.count("pname") == 0) {
    std::string message = "project name is missing";
    vise_error_page(message, response_format, response);
    return;
  }
  std::string pname;
  vise::url_decode(param.at("pname"), pname);

  if(pname == "") {
    std::string message = "project name cannot be empty";
    vise_error_page(message, response_format, response);
    return;
  }

  if(!is_project_name_valid("pname")) {
    std::ostringstream msg;
    msg << "project name [" << pname << "] "
        << "should only contains the following alpha-numeric character [a-z|A-Z|0-9] and space "
        << "and it cannot contain spaces or special characters like %, *, &, $, etc.";
    vise_error_page(msg.str(), response_format, response);
    return;
  }

  if(project_exists(pname)) {
    std::ostringstream msg;
    msg << "project [" << pname << "] already exists.";
    vise_error_page(msg.str(), response_format, response);
    return;
  }

  if(!project_create(pname)) {
    std::ostringstream msg;
    msg << "failed to create project [" << pname << "].";
    vise_error_page(msg.str(), response_format, response);
    return;
  }

  if(!project_load(pname)) {
    std::ostringstream msg;
    msg << "failed to load project [" << pname << "].";
    vise_error_page(msg.str(), response_format, response);
    return;
  }

  std::ostringstream json;
  json << d_conf.at("http_uri_namespace") << pname << "/";
  response.redirect_to(json.str());
  return;
}

void project_manager::vise_project_delete(std::map<std::string, std::string> const &param,
                                          http_response &response) {
  try {
    std::map<std::string, std::string>::const_iterator itr;
    for (itr = param.begin(); itr != param.end(); ++itr) {
      if (itr->second != "1") {
        continue;
      }
      std::string pname;
      vise::url_decode(itr->first, pname);
      if (!project_exists(pname)) {
        continue;
      }

      if (d_projects.count(pname)) {
        bool success;
        std::string message;
        d_projects[pname]->index_unload(success, message);
        if (!success) {
          std::cout << "index_unload() failed for project " << pname << std::endl;
          std::cout << message << std::endl;
        }
        d_projects[pname].reset(nullptr);
        std::map<std::string, std::unique_ptr<vise::project> >::iterator pitr = d_projects.find(pname);
        d_projects.erase(pitr);
      }

      boost::filesystem::path project_dir = boost::filesystem::path(d_conf.at("vise_store")) / pname;
      uint32_t del_count = boost::filesystem::remove_all(project_dir);
      std::cout << "deleted project " << pname << " ("
                << del_count << " files)" << std::endl;
    }

    std::ostringstream redirect_url;
    redirect_url << d_conf.at("http_uri_namespace") << "settings";
    response.redirect_to(redirect_url.str());
  }
  catch (std::exception& ex) {
    std::cout << "vise_project_delete(): exception: " << ex.what() << std::endl;
  }
}

void project_manager::vise_project_unload(std::map<std::string, std::string> const& param,
                                          http_response& response) {
  std::map<std::string, std::string>::const_iterator itr;
  for (itr = param.begin(); itr != param.end(); ++itr) {
    if (itr->second != "1") {
      continue;
    }
    std::string pname;
    vise::url_decode(itr->first, pname);
    if (!project_exists(pname)) {
      continue;
    }
    try {
      if (d_projects.count(pname)) {
        bool success;
        std::string message;
        d_projects[pname]->index_unload(success, message);
        d_projects[pname].reset(nullptr);
        std::map<std::string, std::unique_ptr<vise::project> >::iterator itr = d_projects.find(pname);
        d_projects.erase(itr);
      }
    }
    catch (std::exception & ex) {
      std::cout << "exception: " << ex.what() << std::endl;
    }
  }

  std::ostringstream redirect_url;
  redirect_url << d_conf.at("http_uri_namespace") << "settings";
  response.redirect_to(redirect_url.str());
}

void project_manager::vise_settings(std::map<std::string, std::string> const &param,
                                    http_response &response) const {
  std::ostringstream json;
  json << "{\"VISE_FULLNAME\":\"" << VISE_FULLNAME << "\""
       << ",\"VISE_NAME\":\"" << VISE_NAME << "\""
       << ",\"VISE_STORE\":\"" << d_conf.at("vise_store") << "\""
       << ",\"VERSION\":\"" << VISE_VERSION_MAJOR << "." << VISE_VERSION_MINOR << "." << VISE_VERSION_PATCH << "\""
       << ",\"SETTINGS\":{";

  std::map<std::string, std::string>::const_iterator itr;
  bool is_first_item = true;
  for(itr = d_conf.begin(); itr != d_conf.end(); ++itr) {
    if(itr->first.front() != '#') {
      if(is_first_item) {
        is_first_item = false;
      } else {
        json << ",";
      }
      json << "\"" << itr->first << "\":\"" << vise::json_escape_str(itr->second) << "\"";
    }
  }
  json << "},\"PROJECT_LIST\":{";
  std::vector<std::string> pname_list;
  project_pname_list(pname_list);
  if(pname_list.size()) {
    json << "\"" << pname_list.at(0) << "\":{}";
    for(uint32_t i=1; i<pname_list.size(); ++i) {
      json << ",\"" << pname_list.at(i) << "\":{}";
    }
  }
  json << "},\"LOADED_PROJECT_LIST\":[";
  if (d_projects.size()) {
      std::map<std::string, std::unique_ptr<vise::project> >::const_iterator itr = d_projects.begin();
      json << "\"" << itr->first << "\"";
      ++itr;
      for (; itr != d_projects.end(); ++itr) {
          json << ",\"" << itr->first << "\"";
      }
  }
  json << "]}";

  if(param.count("response_format") == 1 &&
     param.at("response_format") == "json" ) {
    response.set_json_payload(json.str());
  } else {
    // default response format is HTML
    std::ostringstream html;
    html << vise::VISE_HTML_HEAD
         << "<body>\n"
         << vise::HTML_SVG_ASSETS
         << "<script>\n"
         << "// JS code generated automatically by src/vise/project_manager.cc::vise_settings()\n"
         << "var _vise_data = " << json.str() << ";\n"
         << "console.log(_vise_data);\n"
         << "</script>\n"
         << "<script src=\"vise_common.js\"></script>\n"
         << "<script src=\"vise_settings.js\"></script>\n"
         << vise::HTML_EMPTY_TAIL;
    response.set_html_payload(html.str());
  }
  return;
}

void project_manager::vise_settings_save(std::string &settings_formdata,
                                         http_response &response) {
  std::map<std::string, std::string> conf;
  std::istringstream ss(settings_formdata);
  std::string line;
  while(std::getline(ss, line)) {
    std::size_t eqpos = line.find('=');
    if(eqpos != std::string::npos) {
      std::string key = line.substr(0, eqpos);
      std::string val = line.substr(eqpos+1);
      conf[key] = val;
    }
  }

  vise::init_vise_settings_comments(conf); // set the comments
  const boost::filesystem::path visehome = vise::vise_home();
  boost::filesystem::path vise_settings = visehome / "vise_settings.txt";
  bool result = vise::configuration_save(conf, vise_settings.string());
  if(result) {
    response.set_status(200);
    response.set_text_payload("Saved, please restart of VISE.");
    return;
  } else {
    response.set_status(412);
    response.set_text_payload("Failed");
    return;
  }
}

void project_manager::vise_about(std::map<std::string, std::string> const &param,
                                 http_response &response) const {
  std::ostringstream json;
  json << "{\"VISE_FULLNAME\":\"" << VISE_FULLNAME << "\""
       << ",\"VISE_NAME\":\"" << VISE_NAME << "\""
       << ",\"VERSION\":\"" << VISE_VERSION_MAJOR << "." << VISE_VERSION_MINOR << "." << VISE_VERSION_PATCH << "\""
       << "}";

  if(param.count("response_format") == 1 &&
     param.at("response_format") == "json" ) {
    response.set_json_payload(json.str());
  } else {
    // default response format is HTML
    std::ostringstream html;
    html << vise::VISE_HTML_HEAD
         << "<body>\n"
         << vise::HTML_SVG_ASSETS
         << "<script>\n"
         << "// JS code generated automatically by src/vise/project_manager.cc::vise_about()\n"
         << "var _vise_data = " << json.str() << ";\n"
         << "</script>\n"
         << "<script src=\"vise_common.js\"></script>\n"
         << "<script src=\"vise_about.js\"></script>\n"
         << vise::HTML_EMPTY_TAIL;
    response.set_html_payload(html.str());
  }
  return;
}

void project_manager::vise_error_page(const std::string message,
                                      const std::string response_format,
                                      http_response &response) const {
  std::ostringstream json;
  json << "{\"STATUS\":\"Error\""
       << ",\"MESSAGE\":\"" << message << "\"}";
  if(response_format == "json" ) {
    response.set_json_payload(json.str());
  } else {
    // default response format is HTML
    std::ostringstream html;
    html << vise::VISE_HTML_HEAD
         << "<body>\n"
         << vise::HTML_SVG_ASSETS
         << "<script>\n"
         << "// JS code generated automatically by src/vise/project_manager.cc::vise_error_page()\n"
         << "var _vise_data = " << json.str() << ";\n"
         << "</script>\n"
         << "<script src=\"../vise_common.js\"></script>\n"
         << "<script src=\"../vise_error.js\"></script>\n"
         << vise::HTML_TAIL;
    response.set_html_payload(html.str());
  }
}

void project_manager::vise_wait_page(const std::string message,
                                     const std::string response_format,
                                     http_response &response) const {
  std::ostringstream json;
  json << "{\"STATUS\":\"Please wait\""
       << ",\"MESSAGE\":\"" << message << "\"}";
  if(response_format == "json" ) {
    response.set_json_payload(json.str());
  } else {
    // default response format is HTML
    std::ostringstream html;
    html << vise::VISE_WAIT_PAGE_HEAD
         << "<body>\n"
         << vise::HTML_SVG_ASSETS
         << "<script>\n"
         << "// JS code generated automatically by src/vise/project_manager.cc::vise_wait_page()\n"
         << "var _vise_data = " << json.str() << ";\n"
         << "</script>\n"
         << "<script src=\"../vise_common.js\"></script>\n"
         << "<script src=\"../vise_wait.js\"></script>\n"
         << vise::HTML_TAIL;
    response.set_html_payload(html.str());
  }
}

bool project_manager::is_project_name_valid(const std::string pname) const {
  if(pname.find('/') != std::string::npos ||
     pname.find('\\') != std::string::npos ||
     pname.find('*') != std::string::npos ||
     pname.find('&') != std::string::npos ||
     pname.find('.') != std::string::npos ||
     pname.find('$') != std::string::npos ||
     pname.find('"') != std::string::npos ||
     pname.find('\'') != std::string::npos ||
     pname.find('%') != std::string::npos ||
     pname.find('^') != std::string::npos) {
    return false;
  } else {
    return true;
  }
}

//
// POST /{PNAME}/_file_*
//
void project_manager::project_file_add(std::string pname,
                                       std::map<std::string, std::string> const &param,
                                       http_response &response) {
  if(param.count("source_type") == 0 ||
     param.count("source_loc") == 0 ) {
    response.set_text_payload("required fields missing: source_type, source_loc");
    response.set_status(412);
    return;
  }

  if(param.at("source_type") != "local_folder") {
    response.set_text_payload("unknown value for parameter \"source_type\"");
    response.set_status(412);
    return;
  }

  if(param.at("source_loc").size() > 512) {
    response.set_text_payload("\"source_loc\" cannot be more than 512 bytes.");
    response.set_status(412);
    return;
  }
  std::string source_loc;
  vise::url_decode(param.at("source_loc"), source_loc);
  boost::filesystem::path src(source_loc);
  src.make_preferred();
  if(!boost::filesystem::exists(src) ||
     !boost::filesystem::is_directory(src)) {
    response.set_text_payload("folder does not exist");
    response.set_status(412);
    return;
  }

  boost::filesystem::path dst_prefix(d_projects.at(pname)->pconf("image_src_dir"));
  if(dst_prefix.string() == "" ||
     !boost::filesystem::exists(dst_prefix)) {
    response.set_text_payload("image_src_dir does not exist");
    response.set_status(412);
    return;
  }

  boost::filesystem::recursive_directory_iterator end_itr;
  std::vector<std::string> added_fn_list;
  std::vector<std::string> discarded_fn_list;
  for(boost::filesystem::recursive_directory_iterator it(src); it!=end_itr; ++it) {
    if(boost::filesystem::is_regular_file(it->path())) {
      if(it->path().extension() == ".jpg" ||
         it->path().extension() == ".JPG" ||
         it->path().extension() == ".jpeg" ||
         it->path().extension() == ".JPEG" ||
         it->path().extension() == ".png" ||
         it->path().extension() == ".PNG" ||
         it->path().extension() == ".TIF" ||
         it->path().extension() == ".tif" ||
         it->path().extension() == ".TIFF" ||
         it->path().extension() == ".tiff"
         ) {
        boost::filesystem::path dst(dst_prefix);
        dst = dst / boost::filesystem::relative(it->path(), src);
        boost::filesystem::path dst_dir = dst.parent_path();
        if (!boost::filesystem::exists(dst_dir)) {
            boost::filesystem::create_directories(dst_dir);
        }
        if(!boost::filesystem::exists(dst)) {
          boost::filesystem::copy_file(it->path(), dst);
          added_fn_list.push_back(boost::filesystem::relative(dst, dst_prefix).string());
        } else {
          discarded_fn_list.push_back(it->path().string());
        }
      } else {
        discarded_fn_list.push_back(it->path().string());
      }
    }
  }

  std::ostringstream json;
  json << "{\"PNAME\":\"" << pname << "\""
       << ",\"COMMAND\":\"_file_add\""
       << ",\"SOURCE_TYPE\":\"" << param.at("source_type") << "\""
       << ",\"SOURCE_LOC\":\"" << vise::json_escape_str(src.string()) << "\""
       << ",\"ADDED_FILENAME_LIST\":[";
  if(added_fn_list.size()) {
    json << "\"" << vise::json_escape_str(added_fn_list.at(0)) << "\"";
    for(uint32_t i=1; i<added_fn_list.size(); ++i) {
      json << ",\"" << vise::json_escape_str(added_fn_list.at(i)) << "\"";
    }
  }
  json << "],\"DISCARDED_FILENAME_LIST\":[";
  if(discarded_fn_list.size()) {
    json << "\"" << vise::json_escape_str(discarded_fn_list.at(0)) << "\"";
    for(uint32_t i=1; i<discarded_fn_list.size(); ++i) {
      json << ",\"" << vise::json_escape_str(discarded_fn_list.at(i)) << "\"";
    }
  }
  json << "]}";

  if(param.count("response_format") == 1 &&
     param.at("response_format") == "json" ) {
    response.set_json_payload(json.str());
  } else {
    // default response format is HTML
    std::ostringstream html;
    html << vise::PROJECT_HTML_HEAD
         << "<body>\n"
         << "<script>\n"
         << "// JS code generated automatically by src/vise/project_manager.cc::project_file_add()\n"
         << "var _vise_data = " << json.str() << ";\n"
         << "</script>\n"
         << vise::HTML_TAIL;
    response.set_html_payload(html.str());
  }
}

//
// POST /{PNAME}/_conf
//
void project_manager::project_config_save(std::string pname,
                                          std::string &config_formdata,
                                          http_response &response) {
  bool success = d_projects.at(pname)->conf_from_plaintext(config_formdata);
  if(success) {
    response.set_status(200);
    response.set_text_payload("Saved");
    return;
  } else {
    response.set_status(412);
    response.set_text_payload("Failed");
    return;
  }
}

void project_manager::project_config_use_preset(std::string pname,
                                                std::string preset_conf_id,
                                                http_response &response) {
  bool success = d_projects.at(pname)->use_preset_conf(preset_conf_id);
  if(success) {
    response.set_status(200);
    response.set_text_payload("Saved");
    return;
  } else {
    response.set_status(412);
    response.set_text_payload("Failed to use preset configuration");
    return;
  }
}

//
// limit serving
//
void project_manager::serve_only(std::map<std::string, std::string> const pname_pconf_fn_map) {
  std::lock_guard<std::mutex> lock(d_project_load_mutex);

  try {
    std::map<std::string, std::string>::const_iterator itr;
    for(itr=pname_pconf_fn_map.begin(); itr!=pname_pconf_fn_map.end(); ++itr) {
      if (d_projects.count(itr->first) == 1) {
        continue ;
      } else {
        d_projects[itr->first] = std::unique_ptr<vise::project>(new vise::project(itr->first, itr->second));
      }
    }
    is_serve_only_active = true;
  } catch(std::exception &e) {
    std::cout << "project_manager::serve_only(): "
              << e.what() << std::endl;
    throw std::runtime_error(e.what());
    is_serve_only_active = false;
  }
}

void project_manager::serve_only_4xx_response(http_response &response) const {
  std::ostringstream html;
  html << vise::VISE_HTML_HEAD
       << "<body>\n"
       << "<p>The following VISE projects are available for search query:</p>"
       << "<ul>";
  std::map<std::string, std::unique_ptr<vise::project> >::const_iterator itr;
  for(itr=d_projects.begin(); itr!=d_projects.end(); ++itr) {
    html << "<li><a href=\"" << d_conf.at("http_uri_namespace") << "/" << itr->first << "/\">" << itr->first << "</a></li>";
  }
  html << "</ul>"
       << vise::HTML_EMPTY_TAIL;
  response.set_html_payload(html.str());
}

//
// http uri namespace
//
void project_manager::uri_namespace_mismatch_4xx_response(http_response &response) const {
  std::ostringstream html;
  std::ostringstream vise_home;
  vise_home << "http://" << d_conf.at("address") << ":" << d_conf.at("port")
            << d_conf.at("http_uri_namespace");
  html << vise::VISE_HTML_HEAD
       << "<body>\n"
       << "<p>Resources hosted by VISE are available at: <a href=\""
       << vise_home.str() << "\">" << vise_home.str() << "</a></p>"
       << vise::HTML_EMPTY_TAIL;
  response.set_html_payload(html.str());
}
