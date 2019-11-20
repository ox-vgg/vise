/** @file vise_util.h
 *  @brief Various utility functions used by VISE
 *  @author Abhishek Dutta
 *  @date 12 Nov. 2019
 */

#include "vise_util.h"

void vise::configuration_load(std::string filename,
                              std::map<std::string, std::string> &conf ) {
  conf.clear();
  std::ifstream file(filename);
  if (!file.is_open()) {
    throw std::runtime_error("failed to open configuration file [" + filename + "]");
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
}

void vise::configuration_show(std::map<std::string, std::string> const &conf) {
  std::map<std::string, std::string>::const_iterator itr;
  for (itr=conf.begin(); itr!=conf.end(); ++itr) {
    std::cout << "::" << itr->first << "=" << itr->second << std::endl;
  }
}

bool vise::starts_with(const std::string &s, const std::string prefix) {
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

std::vector<std::string> vise::split(const std::string &s, const char separator) {
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
                 const std::string stop_string,
                 std::vector<std::string> &chunks) {
  chunks.clear();
  if (s.size() == 0) {
    return;
  }

  std::size_t start = 0;
  std::size_t stop = s.find(stop_string);
  if (stop == std::string::npos) {
    stop = s.size();
  }
  std::size_t sep = s.find(separator, start);
  if (sep == std::string::npos ||
      sep >= stop) {
    chunks.push_back(s.substr(start, stop-start));
    return;
  } else {
    while (sep < stop) {
      chunks.push_back(s.substr(start, sep-start));
      start = sep + 1;
      sep = s.find(separator, start);
      if (sep == std::string::npos) {
        chunks.push_back(s.substr(start, stop-start));
        break;
      }
    }
  }
}

void vise::decompose_uri(const std::string &uri,
                         std::vector<std::string>& uri_components,
                         std::map<std::string, std::string>& uri_param) {
  vise::split(uri, '/', "?", uri_components);
  std::size_t n = uri_components.size();

  std::size_t param_start_index = uri_components.at(n-1).find('?');
  if ( param_start_index != std::string::npos ) {
    std::string param_str = uri_components.at(n-1).substr( param_start_index + 1 );
    uri_components.at(n-1) = uri_components.at(n-1).substr(0, param_start_index);
    std::vector<std::string> param_list = vise::split(param_str, '&');

    std::size_t np = param_list.size();
    if ( np ) {
      uri_param.clear();
      for ( std::size_t i = 0; i < np; ++i ) {
        std::vector<std::string> pi = vise::split(param_list.at(i), '=');
        if ( pi.size() == 2 ) {
          uri_param[ pi[0] ] = pi[1];
        }
      }
    }
  }
}

bool vise::load_file(const boost::filesystem::path fn,
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
