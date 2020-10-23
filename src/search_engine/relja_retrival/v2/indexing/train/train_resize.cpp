#include "train_resize.h"

#include <fstream>
#include <vector>

#include <boost/filesystem.hpp>

#ifdef RR_MPI
#include <boost/mpi/collectives.hpp>
#include <boost/serialization/utility.hpp> // for std::pair
#include <boost/serialization/string.hpp>
#endif

#include "mpi_queue.h"
#include "par_queue.h"
#include "same_random.h"
#include "timing.h"
#include "vise/vise_util.h"

namespace buildIndex {
  typedef std::pair<uint32_t, std::string> trainResizeResult;

  class trainResizeManager : public queueManager<trainResizeResult> {
  public:
    trainResizeManager(uint32_t nJobs,
                       std::string const filelist_fn,
                       std::string const filestat_fn,
                       std::vector<std::string> &resize_dst_rel_image_fn_list,
                       std::vector<std::string> &resize_src_rel_image_fn_list,
                       std::ofstream &logf,
                       vise::task_progress *progress)
      : d_job_count(nJobs),
        d_resize_dst_rel_image_fn_list(&resize_dst_rel_image_fn_list),
        d_resize_src_rel_image_fn_list(&resize_src_rel_image_fn_list),
        d_progress_print(nJobs, std::string("preprocess"), &logf),
        d_logf(&logf),
        d_progress(progress)
    {
      (*d_logf) << "preprocess:: starting resize of " << d_job_count << " images" << std::endl;
      d_filelist_f.open(filelist_fn, std::fstream::out);
      d_filestat_f.open(filestat_fn, std::fstream::out);

      ASSERT(d_filelist_f.is_open());
      ASSERT(d_filestat_f.is_open());
      d_filestat_f << "# src_filename,dst_filename,src_width,src_height,dst_width,dst_height"
                   << std::endl;
    }

    ~trainResizeManager() {
      d_filelist_f.close();
      d_filestat_f.close();
    }

    void operator()( uint32_t jobID, trainResizeResult &result );

  private:
    uint32_t d_job_count;
    std::ofstream d_filelist_f;
    std::ofstream d_filestat_f;
    std::vector<std::string> const *d_resize_dst_rel_image_fn_list;
    std::vector<std::string> const *d_resize_src_rel_image_fn_list;
    timing::progressPrint d_progress_print;
    std::ofstream *d_logf;
    vise::task_progress *d_progress;
    DISALLOW_COPY_AND_ASSIGN(trainResizeManager)
  };

  void trainResizeManager::operator()( uint32_t jobID, trainResizeResult &result ){
    std::ostringstream ss;
    if(result.first == 0) {
      // indicates success
      d_filelist_f << d_resize_dst_rel_image_fn_list->at(jobID) << std::endl;
      ss << d_resize_src_rel_image_fn_list->at(jobID) << ","
         << d_resize_dst_rel_image_fn_list->at(jobID) << "," << result.second << std::endl;
      d_filestat_f << ss.str();
    } else {
      ss << "preprocess:: DISCARD=" << d_resize_dst_rel_image_fn_list->at(jobID)
         << ", REASON=" << result.second << std::endl;
      (*d_logf) << ss.str();
    }
    if(d_progress) {
      d_progress->add(1);
    }
    d_progress_print.inc();
  }

  class trainResizeWorker : public queueWorker<trainResizeResult> {
  public:

    trainResizeWorker(std::vector<std::string> const &resize_src_image_fn_list,
                      std::vector<std::string> const &resize_dst_image_fn_list,
                      std::string resize_dimension);

    void operator() ( uint32_t jobID, trainResizeResult &result ) const;

  private:
    std::vector<std::string> const *d_resize_src_image_fn_list;
    std::vector<std::string> const *d_resize_dst_image_fn_list;
    std::string const d_resize_dimension;

    DISALLOW_COPY_AND_ASSIGN(trainResizeWorker)
  };

  trainResizeWorker::trainResizeWorker(std::vector<std::string> const &resize_src_image_fn_list,
                                       std::vector<std::string> const &resize_dst_image_fn_list,
                                       std::string resize_dimension)
    : d_resize_src_image_fn_list(&resize_src_image_fn_list),
      d_resize_dst_image_fn_list(&resize_dst_image_fn_list),
      d_resize_dimension(resize_dimension){
  }

  void trainResizeWorker::operator() ( uint32_t jobID, trainResizeResult &result ) const {
    uint32_t fn_index = jobID;
    result = std::make_pair(1, "Unknown Error");

    std::string srcfn = d_resize_src_image_fn_list->at(fn_index);
    std::string dstfn = d_resize_dst_image_fn_list->at(fn_index);

    try {
      std::ostringstream resize_stat;
      std::ostringstream geom_spec;

      bool resize_image = true;
      if((d_resize_dimension) == "-1") {
        resize_image = false;
      } else {
        geom_spec << d_resize_dimension;
      }
      geom_spec << "+0+0>";

      Magick::Image src(srcfn);
      src.quiet(true); // to supress warnings
      resize_stat << src.rows() << "," << src.columns(); // old size
      src.magick("JPEG");
      src.colorSpace(Magick::RGBColorspace);
      if (resize_image) {
        Magick::Geometry dst_img_size(geom_spec.str());
        src.resize(dst_img_size);
      }

      src.write(dstfn);
      src.quiet(false);
      resize_stat << "," << src.rows() << "," << src.columns(); // new size
      result = std::make_pair(0, resize_stat.str());
    } catch(std::exception &ex) {
      result = std::make_pair(1, ex.what());
    }
  }

