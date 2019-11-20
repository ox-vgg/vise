/*
==== Author:

Relja Arandjelovic (relja@robots.ox.ac.uk)
Visual Geometry Group,
Department of Engineering Science
University of Oxford

==== Copyright:

The library belongs to Relja Arandjelovic and the University of Oxford.
No usage or redistribution is allowed without explicit permission.
*/

#include <stdint.h>
#include <stdio.h>
#include <string>
#include <vector>

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

int main(int argc, char* argv[]){
  // MPI initialization
  MPI_INIT_ENV
  MPI_GLOBAL_ALL

  if ( argc != 6 ) {
    std::cout << "  Usage: " << argv[0] 
              << " iidx_filename fidx_filename wght_filename hamm_filename out_graph_filename"
              << std::endl;
    return 0;
  }
  
  // file names
  std::string iidxFn= util::expandUser(argv[1]);
  std::string fidxFn= util::expandUser(argv[2]);
  std::string wghtFn= util::expandUser(argv[3]);
  std::string hammFn= util::expandUser(argv[4]);
  
  // file name
  std::string imageGraphFn= util::expandUser(argv[5]);

  if ( rank == 0 ) {
    std::cout << "\nBuilding image graph (numProc=" << numProc << ") ..." << std::flush;
    std::cout << "\n  iidx = " << iidxFn;
    std::cout << "\n  fidx = " << fidxFn;
    std::cout << "\n  wght = " << wghtFn;
    std::cout << "\n  hamm = " << hammFn << std::endl;
    std::cout << "\n  graph output = " << imageGraphFn << std::endl;
  }

  // load fidx
  
  protoDbFile dbFidx_file(fidxFn);
  protoDbInRam dbFidx(dbFidx_file, rank==0);
  protoIndex fidx(dbFidx, false);
  
  // load iidx
  
  protoDbFile dbIidx_file(iidxFn);
  protoDbInRam dbIidx(dbIidx_file, rank==0);
  protoIndex iidx(dbIidx, false);
  
  // create retriever
  
  tfidfV2 tfidfObj(&iidx, &fidx, wghtFn);
  hammingEmbedderFactory embFactory(hammFn, 64);
  hamming hammingObj(tfidfObj, &iidx, embFactory, &fidx);
  spatialVerifV2 spatVerifHamm(hammingObj, &iidx, &fidx, true);
  
  imageGraph imGraph;
  // compute image graph in parallel
  imGraph.computeParallel(imageGraphFn, fidx.numIDs(), spatVerifHamm, 999, 3 );
  //imGraph.computeSingle(imageGraphFn, fidx.numIDs(), spatVerifHamm, 9999, 3 );

  // required clean up for protocol buffers
  google::protobuf::ShutdownProtobufLibrary();
  return 0;
}
