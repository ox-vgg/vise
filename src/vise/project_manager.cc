#include "project_manager.h"

using namespace vise;

project_manager::project_manager(std::map<std::string, std::string> const &conf)
  : d_conf(conf)
{
  std::cout << "project_manager: showing configuration"
            << std::endl;
  vise::configuration_show(d_conf);
  d_projects.clear();

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
  std::cout << ss.str() << std::endl;

  std::vector<std::string> uri;
  std::map<std::string, std::string> param;
  vise::decompose_uri(request.d_uri, uri, param);
  //vise::print_vector("uri", uri);
  //vise::print_map("param", param);

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
                                 http_response &response) const
{
  if ( uri.size() == 1 ) {
    response.set_status(404);
    return;
  }
  if (uri[0] != "") {
    response.set_status(404);
    return;
  }
  if(uri[1] == "") {
    // redirect localhost:port/ to localhost:port/_ui/index.html
    response.redirect_to("/index.html");
    return;
  }

  if (uri.size() == 2) {
    if (uri[1].front() == '_') {
      // a command (e.g. _create, _delete
      if(uri[1] == "_project_list") {
        project_list(param, response);
        return;
      }
      response.set_status(412);
      response.set_payload("unknown command");
      return;
    } else {
      // serve static content from www root
      boost::filesystem::path file_loc(d_conf.at("www_root"));
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

    if(uri[2].size() == 0) {
      // GET /PNAME/ redirects to project home
      project_home(pname, response);
      return;
    }

    if(uri[2] == "filelist") {
      project_filelist(pname, response, param);
      return;
    }

    if(uri[2] == "file") {
      project_file(pname, response, param);
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
      std::string asset_uri_decoded;
      bool decode_result = vise::url_decode(asset, asset_uri_decoded);
      if(decode_result) {
        boost::filesystem::path fn = d_conf.at("project_store");
        fn = fn / pname;
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

bool project_manager::project_is_loaded(std::string pname) const {
  if (d_projects.count(pname) == 1) {
    return true;
  } else {
    return false;
  }
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
    html << vise::HTML_HEAD
         << "<body>\n"
         << "<script>\n"
         << "// JS code generated automatically by src/vise/project_manager.cc::project_index_search()\n"
         << "var _vise_data = " << json.str() << ";\n"
         << "</script>\n"
         << "<script src=\"/project_search.js\"></script>\n"
         << "</script>\n"
         << "</body>\n"
         << "</html>";
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

  std::ostringstream html;
  html << vise::HTML_HEAD
       << "<body>\n"
       << "<script>\n"
       << "// JS code generated automatically by src/vise/project_manager.cc::project_show_match()\n"
       << "var _VISE_PNAME='" << pname << "';\n"
       << "var _VISE_QUERY=" << query.to_json() << ";\n"
       << "var _VISE_MATCH=" << match.str() << ";\n"
       << "var _VISE_MATCH_DETAILS=" << match_details.str() << ";\n";
  html << "\nconsole.log(_VISE_QUERY); console.log(_VISE_MATCH); console.log(_VISE_MATCH_DETAILS);\n";
  html << "</script>\n"
       << "<script src=\"/project_showmatch.js\"></script>\n"
       << "</script>\n"
       << "</body>\n"
       << "</html>";
  response.set_html_payload(html.str());
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

void project_manager::project_home(std::string pname,
                                   http_response &response) const {
  if(!project_is_loaded(pname)) {
    response.set_payload("project not loaded yet!");
    response.set_field("Content-Type", "text/plain");
    return;
  }

  std::cout << pname << ": state=" << d_projects.at(pname)->state_name() << std::endl;
  std::ostringstream ss;

  switch(d_projects.at(pname)->state()) {
  case vise::project_state::SET_CONFIG:
    ss << "Set configuration" << std::endl;
    response.set_html_payload(ss.str());
    break;
  case vise::project_state::INDEX_ONGOING:
    ss << "Index is ongoing" << std::endl;
    response.set_html_payload(ss.str());
    break;
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
                                       http_response &response,
                                       std::map<std::string, std::string> const &param) const {
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
    html << vise::HTML_HEAD
         << "<body>\n"
         << "<script>\n"
         << "// JS code generated automatically by src/vise/project_manager.cc::project_filelist()\n"
         << "var _vise_data = " << json.str() << ";\n"
         << "</script>\n"
         << "<script src=\"/project_filelist.js\"></script>\n"
         << "</script>\n"
         << "</body>\n"
         << "</html>";
    response.set_html_payload(html.str());
  }
}

void project_manager::project_file(std::string pname,
                                   http_response &response,
                                   std::map<std::string, std::string> const &param) const {
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
    html << vise::HTML_HEAD
         << "<body>\n"
         << "<script>\n"
         << "// JS code generated automatically by src/vise/project_manager.cc::project_file()\n"
         << "var _vise_data = " << json.str() << ";\n"
         << "</script>\n"
         << "<script src=\"/via0.js\"></script>\n"
         << "<script src=\"/project_file.js\"></script>\n"
         << "</script>\n"
         << "</body>\n"
         << "</html>";
    response.set_html_payload(html.str());
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
    response.set_payload("required fields missing: source_type, source_loc");
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
    response.set_payload("unknown value for parameter \"source_type\"");
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
  std::cout << fn.string() << std::endl;

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
// GET /_project?pname={PNAME}
// if pname is missing, returns metadata of all available projects
//
void project_manager::project_list(std::map<std::string, std::string> const &param,
                                   http_response &response) const {
  std::vector<std::string> pname_list;
  project_pname_list(pname_list);
  if(pname_list.empty()) {
    response.set_payload("{}");
    response.set_status(200);
    return;
  }
  std::ostringstream payload;
  payload << "{";
  std::vector<std::string>::const_iterator itr;
  for(itr=pname_list.begin(); itr!=pname_list.end(); ++itr) {
    std::string pname(*itr);
    boost::filesystem::path pconf_fn(d_conf.at("vise_store"));
    pconf_fn = pconf_fn / pname;
    pconf_fn = pconf_fn / "conf.txt";
    std::map<std::string, std::string> pconf;
    vise::configuration_load(pconf_fn.string(), pconf);
    if(itr!=pname_list.begin()) {
      payload << ",";
    }
    payload << "\"" << pname << "\":{";
    if(pconf.count("cover_image_filename")) {
      payload  << "\"cover_image_filename\":\""
               << pconf.at("cover_image_filename")
               << "\"";
    }
    payload << "}";
  }
  payload << "}";
  response.set_payload(payload.str());
  response.set_field("Content-Type", "application/json");
  response.set_status(200);
  return;
}

void project_manager::project_pname_list(std::vector<std::string> &pname_list) const {
  pname_list.clear();
  boost::filesystem::path project_store(d_conf.at("project_store"));
  boost::filesystem::directory_iterator end_itr;
  for (boost::filesystem::directory_iterator it(project_store); it!=end_itr; ++it) {
    if (boost::filesystem::is_directory(it->path())) {
      pname_list.push_back(boost::filesystem::relative(it->path(), project_store).string());
    }
  }
}

//
// DEBUG
//
void project_manager::debug() {
  std::string pname("ox200test");
  project_load(pname);

  bool success;
  std::string message;
  d_projects.at(pname)->index_load(success, message);

  std::map<std::string, std::string> param;
  param["file_id"] = "12";
  param["x"] = "64";
  param["y"] = "130";
  param["width"] = "64";
  param["height"] = "64";

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
