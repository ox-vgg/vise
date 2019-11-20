/*
Image graph is a graph with vertices corresponding to images in the dataset and
edges corresponding to the target vertex being in the search result using source
vertex as query. Such an image graph can be produced using
"src/search_engine/relja_retrival/v2/retrieval/create_image_graph.cpp"

Groups of images containing similar patterns correspond to strongly connected
components of this image graph and is computed by this code. These groups are
exported in a VGG Image Annotator (VIA) project file format for visualization
and further annotation. VIA is an open source manual image annotator and can
be obtained from:
http://www.robots.ox.ac.uk/~vgg/software/via

Author: Abhishek Dutta <adutta@robots.ox.ac.uk>
Date: 5 Dec. 2018
*/

#include <stdint.h>
#include <stdio.h>
#include <string>
#include <vector>
#include <fstream>
#include <stack> // for DFS in graph
#include <sstream>
#include <algorithm>

// for filesystem i/o
#include <boost/filesystem.hpp>

// to compute connected components
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/connected_components.hpp>
#include <boost/graph/strong_components.hpp>

#include "dataset_v2.h"
#include "hamming.h"
#include "hamming_embedder.h"
#include "mpi_queue.h"
#include "image_graph.h"
#include "proto_db.h"
#include "proto_db_file.h"
#include "proto_index.h"
#include "spatial_verif_v2.h"
#include "tfidf_v2.h"
#include "util.h"

std::string get_via_group_annotation_json_str(std::string filename, uint32_t group_id) {
  std::ostringstream ss;
  ss << "\"" << filename << "-1\":{"
     << "\"filename\":\"" << filename << "\","
     << "\"size\":-1,"
     << "\"regions\":[],"
     << "\"file_attributes\":"
     << "{\"group_id\":\"" << group_id << "\","
     << "\"is_good\":\"1\","
     << "\"group_description\":\"\"}}";
  return ss.str();
}

std::string get_via_empty_annotation_json_str(std::string filename) {
  std::ostringstream ss;
  ss << "\"" << filename << "-1\":{"
     << "\"filename\":\"" << filename << "\","
     << "\"size\":-1,"
     << "\"regions\":[],"
     << "\"file_attributes\":"
     << "{\"group_id\":\"\","
     << "\"is_good\":\"\","
     << "\"group_description\":\"\"}}";
  return ss.str();
}

std::string get_via_settings_json_str(std::string project_name, std::string img_path) {
  std::ostringstream ss;
  ss << "\"_via_settings\":{"
     << "\"ui\":{\"annotation_editor_height\":25,\"annotation_editor_fontsize\":0.8,\"leftsidebar_width\":18,\"image_grid\":{\"img_height\":80,\"rshape_fill\":\"none\",\"rshape_fill_opacity\":0.3,\"rshape_stroke\":\"yellow\",\"rshape_stroke_width\":2,\"show_region_shape\":true,\"show_image_policy\":\"all\"},\"image\":{\"region_label\":\"__via_region_id__\",\"region_label_font\":\"10px Sans\",\"on_image_annotation_editor_placement\":\"NEAR_REGION\"}},"
     << "\"core\":{\"buffer_size\":18,\"filepath\":{},\"default_filepath\":\"" << img_path << "/\"},"
     << "\"project\":{\"name\":\"" << project_name << "\"}"
     << "}";
  return ss.str();
}

std::string get_via_metadata_json_str() {
  std::ostringstream ss;
  ss << "\"_via_attributes\":{"
     << "\"file\":{\"group_id\":{\"type\":\"text\"},\"is_good\":{\"type\":\"radio\",\"description\":\"\",\"options\":{\"1\":\"Yes\",\"0\":\"No\"}},\"group_description\":{\"type\":\"text\"}},"
     << "\"region\":{}}";
  return ss.str();
}

void export_via_project( std::vector<uint32_t>& gid_list,
                         std::vector<std::string>& fn_list,
                         std::string project_name,
                         boost::filesystem::path image_dir,
                         boost::filesystem::path via_project_fn
                         ) {
  std::ofstream cc( via_project_fn.string() );
  cc << "{";
  cc << get_via_settings_json_str( project_name, image_dir.string() );
  cc << "," << get_via_metadata_json_str();
  cc << ",\"_via_img_metadata\":{";
  bool first_entry = true;
  for ( std::size_t fid = 0; fid < gid_list.size(); ++fid ) {
    if ( first_entry ) {
      first_entry = false;
    } else {
      cc << ",";
    }

    // Mike Allaway updated the image file locations
    // 000010010000050_0.png -> 000/000010010000050_0.png
    std::string img_rel_path = fn_list[fid].substr(0,3) + "/" + fn_list[fid];
    cc << get_via_group_annotation_json_str( img_rel_path, gid_list[fid] );
  }
  cc << "}"; // end of _via_img_metadata
  cc << "}"; // end of JSON
  cc.close();
}

