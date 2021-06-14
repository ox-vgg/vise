/** @file vise_util.h
 *  @brief Various utility functions used by VISE
 *  @author Abhishek Dutta
 *  @date 12 Nov. 2019
 */

#include "vise_util.h"

//
// vise command line interface
//

// Usage: ./vise --run-mode=[create-project | serve-project | ...]
//               --port=9103 --vise-home=/data/vise --nthread=4
//               PNAME1:PCONF1 PNAME2:PCONF2 ...
bool vise::parse_cli_args(int argc, char **argv,
                          std::unordered_map<std::string, std::string> &cli_args,
                          std::unordered_map<std::string, std::string> &pname_pconf_list) {
  cli_args.clear();
  pname_pconf_list.clear();
  if(argc == 1) {
    return false;
  }

  for(std::size_t i=1; i<argc; ++i) {
    std::string arg(argv[i]);
    if(arg.size() < 3) {
      std::cout << "vise::parse_cli_args(): ignoring unknown argument ["
                << arg << "]" << std::endl;
      continue;
    }
    if(arg[0] == '-' && arg[1] == '-') {
      std::size_t eq_pos = arg.find('=');
      std::string key, val;
      if(eq_pos == std::string::npos) {
        // for arguments without a value (e.g. --version, --help, ...)
        key = arg.substr(2, arg.size());
        val = "";
      } else {
        key = arg.substr(2, eq_pos-2);
        val = arg.substr(eq_pos+1);
      }
      if(cli_args.count(key) == 1) {
        std::cout << "vise::parse_vise_cli_args(): duplicate arguments for ["
                  << key << "], last one will be used." << std::endl;
      }
      cli_args[key] = val;
    } else {
      std::size_t colon_pos = arg.find(':');
      if(colon_pos == std::string::npos) {
        std::cout << "vise::parse_cli_args(): ignoring invalid argument ["
                  << arg << "]" << std::endl;
        continue;
      }
      std::string pname(arg.substr(0, colon_pos));
      boost::filesystem::path project_conf_fn(arg.substr(colon_pos+1));
      if(pname_pconf_list.count(pname) == 1) {
        std::cout << "vise::parse_vise_cli_args(): duplicate project name ["
                  << pname << "], last one will be used." << std::endl;
      }

      pname_pconf_list[pname] = project_conf_fn.string();
    }
  }
  return true;
}

//
// vise settings
//
bool vise::does_vise_home_and_store_exist(std::map<std::string, std::string> &vise_settings) {
  if(vise_settings.count("vise_home") == 0 ||
     vise_settings.count("vise_store") == 0 ||
     vise_settings.count("www_store") == 0 ||
     vise_settings.count("asset_store") == 0 ) {
    std::cout << "vise::does_vise_home_and_store_exist(): missing vise_home, vise_store, www_store, asset_store."
              << std::endl;
    return false;
  }
  const boost::filesystem::path vise_home(vise_settings.at("vise_home"));
  const boost::filesystem::path vise_store(vise_settings.at("vise_store"));
  const boost::filesystem::path www_store(vise_settings.at("www_store"));
  const boost::filesystem::path asset_store(vise_settings.at("asset_store"));

  if(!boost::filesystem::exists(vise_home)) {
    return false;
  }
  if(!boost::filesystem::exists(vise_store)) {
    return false;
  }
  if(!boost::filesystem::exists(www_store)) {
    return false;
  }
  if(!boost::filesystem::exists(asset_store)) {
    return false;
  }
  return true;
}

