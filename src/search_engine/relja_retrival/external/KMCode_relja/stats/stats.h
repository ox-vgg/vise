#ifndef _stats_h_
#define _stats_h_

/*
Distance measures
Krystian Mikolajczyk 
04.01.2000
* vec1 - first variable 
** covMat1 - covariance matrix of the first variable 
* vec2 - second variable 
**covMat1 - covariance matrix of the second variable 
n - size of the vectors
*/
// distMahalanovbis = (ux-uy)*(1/(Lx+Ly))*(ux-uy) 

#include "../matrix/matrix.h"
#include <math.h>

using namespace std;
#include <vector>

#ifndef M_2PI
#define M_2PI 6.2831853072
#endif

class  GaussMixture{
   private:
    Matrix **covMat;//covariance matrices
    Matrix **invCovMat;//covariance matrices
    float *fact;//proportions
    float **centers;//gauss centers
    float *p;//proportions
    int K,Kp,Kc,Kv;//clusters 
    int size,sc,sv; //variabl size
    int act_k;
    float act_prob;
    float act_dist;
    float act_prob_map;
    int act_cluster;
      
 public: 
    GaussMixture(void){act_k=-1;size=0;K=0;}
    void setCovariance(Matrix *mat);
    inline void init(const int k, const int size_in);
    void setProportions(float *prop);
    void setCenters(float **centers_in);
    float probMAP(float *var);
    float densityMahal(float *var,int cluster);
    float distMahal(float *var,int cluster);
    float distMahal(float *var);
    int isOK();
    inline float getActProb(){return act_prob;}
    inline float getActProbMAP(){return act_prob_map;}
    inline int getActCluster(){return act_cluster;}
    ~GaussMixture(void);
    void loadProportions(const char *filename);
    void loadCenters(const char *filename);
    void loadCovariances(const char *filename);
    void loadMixture(const char *prop, const char *cent,const char *covar);
    
};






float distMahalanobis(float *vec1, float *vec2, Matrix *covMat);
float densityMahalanobis(float *vec1, float *vec2, Matrix *covMat);
float distHistogram(float* histos1,float* histos2,int taillehisto);
float distEuc(float* vec1,float* vec2,int size);
float distChi(float* histos1,float* histos2,int taillehisto);
float distCSift(float* vec1,float* vec2,int size);


void sort(vector<float> &vec);
float variance(vector<float> &vec, float center);
float stdev(vector<float> &vec, float center);
float mean(vector<float> &vec);
float median(vector<float> &vec, float thres);
float median(vector<float> &vec);
float max(vector<float> &vec);
float min(vector<float> &vec);
float gamma(float n);
float chi2(float x, float n);
float thresChi2(float prob, float n);
float probChi2(float thres, float n);
float round(float x, int n);
vector<float> hist(vector<float> &vec,float min, float max, int bins);

#endif





