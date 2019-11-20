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

#include "train_hamming.h"

#include <fstream>
#include <vector>

#include <boost/filesystem.hpp>

#ifdef RR_MPI
#include <boost/mpi/collectives.hpp>
#endif

#include <Eigen/SVD>

#include "clst_centres.h"
#include "flat_desc_file.h"
#include "hamming_data.pb.h"
#include "median_computer.h"
#include "mpi_queue.h"
#include "par_queue.h"
#include "protobuf_util.h"
#include "same_random.h"
#include "timing.h"




namespace buildIndex {


typedef std::vector<float> trainHammingResult; // medians after cluster centres were subtracted



class trainHammingManager : public managerWithTiming<trainHammingResult> {
    public:

        trainHammingManager(uint32_t const nJobs,
                            std::string const trainHammFn,
                            std::vector<float> const &rot,
                            uint32_t const numDims,
                            uint32_t const numClst,
                            uint32_t const vocChunkSize)
                            : managerWithTiming<trainHammingResult>(nJobs, "trainHammingManager"),
                              trainHammFn_(trainHammFn),
                              vocChunkSize_(vocChunkSize),
                              hammEmbBits_(rot.size() / numDims),
                              rot_(&rot) {
                ASSERT(rot.size() % numDims==0);

                hamm_.set_k(numClst);
                hamm_.set_numdims(numDims);
                hamm_.set_numbits(hammEmbBits_);

                // resize the array to its final size
                median_.resize(numClst*hammEmbBits_, 0);
            }

        ~trainHammingManager();

        void
            compute( uint32_t jobID, trainHammingResult &result );

    private:
        std::string trainHammFn_;
        uint32_t const vocChunkSize_, hammEmbBits_;
        rr::hammingData hamm_;
        std::vector<float> const *rot_;
        std::vector<float> median_;
        DISALLOW_COPY_AND_ASSIGN(trainHammingManager)
};



trainHammingManager::~trainHammingManager(){
    // organization:
    // magicNumber, sizeof(header), header, rotation, medians
    std::string headerStr;
    hamm_.SerializeToString(&headerStr);
    uint32_t const magicNumber= 0xF1234987;
    uint32_t const headerSize= headerStr.length();

    FILE *f= fopen(trainHammFn_.c_str(), "wb");

    fwrite( &(magicNumber), sizeof(uint32_t), 1, f );
    fwrite( &(headerSize), sizeof(uint32_t), 1, f );
    fwrite( &(headerStr[0]), sizeof(char), headerSize, f );
    fwrite( &((*rot_)[0]), sizeof(float), rot_->size(), f );
    fwrite( &median_[0], sizeof(float), median_.size(), f );

    fclose(f);
}



void
trainHammingManager::compute( uint32_t jobID, trainHammingResult &result ){
    // make sure results are saved sorted by job!
    uint32_t wordStart= jobID*vocChunkSize_;
    uint32_t const wordEnd= std::min( (jobID+1)*vocChunkSize_, hamm_.k() );
    ASSERT( (wordEnd-wordStart)*hammEmbBits_ == result.size());
    std::memcpy( &median_[wordStart*hammEmbBits_],
                 &result[0],
                 result.size() * sizeof(float) );
}



class trainHammingWorker : public queueWorker<trainHammingResult> {
    public:
        trainHammingWorker(std::string const trainDescsFn,
                           std::string const trainAssignsFn,
                           bool const RootSIFT,
                           std::vector<float> const &rot,
                           clstCentres const &clstCentres_obj,
                           uint32_t const vocChunkSize)
                           : descFile_(trainDescsFn, RootSIFT),
                             hammEmbBits_(rot.size() / descFile_.numDims()),
                             numClst_(clstCentres_obj.numClst),
                             vocChunkSize_(vocChunkSize),
                             numDims_(descFile_.numDims()),
                             numDescs_(descFile_.numDescs()),
                             rot_(&rot),
                             clstCentres_obj_(&clstCentres_obj){
                  ASSERT(clstCentres_obj_->numDims==numDims_);
                  ASSERT(rot.size() % numDims_==0);
                  f_= fopen(trainAssignsFn.c_str(), "rb");
                  fd_= fileno(f_);
            }