bool vise::create_vise_home_and_store(std::map<std::string, std::string> &vise_settings) {
  if(vise_settings.count("vise_home") == 0 ||
     vise_settings.count("vise_store") == 0 ||
     vise_settings.count("www_store") == 0 ||
     vise_settings.count("asset_store") == 0 ) {
    std::cout << "vise::create_vise_store(): missing vise_home, vise_store, www_store, asset_store."
              << std::endl;
    return false;
  }

  const boost::filesystem::path vise_home(vise_settings.at("vise_home"));
  const boost::filesystem::path vise_store(vise_settings.at("vise_store"));
  const boost::filesystem::path www_store(vise_settings.at("www_store"));
  const boost::filesystem::path asset_store(vise_settings.at("asset_store"));

  if(boost::filesystem::exists(vise_home)) {
    if(!boost::filesystem::exists(vise_store)) {
      boost::filesystem::create_directory(vise_store);
    }
    if(!boost::filesystem::exists(www_store)) {
      boost::filesystem::create_directory(vise_store);
    }
    if(!boost::filesystem::exists(asset_store)) {
      boost::filesystem::create_directory(vise_store);
    }
  } else {
    boost::filesystem::create_directories(vise_home);
    boost::filesystem::create_directory(vise_store);
    boost::filesystem::create_directory(www_store);
    boost::filesystem::create_directory(asset_store);
  }
  return true;
}

void vise::init_default_vise_settings(std::map<std::string, std::string> &vise_settings) {
  const boost::filesystem::path vise_home = vise::vise_home();

  // use default configuration for VISE
  boost::filesystem::path project_dir = vise_home / "project";
  boost::filesystem::path asset_dir = vise_home / "asset";
  boost::filesystem::path www_dir = vise_home / "www";

  vise_settings.clear();
  vise_settings["vise-home-dir"] = vise_home.string();
  vise_settings["vise-project-dir"] = project_dir.string();
  vise_settings["vise-asset-dir"] = asset_dir.string();
  vise_settings["http-www-dir"] = www_dir.string();
  vise_settings["http-address"] = "localhost";
  vise_settings["http-port"] = "9669";
  vise_settings["http-worker"] = "2";
  vise_settings["http-namespace"] = "/";
}

void vise::init_vise_settings(std::map<std::string, std::string> &vise_settings) {
  const boost::filesystem::path visehome = vise::vise_home();
  boost::filesystem::path vise_settings_fn = visehome / "vise_settings.txt";

  if(!boost::filesystem::exists(vise_settings_fn)) {
    // use default configuration for VISE
    boost::filesystem::path project_dir = visehome / "project";
    boost::filesystem::path asset_dir = visehome / "asset";
    boost::filesystem::path www_dir = visehome / "www";

    if(!boost::filesystem::exists(visehome)) {
      boost::filesystem::create_directories(visehome);
      boost::filesystem::create_directory(project_dir);
      boost::filesystem::create_directory(asset_dir);
      boost::filesystem::create_directory(www_dir);
    }
    boost::filesystem::path generic_visual_vocabulary(asset_dir);
    generic_visual_vocabulary = generic_visual_vocabulary / "relja_retrival";
    generic_visual_vocabulary = generic_visual_vocabulary / "visual_vocabulary";
    generic_visual_vocabulary = generic_visual_vocabulary / "latest";

    vise_settings.clear();
    vise_settings["vise-home-dir"] = visehome.string();
    vise_settings["vise-project-dir"] = project_dir.string();
    vise_settings["vise-asset-dir"] = asset_dir.string();
    vise_settings["http-www-dir"] = www_dir.string();
    vise_settings["generic-visual-vocabulary"] = generic_visual_vocabulary.string();
    vise_settings["http-address"] = "localhost";
    vise_settings["http-port"] = "9669";
    vise_settings["http-worker"] = "2";
    vise_settings["http-namespace"] = "/";
    vise_settings["nthread-indexing"] = "-1"; // use all threads

    vise::configuration_save(vise_settings, vise_settings_fn.string());
  } else {
    vise_settings.clear();
    vise::configuration_load(vise_settings_fn.string(), vise_settings);

    bool settings_changed = false;
    if(vise_settings.count("vise-asset-dir") == 0) {
      boost::filesystem::path asset_dir = visehome / "asset";
      boost::filesystem::create_directory(asset_dir);
      vise_settings["vise-asset-dir"] = asset_dir.string();
      settings_changed = true;
    }
    if(vise_settings.count("generic-visual-vocabulary") == 0) {
      boost::filesystem::path asset_dir = visehome / "asset";
      boost::filesystem::path generic_visual_vocabulary(asset_dir);
      generic_visual_vocabulary = generic_visual_vocabulary / "relja_retrival";
      generic_visual_vocabulary = generic_visual_vocabulary / "visual_vocabulary";
      generic_visual_vocabulary = generic_visual_vocabulary / "latest";
      vise_settings["generic-visual-vocabulary"] = generic_visual_vocabulary.string();
      settings_changed = true;
    }
    if(vise_settings.count("http-namespace") == 0) {
      vise_settings["http-namespace"] = "/";
      settings_changed = true;
    } else {
      if(vise_settings.at("http-namespace").back() != '/') {
        std::string ns_with_slash = vise_settings.at("http-namespace") + "/";
        vise_settings["http-namespace"] = ns_with_slash;
      }
    }
    if(vise_settings.count("nthread-indexing") == 0) {
      vise_settings["nthread-indexing"] = "-1";
      settings_changed = true;
    }
    if(vise_settings.count("this-file-saved-by") == 0) {
      settings_changed = true;
    }
    if(settings_changed) {
      vise::configuration_save(vise_settings, vise_settings_fn.string());
    }
  }
}

