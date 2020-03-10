#include "project_manager.h"

using namespace vise;

project_manager::project_manager(std::map<std::string, std::string> const &conf)
  : d_conf(conf)
{
  std::cout << "project_manager: showing configuration"
            << std::endl;
  vise::configuration_show(d_conf);
  d_projects.clear();

  /*
  // DEBUG
  std::string debug_pname("15ci");
  //std::string debug_pname("ox200test");
  std::cout << "project_manager(): DEBUG**********************, loading "
            << debug_pname
            << std::endl;
  bool success = project_load(debug_pname);
  if(success) {
    std::clog << "DEBUG: loaded project " << debug_pname << std::endl;
  } else {
    std::cerr << "DEBUG: failed to project " << debug_pname << std::endl;
  }
  */
}

project_manager::~project_manager() {
  std::cout << "~project_manager()" << std::endl;
}

// WARNING: this method may be invoked by multiple threads simultaneously
// be careful of avoid race conditions
void project_manager::process_http_request(http_request const &request,
                                           http_response &response)
{
  std::ostringstream ss;
  ss << "project_manager: " << request.d_method << " " << request.d_uri;

  std::vector<std::string> uri;
  std::map<std::string, std::string> param;
  vise::decompose_uri(request.d_uri, uri, param);
  std::cout << ss.str() << std::endl;

  //request.parse_urlencoded_form_data();
  //vise::print_vector("uri", uri);
  //vise::print_map("param", param);
  //vise::print_map("d_fields", request.d_fields);
  //vise::print_map("d_multipart_data", request.d_multipart_data);
  //std::cout << "payload=" << request.d_payload.str() << std::endl;

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
    response.redirect_to("/home");
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
    {
      boost::filesystem::path file_loc(d_conf.at("www_store"));
      file_loc = file_loc / uri[1];
      file_send(file_loc, response);
      return;
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
      // project command that do not trigger loading of a project
      if (uri[2] == "_cover_image") {
        boost::filesystem::path image_dir(d_conf.at("vise_store"));
        image_dir = image_dir / pname;
        image_dir = image_dir / "image";
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

    if(uri[2].size() == 0) {
      // GET /PNAME/ redirects to project home
      project_home(pname, response);
      return;
    }

    if(uri[2] == "filelist") {
      project_filelist(pname, param, response);
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
        uint32_t count = d_projects.at(pname)->image_src_count();
        response.set_text_payload( std::to_string(count) );
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
        boost::filesystem::path fn(d_conf.at("vise_store"));
        fn = fn / pname;
        fn = fn / "image";
        fn = fn / asset_uri_decoded;
        file_send(fn, response);
      } else {
        response.set_status(400);
      }
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
    if (uri[1].front() == '_') {
      if(uri[1] == "_project_create") {
        std::map<std::string, std::string> formdata;
        vise::parse_urlencoded_form(request.d_payload.str(), formdata);
        vise_project_create(formdata, response);
        return;
      }
      if(uri[1] == "_project_delete") {
        std::cout << "payload: " << request.d_payload.str() << std::endl;
        std::map<std::string, std::string> formdata;
        vise::parse_urlencoded_form(request.d_payload.str(), formdata);
        vise_project_delete(formdata, response);
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
    bool ok = vise::file_save(fn, payload);
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
  } catch(std::exception &e) {
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
    return true;
  } catch(std::exception &e) {
    std::cout << "project_manager::project_load(): " << e.what() << std::endl;
    return false;
  }
}

void project_manager::project_delete(std::string pname) {
  boost::filesystem::path project_dir = d_conf.at("vise_store");
  project_dir = project_dir / pname;
  boost::filesystem::remove_all(project_dir);
}

void project_manager::project_index_create(std::string pname,
                                           http_response &response) {
  if (project_load(pname)) {
    bool success;
    std::string message;
    d_projects.at(pname)->index_create(success, message);
    std::ostringstream redirect_url;
    redirect_url << "/" << pname << "/index_status";
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

  std::cout << "project_index_search(): " << std::endl;
  uint32_t file_id = std::atoi(param.at("file_id").c_str());

  vise::search_query query(param);
  if(query.d_max_result_count == 0) {
    query.d_max_result_count = 256;
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
         << "<script src=\"/vise_common.js\"></script>\n"
         << "<script src=\"/project_search.js\"></script>\n"
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
       << ",\"MATCH_DETAILS\":" << match_details.str() << "}";

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
         << "<script src=\"/vise_common.js\"></script>\n"
         << "<script src=\"/project_showmatch.js\"></script>\n"
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

  if( !d_projects.at(pname)->index_is_loaded() ) {
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
  if(H0_str.front() != '[' && H0_str.back() != ']') {
    response.set_status(412);
    response.set_payload("H0 must be a JSON array like H0 = [0.92, 0, 21.92, ..., ]");
    return;
  }

  uint32_t file1_id = std::atoi(param.at("file1_id").c_str());
  uint32_t file2_id = std::atoi(param.at("file2_id").c_str());
  double x = std::atoi(param.at("x").c_str());
  double y = std::atoi(param.at("x").c_str());
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
  std::ostringstream debug_response;
  debug_response << "{\"H\":["
                 << H[0] << "," << H[1] << "," << H[2] << ","
                 << H[3] << "," << H[4] << "," << H[5] << ","
                 << H[6] << "," << H[7] << "," << H[8] << "]}";
  response.set_status(200);
  response.set_payload(debug_response.str());
  response.set_field("Content-Type", "application/json");
  return;
}

void project_manager::project_home(std::string pname,
                                   http_response &response) const {
  std::cout << pname << ": state=" << d_projects.at(pname)->state_name() << std::endl;
  std::ostringstream ss;

  switch(d_projects.at(pname)->state()) {
  case vise::project_state::SET_CONFIG:
    {
      std::ostringstream redirect_url;
      redirect_url << "/" << pname << "/configure";
      response.redirect_to(redirect_url.str());
      break;
    }
  case vise::project_state::INDEX_ONGOING:
    {
      std::ostringstream redirect_url;
      redirect_url << "/" << pname << "/index_status";
      response.redirect_to(redirect_url.str());
      break;
    }
  case vise::project_state::BROKEN_INDEX:
    ss << "Index seems to be broken" << std::endl;
    response.set_html_payload(ss.str());
    break;
  case vise::project_state::SEARCH_READY:
    {
      std::ostringstream redirect_url;
      redirect_url << "/" << pname << "/filelist";
      response.redirect_to(redirect_url.str());
      break;
    }
  default:
    ss << "unknown project state: "
       << d_projects.at(pname)->state_name() << std::endl;
    response.set_html_payload(ss.str());
  }
}

void project_manager::project_filelist(std::string pname,
                                       std::map<std::string, std::string> const &param,
                                       http_response &response) const {
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

  uint32_t per_page = end - start;
  if(param.count("per_page") == 1) {
    per_page = std::atoi(param.at("per_page").c_str());
  }

  std::ostringstream json;
  json << "{\"PNAME\":\"" << pname << "\""
       << ",\"FLIST_SIZE\":" << d_projects.at(pname)->fid_count()
       << ",\"FLIST_PER_PAGE\":" << per_page
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
         << "<script src=\"/vise_common.js\"></script>\n"
         << "<script src=\"/project_filelist.js\"></script>\n"
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
         << "<script src=\"/vise_common.js\"></script>\n"
         << "<script src=\"/via0.js\"></script>\n"
         << "<script src=\"/project_file.js\"></script>\n"
         << vise::HTML_TAIL;
    response.set_html_payload(html.str());
  }
}

void project_manager::project_configure(std::string pname,
                                        std::map<std::string, std::string> const &param,
                                        http_response &response) const {
  if(d_projects.at(pname)->index_is_done()) {
    std::ostringstream uri;
    uri << "/" << pname << "/";
    response.redirect_to(uri.str());
    return;
  }

  std::ostringstream json;
  json << "{\"PNAME\":\"" << pname << "\""
       << ",\"PCONF\":";
  d_projects.at(pname)->conf_to_json(json);
  json << ",\"IMAGE_SRC_COUNT\":"
       << d_projects.at(pname)->image_src_count()
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
       << "<script src=\"/vise_common.js\"></script>\n"
       << "<script src=\"/project_configure.js\"></script>\n"
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
      uri << "/" << pname << "/";
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
           << "<script src=\"/vise_common.js\"></script>\n"
           << "<script src=\"/project_index_status.js\"></script>\n"
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
         << "<script src=\"/vise_common.js\"></script>\n"
         << "<script src=\"/vise_home.js\"></script>\n"
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
  json << "/" << pname << "/";
  response.redirect_to(json.str());
  return;
}

void project_manager::vise_project_delete(std::map<std::string, std::string> const &param,
                                          http_response &response) {

  std::map<std::string, std::string>::const_iterator itr;
  for(itr=param.begin(); itr!=param.end(); ++itr) {
    if(itr->second != "1") {
      continue;
    }
    std::string pname;
    vise::url_decode(itr->first, pname);
    if(!project_exists(pname)) {
      continue;
    }

    if(d_projects.count(pname)) {
      d_projects[pname].reset(nullptr);
    }

    boost::filesystem::path project_dir = boost::filesystem::path(d_conf.at("vise_store")) / pname;
    uint32_t del_count = boost::filesystem::remove_all(project_dir);
    std::cout << "deleted project " << pname << " ("
              << del_count << " files)" << std::endl;
  }

  std::ostringstream redirect_url;
  redirect_url << "/settings";
  response.redirect_to(redirect_url.str());
}

void project_manager::vise_settings(std::map<std::string, std::string> const &param,
                                    http_response &response) const {
  std::ostringstream json;
  json << "{\"VISE_FULLNAME\":\"" << VISE_FULLNAME << "\""
       << ",\"VISE_NAME\":\"" << VISE_NAME << "\""
       << ",\"VERSION\":\"" << VISE_VERSION_MAJOR << "." << VISE_VERSION_MINOR << "." << VISE_VERSION_PATCH << "\""
       << ",\"SETTINGS\":{";

  std::map<std::string, std::string>::const_iterator itr = d_conf.begin();
  json << "\"" << itr->first << "\":\"" << itr->second << "\"";
  ++itr;
  for(; itr != d_conf.end(); ++itr) {
    json << ",\"" << itr->first << "\":\"" << itr->second << "\"";
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
         << "// JS code generated automatically by src/vise/project_manager.cc::vise_settings()\n"
         << "var _vise_data = " << json.str() << ";\n"
         << "</script>\n"
         << "<script src=\"/vise_common.js\"></script>\n"
         << "<script src=\"/vise_settings.js\"></script>\n"
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
         << "<script src=\"/vise_common.js\"></script>\n"
         << "<script src=\"/vise_about.js\"></script>\n"
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
         << "<script src=\"/vise_common.js\"></script>\n"
         << "<script src=\"/vise_error.js\"></script>\n"
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

  std::string source_loc;
  vise::url_decode(param.at("source_loc"), source_loc);
  boost::filesystem::path src(source_loc);
  if(!boost::filesystem::exists(src) ||
     !boost::filesystem::is_directory(src)) {
    response.set_text_payload("folder does not exist");
    response.set_status(412);
    return;
  }

  boost::filesystem::path dst_prefix(d_projects.at(pname)->pconf("image_src_dir"));
  boost::filesystem::directory_iterator end_itr;
  std::vector<std::string> added_fn_list;
  std::vector<std::string> discarded_fn_list;
  for(boost::filesystem::directory_iterator it(src); it!=end_itr; ++it) {
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
        dst = dst / it->path().filename();
        if(!boost::filesystem::exists(dst)) {
          boost::filesystem::copy_file(it->path(), dst);
          added_fn_list.push_back(it->path().filename().string());
        } else {
          discarded_fn_list.push_back(it->path().filename().string());
        }
      } else {
        discarded_fn_list.push_back(it->path().filename().string());
      }
    }
  }

  std::ostringstream json;
  json << "{\"PNAME\":\"" << pname << "\""
       << ",\"COMMAND\":\"_file_add\""
       << ",\"SOURCE_TYPE\":\"" << param.at("source_type") << "\""
       << ",\"SOURCE_LOC\":\"" << src.string() << "\""
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