int main(int argc, char* argv[]){
  if ( argc != 6 ) {
    std::cout << "  Usage: " << argv[0]
              << " dset_filename image_dir image_graph_filename score_threshold out_dir"
              << std::endl;
    return 0;
  }

  // file names
  boost::filesystem::path dset_filename( argv[1] );
  boost::filesystem::path image_dir( argv[2] );
  boost::filesystem::path image_graph_filename( argv[3] );
  double SCORE_THRESHOLD;
  std::stringstream ss1;
  ss1 << argv[4];
  ss1 >> SCORE_THRESHOLD;
  boost::filesystem::path outdir( argv[5] );

  std::cout << "Loading graph data form file [" << image_graph_filename << "]" << std::endl;
  protoDbFile graphdata(image_graph_filename.string());
  protoIndex graphdata_index(graphdata, false);
  std::size_t nvertex = graphdata_index.numIDs();

  //// create a boost graph data structure
  std::cout << "Creating boost graph with [" << nvertex << "] vertices "
            << "(SCORE_THRESHOLD=" << SCORE_THRESHOLD << ") ... " << std::flush;
  //typedef boost::adjacency_list <boost::vecS, boost::vecS, boost::undirectedS> Graph;
  typedef boost::adjacency_list <boost::vecS, boost::vecS, boost::directedS> Graph;
  Graph G(nvertex);

  for ( uint32_t fid = 0; fid < graphdata_index.numIDs(); ++fid ) {
    std::vector<rr::indexEntry> entries;
    graphdata_index.getEntries(fid, entries);
    if ( entries.size() == 1 ) {
      rr::indexEntry const &entry = entries[0];
      int nbd = entry.id_size();
      for ( int i = 0; i < nbd; ++i ) {
        if ( entry.weight(i) >= SCORE_THRESHOLD ) {
          boost::add_edge(fid, entry.id(i), G);
        }
      }
    }
  }
  std::cout << "done" << std::endl;

  typedef boost::graph_traits< boost::adjacency_list<boost::vecS, boost::vecS, boost::directedS> >::vertex_descriptor Vertex;

  std::vector<uint32_t> cluster(boost::num_vertices(G)), discover_time(boost::num_vertices(G));
  std::vector<boost::default_color_type> color(boost::num_vertices(G));
  std::vector<Vertex> root(boost::num_vertices(G));
  uint32_t cluster_count;
  cluster_count = strong_components(G, boost::make_iterator_property_map(cluster.begin(), boost::get(boost::vertex_index, G)),
                                    root_map(boost::make_iterator_property_map(root.begin(), boost::get(boost::vertex_index, G))).
                                    color_map(boost::make_iterator_property_map(color.begin(), boost::get(boost::vertex_index, G))).
                                    discover_time_map(boost::make_iterator_property_map(discover_time.begin(), boost::get(boost::vertex_index, G))));
  std::cout << "Total number of strongly connected components (or clusters): " << cluster_count << std::endl;

  std::cout << "Building histogram of connected components ..." << std::endl;
  std::vector<uint32_t> cluster_file_count(cluster_count, 0);
  std::vector<uint32_t> cluster_id(cluster_count, 0);
  std::vector<uint32_t> sorted_cluster_id(cluster_count, 0);
  for ( uint32_t i = 0; i < nvertex; ++i ) {
    cluster_file_count[ cluster[i] ] = cluster_file_count[ cluster[i] ] + 1;
  }

  /*
  std::cout << "Writing file count of each cluster to file " << hist_filename << std::endl;
  std::ofstream hist( hist_filename.string() );
  hist << "group_id,group_file_count" << std::endl;

  // initialize original cluster id for sorting
  for ( uint32_t i = 0; i < cluster_count; ++i ) {
    cluster_id[i] = i;
    sorted_cluster_id[i] = i;
    hist << i << "," << cluster_file_count[i] << std::endl;
  }
  hist.close();
  */

  //// load dataset (needed to convert file id into filename)
  // dataset_->getNumDoc();
  // dataset_->getInternalFn(file_id);
  // dataset_->getDocID(filename);
  // construct dataset
  std::cout << "Loading dataset from file [" << dset_filename << "]" << std::endl;
  datasetV2 *dset = new datasetV2( dset_filename.string(), image_dir.string() + "/" );

  // obtained after looking at the histogram of cluster_file_count of FLEURON
  std::vector<uint32_t> file_count_breaks = {2, 3, 4, 5, 10, 20, 50, 100, 5000};
  int group_id = 1;
  boost::filesystem::path group_index_fn = outdir / "group_index.txt";
  std::ofstream group_index_f( group_index_fn.string() );
  group_index_f << "group_id,subgroup_count,filecount_range_start,filecount_range_end" << std::endl;
  for ( std::size_t i = 0; i < file_count_breaks.size() - 1; ++i ) {
    std::vector<uint32_t> subgroup_gid_list;
    std::vector<std::string> subgroup_fn_list;

    uint32_t start = file_count_breaks[i];
    uint32_t end   = file_count_breaks[i+1];
    uint32_t gid, gcount;
    std::set<uint32_t> subgroup_gid_set;

    // collect list of all fid whose groups have file_count in [start,end)
    for ( uint32_t fid = 0; fid < nvertex; ++fid ) {
      gid = cluster[fid];
      gcount = cluster_file_count[gid];
      if ( gcount >= start && gcount < end ) {
        subgroup_gid_list.push_back(gid);
        subgroup_gid_set.insert(gid);
        subgroup_fn_list.push_back( dset->getInternalFn(fid) );
      }
    }

    // write these files and their corresponding group_id as a via project
    std::ostringstream project_name;
    project_name << "fleuron_cluster_set_" << group_id << ".json";
    boost::filesystem::path via_project_fn = outdir / project_name.str();

    std::cout << "Writing via project for group file count size in range ["
              << start << "," << end << ") ...";
    export_via_project( subgroup_gid_list, subgroup_fn_list, project_name.str(),
                        image_dir, via_project_fn);
    std::cout << " done" << std::endl;

    group_index_f << group_id << "," << subgroup_gid_set.size()
                  << "," << start << "," << end << std::endl;
    group_id = group_id + 1;
  }

  group_index_f.close();

  // required clean up for protocol buffers
  google::protobuf::ShutdownProtobufLibrary();
  return 0;
}