//
// vise configuration
//
bool vise::configuration_load(std::string filename,
                              std::map<std::string, std::string> &conf ) {
  conf.clear();
  std::ifstream file(filename);
  if (!file.is_open()) {
    std::cerr << "failed to open configuration file ["
              << filename << "]" << std::endl;
    return false;
  }

  std::string line;
  while (!file.eof() && file.good()) {
    std::getline(file, line);
    if (line[0] == '#') {
      continue; // ignore comments
    }
    std::size_t sep_loc = line.find("=");
    if (sep_loc == std::string::npos) {
      continue; // ignore malformed line
    }
    std::string key = line.substr(0, sep_loc);
    std::string value = line.substr(sep_loc+1);
    conf[key] = value;
  }
  file.close();
  //std::cout << "vise_util::configuration_load(): loaded from " << filename << std::endl;

  return true;
}

bool vise::configuration_save(std::map<std::string, std::string> &conf,
                              std::string filename) {
  std::ofstream file(filename);
  if (!file.is_open()) {
    std::cerr << "failed to open configuration file ["
              << filename << "]" << std::endl;
    return false;
  }

  // automatically add a marker of the VISE version saving this conf
  std::ostringstream ss;
  ss << VISE_NAME << "-" << VISE_VERSION_MAJOR
     << "." << VISE_VERSION_MINOR << "." << VISE_VERSION_PATCH;
  conf["this-file-saved-by"] = ss.str();

  std::map<std::string, std::string>::const_iterator itr;
  for(itr = conf.begin(); itr != conf.end(); ++itr) {
    file << itr->first << "=" << itr->second << std::endl;
  }
  file.close();
  //std::cout << "vise_util::configuration_save(): saved to " << filename << std::endl;
  return true;
}

void vise::configuration_show(std::map<std::string, std::string> const &conf) {
  std::map<std::string, std::string>::const_iterator itr;
  for (itr=conf.begin(); itr!=conf.end(); ++itr) {
    if(itr->first.at(0) != '#') {
      std::cout << "::" << itr->first << "=" << itr->second << std::endl;
    }
  }
}

bool vise::starts_with(const std::string &s, const std::string prefix) {
  if (s.length() < prefix.length()) {
    return false;
  }
  if ( s.substr(0, prefix.length()) == prefix ) {
    return true;
  } else {
    return false;
  }
}

bool vise::ends_with(const std::string &s, const std::string suffix) {
  size_t pos = s.length() - suffix.length();
  if ( s.substr( pos ) == suffix ) {
    return true;
  } else {
    return false;
  }
}

bool vise::contains(const std::string &s, const std::string substr) {
  if ( s.find(substr) == std::string::npos ) {
    return false;
  } else {
    return true;
  }
}

std::vector<std::string> vise::split(const std::string &s,
                                     const char separator) {
  std::vector<std::string> chunks;
  std::vector<std::size_t> seperator_index_list;

  std::size_t start = 0;
  std::size_t sep_index;
  while ( start < s.length() ) {
    sep_index = s.find(separator, start);
    if ( sep_index == std::string::npos ) {
      break;
    } else {
      chunks.push_back( s.substr(start, sep_index - start) );
      start = sep_index + 1;
    }
  }
  if ( start != s.length() ) {
    chunks.push_back( s.substr(start) );
  }
  return chunks;
}

