#include "util.h"

// updated getTempFileName() for cross platform compatibility
// Abhishek Dutta <adutta@robots.ox.ac.uk>, 12 March 2020
std::string util::getTempFileName(std::string tmpDir, std::string prefix, std::string suffix) {
	boost::filesystem::path temp_dir;
	if (tmpDir == "") {
		temp_dir = boost::filesystem::temp_directory_path();
	}
	else {
		temp_dir = tmpDir;
	}
	std::ostringstream tmpfn_model_str;
	tmpfn_model_str << prefix << "%%%%%%%%%%" << suffix;

	boost::filesystem::path tmpfn_model(tmpfn_model_str.str());
	boost::filesystem::path tmpfn = temp_dir / boost::filesystem::unique_path(tmpfn_model);
	return tmpfn.string();
}