  void computeTrainResize(boost::filesystem::path const image_src_dir,
                          boost::filesystem::path const image_dir,
                          boost::filesystem::path const filelist_fn,
                          boost::filesystem::path const filestat_fn,
                          std::string resize_dimension,
                          std::ofstream& logf,
                          vise::task_progress *progress) {

    MPI_GLOBAL_RANK;

    bool useThreads= detectUseThreads();
    uint32_t numWorkerThreads = vise::configuration_get_nthread();

    std::vector<std::string> resize_src_image_fn_list;
    std::vector<std::string> resize_dst_image_fn_list;
    std::vector<std::string> resize_dst_rel_image_fn_list;
    std::vector<std::string> resize_src_rel_image_fn_list;

    if (rank==0){
      std::ofstream filelist(filelist_fn.string());

      uint32_t copied_img_count = 0;
      uint32_t skipped_img_count = 0;
      uint32_t resize_img_count = 0;
      uint32_t type_conv_img_count = 0;
      boost::filesystem::recursive_directory_iterator end_itr;
      for (boost::filesystem::recursive_directory_iterator it(image_src_dir); it!=end_itr; ++it) {
        if (boost::filesystem::is_regular_file(it->path())) {
          boost::filesystem::path fn = boost::filesystem::relative(it->path(), image_src_dir).string();
          boost::filesystem::path dst_dir = image_dir / boost::filesystem::relative(it->path(), image_src_dir).parent_path();
          if (!boost::filesystem::exists(dst_dir)) {
            boost::filesystem::create_directories(dst_dir);
          }

          std::string dst_str;
          boost::filesystem::path dst = dst_dir / it->path().filename();
          std::string dst_relpath(boost::filesystem::relative(dst, image_dir).string());
          std::string src_relpath(boost::filesystem::relative(it->path(), image_src_dir).string());
#ifdef _WIN32
          // convert relative paths to Unix format so that it is compatible with HTTP requests
          // e.g. GET /pname/subdir1/image1.jpg
          std::replace(dst_relpath.begin(), dst_relpath.end(), '\\', '/');
          std::replace(src_relpath.begin(), src_relpath.end(), '\\', '/');
#endif
          if(resize_dimension == "-1") {
            // simply copy images (no preprocessing required)
            boost::system::error_code ec;
            if(!boost::filesystem::exists(dst)) {
              boost::filesystem::copy_file(it->path(), dst, boost::filesystem::copy_option::overwrite_if_exists, ec);
              if(ec) {
                logf << "preprocess:: DISCARD=" << it->path() << ", REASON=" << ec.message() << std::endl;
              } else {
                filelist << dst_relpath << std::endl;
                copied_img_count++;
              }
            } else {
              filelist << dst_relpath << std::endl;
              skipped_img_count++;
            }
            if(progress) {
              progress->add(1);
            }
          } else {
            std::string ext = it->path().extension().string();
            if(ext == ".jpg" || ext == ".jpeg" || ext == ".JPG" || ext == ".JPEG") {
              resize_src_image_fn_list.push_back(it->path().string());
              resize_dst_image_fn_list.push_back(dst.string());
              resize_dst_rel_image_fn_list.push_back(dst_relpath);
              resize_src_rel_image_fn_list.push_back(src_relpath);
            } else {
              // add ".jpg" extension because all files will get converted to JPG
              boost::filesystem::path dst = dst_dir / it->path().stem();
              dst_str = dst.string() + ".jpg";
              dst_relpath = boost::filesystem::relative(dst, image_dir).stem().string() + ".jpg";
              resize_src_image_fn_list.push_back(it->path().string());
              resize_dst_image_fn_list.push_back(dst_str);
              resize_dst_rel_image_fn_list.push_back(dst_relpath);
              resize_src_rel_image_fn_list.push_back(src_relpath);
              type_conv_img_count++;
            }

            resize_img_count++;
          }
        }
      }
      filelist.close();
      logf << "preprocess:: copied " << copied_img_count << " images, skipped "
           << skipped_img_count << " existing images" << std::endl;
      logf << "preprocess:: resizing " << resize_img_count << " images; "
           << "resize_dimension=" << resize_dimension << std::endl;
      logf << "preprocess:: converting " << type_conv_img_count
           << " images to JPEG format" << " (see " << filestat_fn << ")" << std::endl;
    }

#ifdef RR_MPI
    if (!useThreads)
      boost::mpi::broadcast(comm, imageFns, 0);
#endif
    uint32_t nJobs= resize_src_image_fn_list.size();
    if(nJobs == 0) {
      return;
    }

#ifdef RR_MPI
    if (!useThreads) comm.barrier();
#endif

    trainResizeManager *manager;
    if(rank==0) {
      manager = new trainResizeManager(nJobs,
                                       filelist_fn.string(),
                                       filestat_fn.string(),
                                       resize_dst_rel_image_fn_list,
                                       resize_src_rel_image_fn_list,
                                       logf,
                                       progress);
    } else {
      manager = nullptr;
    }

    trainResizeWorker worker(resize_src_image_fn_list,
                             resize_dst_image_fn_list,
                             resize_dimension);

    if (useThreads) {
      threadQueue<trainResizeResult>::start( nJobs, worker, *manager, numWorkerThreads );
    }
    else {
      mpiQueue<trainResizeResult>::start( nJobs, worker, manager );
    }

    if (rank==0) {
      delete manager;
    }
  }
};