void vise::split(const std::string &s,
                 const char separator,
                 const std::size_t start,
                 const std::size_t stop,
                 std::vector<std::string> &chunks) {
  if(s.size() == 0) {
    return;
  }
  if(start==stop) {
    return;
  }

  std::size_t current_index = start;
  std::size_t sep = s.find(separator, current_index);
  if(sep == std::string::npos ||
     sep >= stop) {
    chunks.push_back(s.substr(start, stop-start));
    return;
  } else {
    while(sep < stop) {
      chunks.push_back(s.substr(current_index, sep-current_index));
      current_index = sep + 1;
      sep = s.find(separator, current_index);
      if(sep == std::string::npos ||
         sep >= stop) {
        chunks.push_back(s.substr(current_index, stop-current_index));
        break;
      }
    }
  }
}

void vise::decompose_uri(const std::string &uri,
                         std::vector<std::string>& uri_components,
                         std::map<std::string, std::string>& uri_param) {
  const char stop_char = '?';
  const char sep_char = '/';
  uri_components.clear();
  uri_param.clear();
  std::size_t start = 0;
  std::size_t stop = uri.find(stop_char);
  if(stop==std::string::npos) {
    stop = uri.size();
  }
  vise::split(uri, sep_char, start, stop, uri_components);

  if(stop!=uri.size()) {
    std::string param_str = uri.substr(stop+1);
    if(param_str.size()==0) {
      return;
    }
    std::vector<std::string> param_list = vise::split(param_str, '&');

    std::size_t np = param_list.size();
    if ( np ) {
      for ( std::size_t i = 0; i < np; ++i ) {
        std::vector<std::string> pi = vise::split(param_list.at(i), '=');
        if ( pi.size() == 2 ) {
          uri_param[ pi[0] ] = pi[1];
        }
      }
    }
  }
}

void vise::escape_string(std::string &in, char match_char, std::string match_replacement) {
  std::size_t start_index = 0;
  std::size_t match_index = in.find(match_char, start_index);
  while(match_index != std::string::npos) {
    in.replace(match_index, 1, match_replacement);
    start_index = match_index + match_replacement.size();
    match_index = in.find(match_char, start_index);
  }
}

bool vise::file_load(const boost::filesystem::path fn,
                     std::string& file_content)
{
  if( !boost::filesystem::exists(fn) ) {
    std::cout << "file does not exist [" << fn.string() << "]" << std::endl;
    return false;
  }
  if( !boost::filesystem::is_regular_file(fn) ) {
    std::cout << "not a regular file [" << fn.string() << "]" << std::endl;
    return false;
  }

  try {
    std::ifstream f;
    f.open(fn.string().c_str(), std::ios::in | std::ios::binary);
    f.seekg(0, std::ios::end);
    file_content.clear();
    file_content.reserve( f.tellg() );
    f.seekg(0, std::ios::beg);
    file_content.assign( (std::istreambuf_iterator<char>(f)),
                         std::istreambuf_iterator<char>() );
    f.close();
    return true;
  } catch(std::exception &e) {
    std::cout << "failed to load file [" << fn.string() << "]" << std::endl;
    return false;
  }
}

bool vise::file_save(const boost::filesystem::path fn,
                     std::string& file_content)
{
  try {
    std::ofstream f;
    f.open(fn.string().c_str());
    f << file_content;
    f.close();
    return true;
  } catch(std::exception &e) {
    std::cout << "failed to save file [" << fn.string() << "]" << std::endl;
    return false;
  }
}

bool vise::file_save_binary(const boost::filesystem::path fn,
                            std::string& file_content)
{
  try {
    std::ofstream f;
    f.open(fn.string().c_str(), std::ofstream::binary);
    f << file_content;
    f.close();
    return true;
  }
  catch (std::exception& e) {
    std::cout << "failed to save file [" << fn.string() << "]" << std::endl;
    return false;
  }
}