        ~trainHammingWorker(){ fclose(f_); }

        void
            operator() ( uint32_t jobID, trainHammingResult &result ) const;

    private:
        flatDescsFile const descFile_;
        FILE *f_;
        int fd_;
        uint32_t const hammEmbBits_, numClst_, vocChunkSize_, numDims_, numDescs_;
        std::vector<float> const *rot_;
        clstCentres const *clstCentres_obj_;

        DISALLOW_COPY_AND_ASSIGN(trainHammingWorker)
};



void
trainHammingWorker::operator() ( uint32_t jobID, trainHammingResult &result ) const {

    result.clear();

    uint32_t wordStart= jobID*vocChunkSize_;
    uint32_t const wordEnd= std::min( (jobID+1)*vocChunkSize_, numClst_ );
    std::vector<medianComputer> medianComp( (wordEnd-wordStart)*hammEmbBits_ );

    uint32_t const descChunkSize= 1000;
    uint32_t clusterIDs[descChunkSize];

    for (uint32_t iDescStart= 0; iDescStart<numDescs_; ){
        uint32_t iDescEnd= std::min(iDescStart + descChunkSize, numDescs_);

        // descriptors
        float *descs;
        descFile_.getDescs(iDescStart, iDescEnd, descs);
        // clusters
        ASSERT( pread64(fd_, clusterIDs,
                       (iDescEnd-iDescStart)*sizeof(uint32_t),
                       static_cast<uint64_t>(iDescStart)*sizeof(uint32_t)) > 0 );

        uint32_t const count= iDescEnd-iDescStart;
        float *itDesc= descs;
        uint32_t *itClusterID= clusterIDs;

        for (uint32_t iDesc= 0; iDesc<count; ++iDesc, ++itClusterID){
            // process only the descriptors with cluster inside [wordStart,wordEnd)
            if (*itClusterID < wordStart || *itClusterID >= wordEnd){
                itDesc+= numDims_;
                continue;
            }

            float * const thisDesc= itDesc;

            // subtract cluster centres
            float const *itC= clstCentres_obj_->clstC_flat + (*itClusterID) * numDims_;
            for (uint32_t iDim= 0; iDim<numDims_; ++iDim, ++itDesc, ++itC)
                *itDesc-= *itC;

            // rotate and add to median computer for every projection dimension
            medianComputer *itMC= &medianComp[0] + (*itClusterID-wordStart) * hammEmbBits_;
            float projection;
            float const *itDescEnd= thisDesc+numDims_;
            float const *itDescC;
            float const *itRot= &((*rot_)[0]);

            for (uint32_t iDim= 0; iDim < hammEmbBits_; ++iDim, ++itMC){
                projection= 0.0f;
                for (itDescC= thisDesc; itDescC!=itDescEnd; ++itDescC, ++itRot)
                    projection+= *itDescC * (*itRot);

                itMC->add(projection);
            }
        }

        delete []descs;

        iDescStart= iDescEnd;
    }

    result.clear();
    result.reserve( (wordEnd-wordStart)*hammEmbBits_ );
    for (medianComputer *itMC= &medianComp[0]; wordStart<wordEnd; ++wordStart){
        for (uint32_t iDim= 0; iDim < hammEmbBits_; ++iDim, ++itMC)
            result.push_back( itMC->getMedian() );
    }

}



void
computeHamming(
        std::string const clstFn,
        bool const RootSIFT,
        std::string const trainDescsFn,
        std::string const trainAssignsFn,
        std::string const trainHammFn,
        uint32_t const hammEmbBits){

    MPI_GLOBAL_ALL;

    if (boost::filesystem::exists(trainHammFn)){
        if (rank==0)
            std::cout<<"buildIndex::computeHamming: trainHammFn already exist ("<<trainHammFn<<")\n";
        return;
    }
    ASSERT( boost::filesystem::exists(trainDescsFn) );
    ASSERT( boost::filesystem::exists(trainAssignsFn) );
    ASSERT( hammEmbBits<=64 );

    // clusters
    if (rank==0)
        std::cout<<"buildIndex::computeHamming: Loading cluster centres\n";
    double t0= timing::tic();
    clstCentres const clstCentres_obj( clstFn.c_str(), true );
    uint32_t numClst= clstCentres_obj.numClst;
    uint32_t numDims= clstCentres_obj.numDims;
    if (rank==0)
        std::cout<<"buildIndex::computeHamming: Loading cluster centres - DONE ("<< timing::toc(t0) <<" ms)\n";
    ASSERT(numDims>=hammEmbBits);

    bool useThreads= detectUseThreads();
    uint32_t numWorkerThreads= omp_get_max_threads();

    // rotation
    std::vector<float> rot;

    if (rank==0){
        #if 1
        // Find the rotation (projection) of the descriptor residuals
        // Original paper does random rotation
        // Better to do PCA to get the top hammEmbBits directions
        // Then do random rotation only on top of this, in order to balance the variances
        // e.g. see the Iterative Quantization paper (this would be even better but too complicated so an overkill)

        // --- do PCA of input descriptors (well, residuals as this is roughly 0-centred) to pick

        // descriptors & assigned clusters
        flatDescsFile descFile(trainDescsFn, RootSIFT);
        uint32_t const numTrainDescsTotal= descFile.numDescs();
        uint32_t const numDims= descFile.numDims();
        FILE *f= fopen(trainAssignsFn.c_str(), "rb");
        ASSERT(f!=NULL);
        int fd= fileno(f);
        uint32_t numTrainDescsPCA= std::min( numTrainDescsTotal, static_cast<uint32_t>(100000) );
        uint32_t const blockSize= std::min(numTrainDescsPCA, static_cast<uint32_t>(1000));
        numTrainDescsPCA-= numTrainDescsPCA%blockSize;
        uint32_t const blockStep= numTrainDescsTotal / (numTrainDescsPCA/blockSize);

        float *descs= new float[numTrainDescsPCA*numDims];
        float *itDesc= descs;
        float const *descsEnd= descs + numTrainDescsPCA*numDims;
        uint32_t *clusterIDs= new uint32_t[blockSize];
        std::cout<<"buildIndex::computeHamming: Reading training "<<numTrainDescsPCA<<" descriptors for PCA\n";

        for (uint32_t iDescStart= 0; itDesc!=descsEnd; iDescStart+= blockStep){
            float *thisBlockDescs;
            // descriptors
            descFile.getDescs(iDescStart, iDescStart+blockSize, thisBlockDescs);
            float const *thisDescIt= thisBlockDescs;
            // clusters
            ASSERT( pread64(fd, clusterIDs,
                            blockSize*sizeof(uint32_t),
                            static_cast<uint64_t>(iDescStart)*sizeof(uint32_t)) > 0 );
            // subtract cluster centres
            uint32_t const *endClstID= clusterIDs + blockSize;
            for (uint32_t const *itClusterID= clusterIDs; itClusterID!=endClstID; ++itClusterID){
                float const *itC= clstCentres_obj.clstC_flat + (*itClusterID) * numDims;
                for (uint32_t iDim= 0; iDim<numDims; ++iDim, ++itDesc, ++itC, ++thisDescIt)
                    *itDesc= *thisDescIt - *itC;
            }
            delete []thisBlockDescs;
        }
        fclose(f);
        delete []clusterIDs;

        double t0= timing::tic();
        std::cout<<"buildIndex::computeHamming: Computing PCA\n";
        Eigen::Map<Eigen::MatrixXf> trainData(descs, numDims, numTrainDescsPCA);
        Eigen::JacobiSVD<Eigen::MatrixXf> svdForPCA(trainData, Eigen::ComputeThinU);
        // doing SVD of trainData is like eig(trainData*trainData'), leftmost columns of U are largest eigenvectors
        Eigen::MatrixXf PCA= svdForPCA.matrixU().block(0,0, numDims,hammEmbBits);
        ASSERT(PCA.rows()==numDims && PCA.cols()==hammEmbBits);
        delete []descs;

        // --- get the random rotation (hammEmbBits x hammEmbBits)
        std::cout<<"buildIndex::computeHamming: Computing random rotation\n";

        // make random matrix
        sameRandomUint32 sr(hammEmbBits * hammEmbBits, 43);
        sameRandomStreamUint32 srS(sr);
        Eigen::MatrixXf randMatrix(hammEmbBits, hammEmbBits);
        for (uint32_t i= 0; i<hammEmbBits; ++i)
            for (uint32_t j= 0; j<hammEmbBits; ++j)
                randMatrix(i,j)= srS.getNextFloat();

        // get random orthonormal matrix by doing SVD
        Eigen::JacobiSVD<Eigen::MatrixXf> svdForRR(randMatrix, Eigen::ComputeFullU);
        Eigen::MatrixXf R= svdForRR.matrixU();
        ASSERT(R.cols()==hammEmbBits && R.rows()==hammEmbBits);
        // check stuff orthogonal just in case, but there is no way it isn't
        for (uint32_t i= 0; i<hammEmbBits; ++i)
            ASSERT( fabs( R.row(i).norm() -1)<1e-4 );

        // --- finally, compute the final rotation via R*PCA'
        R*= PCA.transpose();

        #else

        // just do random rotation
        double t0= timing::tic();

        // --- get the random rotation (hammEmbBits x numDims)
        std::cout<<"buildIndex::computeHamming: Computing random rotation\n";

        // make random matrix
        sameRandomUint32 sr(numDims * numDims, 43);
        sameRandomStreamUint32 srS(sr);
        Eigen::MatrixXf randMatrix(numDims, numDims);
        for (uint32_t i= 0; i<numDims; ++i)
            for (uint32_t j= 0; j<numDims; ++j)
                randMatrix(i,j)= srS.getNextFloat();

        // get random orthonormal matrix by doing SVD
        Eigen::JacobiSVD<Eigen::MatrixXf> svdForRR(randMatrix, Eigen::ComputeFullU);
        Eigen::MatrixXf R= svdForRR.matrixU().block(0,0, hammEmbBits,numDims);
        ASSERT(R.cols()==numDims && R.rows()==hammEmbBits);
        // check stuff orthogonal just in case, but there is no way it isn't
        for (uint32_t i= 0; i<hammEmbBits; ++i)
            ASSERT( fabs( R.row(i).norm() -1)<1e-4 );

        #endif

        // copy the rotation matrix into vector
        rot.reserve(hammEmbBits*numDims);
        for (uint32_t i= 0; i<hammEmbBits; ++i)
            for (uint32_t j= 0; j<numDims; ++j)
                rot.push_back(R(i,j));

        std::cout<<"buildIndex::computeHamming: Done with rotation ("<< timing::toc(t0) <<" ms)\n";
    }

    #ifdef RR_MPI
    // communicate the rotation to everyone
    if (!useThreads)
        boost::mpi::broadcast(comm, rot, 0);
    #endif

    // Parallelization is done a bit differently than normally, due to memory:
    // Each worker will process a range of visual words to find the medians
    uint32_t const vocChunkSize=
        std::min( static_cast<uint32_t>(5000),
                  static_cast<uint32_t>(
                      std::ceil(static_cast<double>(numClst)/std::max(numWorkerThreads, numProc))) );
    uint32_t const nJobs= static_cast<uint32_t>( std::ceil(static_cast<double>(numClst)/vocChunkSize) );

    // compute hamming stuff

    #ifdef RR_MPI
    if (!useThreads) comm.barrier();
    #endif

    trainHammingManager *manager= (rank==0) ?
        new trainHammingManager(nJobs, trainHammFn, rot, numDims, numClst, vocChunkSize) :
        NULL;

    trainHammingWorker worker(trainDescsFn, trainAssignsFn,
                              RootSIFT,
                              rot, clstCentres_obj, vocChunkSize);

    if (useThreads)
        threadQueue<trainHammingResult>::start( nJobs, worker, *manager, numWorkerThreads );
    else
        mpiQueue<trainHammingResult>::start( nJobs, worker, manager );

    if (rank==0) delete manager;

}

};