void vise::print_map(std::string name,
                     std::map<std::string, std::string> const &m )
{
  std::ostringstream s;
  std::map<std::string, std::string>::const_iterator it;

  for ( it = m.begin(); it != m.end(); ++it ) {
    s << it->first << "=" << it->second << ", ";
  }
  std::cout << name << " = [" << s.str() << "]" << std::endl;
}

// source: https://www.boost.org/doc/libs/1_46_0/doc/html/boost_asio/example/http/server3/request_handler.cpp
// Author: Christopher M. Kohlhoff (chris at kohlhoff dot com)
bool vise::url_decode(const std::string& in, std::string& out)
{
  out.clear();
  out.reserve(in.size());
  for (std::size_t i = 0; i < in.size(); ++i)  {
    if (in[i] == '%') {
      if (i + 3 <= in.size()) {
        int value = 0;
        std::istringstream is(in.substr(i + 1, 2));
        if (is >> std::hex >> value) {
          out += static_cast<char>(value);
          i += 2;
        }
        else {
          return false;
        }
      }
      else {
        return false;
      }
    }
    else {
      out += in[i];
    }
  }
  return true;
}

// source: https://www.boost.org/doc/libs/1_46_0/doc/html/boost_asio/example/http/server3/request_handler.cpp
// Author: Christopher M. Kohlhoff (chris at kohlhoff dot com)
bool vise::decode_uri_query_param(const std::string& in, std::string& out)
{
  out.clear();
  out.reserve(in.size());
  for (std::size_t i = 0; i < in.size(); ++i)  {
    if (in[i] == '%') {
      if (i + 3 <= in.size()) {
        int value = 0;
        std::istringstream is(in.substr(i + 1, 2));
        if (is >> std::hex >> value) {
          out += static_cast<char>(value);
          i += 2;
        }
        else {
          return false;
        }
      }
      else {
        return false;
      }
    }
    else {
      if(in[i] == '+') {
        out += ' ';
      } else {
        out += in[i];
      }
    }
  }
  return true;
}

void vise::parse_urlencoded_form(const std::string &formdata_str,
                                 std::map<std::string, std::string> &formdata) {
  formdata.clear();
  std::vector<std::string> tokens = vise::split(formdata_str, '&');
  if(tokens.size()) {
    for(uint32_t i=0; i<tokens.size(); ++i) {
      std::vector<std::string> keyval = vise::split(tokens.at(i), '=');
      if(keyval.size() == 2) {
        formdata.insert( std::pair<std::string, std::string>(keyval[0], keyval[1]) );
      }
    }
  }
}

std::string vise::json_escape_str(const std::string& in) {
  // C:\Data\My "Project" Name\image.jpg
  // C:\\Data\\My \"Project\" Name\\image.jpg
  std::string result;
  if (in.find('"') != std::string::npos ||
      in.find('\\') != std::string::npos
      ) {
    result.reserve(2*in.size());
		for (uint32_t i = 0; i < in.size(); ++i) {
      switch (in[i]) {
      case '\"':
        result.append("\\\"");
        break;
      case '\\':
        result.append("\\\\");
        break;
      default:
        result.push_back(in[i]);
      }
		}
    result.shrink_to_fit();
  }
  else {
    result = in;
  }
  return result;
}

std::string vise::now_timestamp() {
  std::time_t now = std::time(nullptr);
  std::string ts( std::asctime(std::localtime(&now)) );
  ts.pop_back();
  return ts;
}

uint32_t vise::getmillisecs() {
  std::chrono::steady_clock::time_point now = std::chrono::steady_clock::now();
  return std::chrono::time_point_cast<std::chrono::milliseconds>(now).time_since_epoch().count();
}

boost::filesystem::path vise::vise_home() {
#ifdef _WIN32
  boost::filesystem::path home(std::getenv("LOCALAPPDATA"));
  boost::filesystem::path vise_home_dir = home / "VISE";
#else
  boost::filesystem::path home(std::getenv("HOME"));
  std::string vise_dirname(".vise");
  boost::filesystem::path vise_home_dir = home / vise_dirname;
#endif
  return vise_home_dir;
}

bool vise::is_valid_image(std::string img_fn, std::string &message) {
  if(!boost::filesystem::exists(boost::filesystem::path(img_fn))) {
    message = "file missing";
    return false;
  }

  bool success = true;
  try {
    Magick::Image img(img_fn);
    img.quiet(true); // to supress warnings
    if(img.rows() < 32 || img.columns() < 32) {
      std::ostringstream ss;
      ss << "image size " << img.columns() << "x" << img.rows() << " must be greater than 32x32";
      message = ss.str();
      success = false;
    } else if(img.colorSpace() != Magick::RGBColorspace &&
              img.colorSpace() != Magick::sRGBColorspace &&
              img.colorSpace() != Magick::GRAYColorspace) {
      std::ostringstream ss;
      ss << "image color space " << img.colorSpace() << " is not "
         << Magick::RGBColorspace << " (i.e. RGB) or " << Magick::sRGBColorspace
         << " (i.e. sRGB)";
      message = ss.str();
      success = false;
    } else if(img.magick() != "JPEG") {
      message = "image format must be JPEG";
      success = false;
    }
    img.quiet(false);

  } catch(std::exception &ex) {
    message = ex.what();
    success = false;
  }
  return success;
}

bool vise::if_valid_get_image_size(std::string img_fn, std::string &message, uint32_t &width, uint32_t &height) {
  if(!boost::filesystem::exists(boost::filesystem::path(img_fn))) {
    message = "file missing";
    return false;
  }

  bool success = true;
  try {
    Magick::Image img(img_fn);
    img.quiet(true); // to supress warnings
    width = img.columns();
    height = img.rows();
    if(img.rows() < 32 || img.columns() < 32) {
      std::ostringstream ss;
      ss << "image size " << img.columns() << "x" << img.rows() << " must be greater than 32x32";
      message = ss.str();
      success = false;
    } else if(img.colorSpace() != Magick::RGBColorspace &&
              img.colorSpace() != Magick::sRGBColorspace &&
              img.colorSpace() != Magick::GRAYColorspace) {
      message = "image color space must be RGB";
      success = false;
    } else if(img.magick() != "JPEG") {
      message = "image format must be JPEG";
      success = false;
    }
    img.quiet(false);

  } catch(std::exception &ex) {
    message = ex.what();
    success = false;
  }
  return success;
}

void vise::csv_string_to_float_array(std::string csv_string,
                                     std::vector<float> &float_array) {
  std::vector<std::string> tokens = vise::split(csv_string, ',');
  float_array.clear();
  for(std::size_t i=0; i<tokens.size(); ++i) {
    float_array.push_back( std::stof(tokens.at(i) ) );
  }
}

double vise::iou(std::vector<float> &a, std::vector<float> &b) {
  // a and b are rectangle coordinates formatted as : x, y, width, height
  double ax0 = (double) a[0];
  double ay0 = (double) a[1];
  double ax1 = ax0 + (double) a[2];
  double ay1 = ay0 + (double) a[3];

  double bx0 = (double) b[0];
  double by0 = (double) b[1];
  double bx1 = bx0 + (double) b[2];
  double by1 = by0 + (double) b[3];

  // coordinates of the intersection box
  double x0 = fmax(ax0, bx0);
  double y0 = fmax(ay0, by0);
  double x1 = fmin(ax1, bx1);
  double y1 = fmin(ay1, by1);

  double overlap_width  = (x1 - x0);
  double overlap_height = (y1 - y0);
  if( overlap_width < 0 || overlap_height < 0) {
    return 0.0;
  }

  double overlap_area = overlap_width * overlap_height;
  double area_a = (ax1 - ax0) * (ay1 - ay0);
  double area_b = (bx1 - bx0) * (by1 - by0);
  double combined_area = area_a + area_b - overlap_area;
  double epsilon = 1e-5;
  double iou = overlap_area / (combined_area + epsilon);
  /*
  std::cout << "overlap_area=" << overlap_area << ", area_a=" << area_a
            << ", area_b=" << area_b << ", combined_area=" << combined_area
            << ", iou=" << iou << std::endl;*/

  return iou;
}
