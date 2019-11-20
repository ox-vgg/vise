/*
Distance measures
Krystian Mikolajczyk 
04.01.2000
* vec1 - first variable 
** covMat - covariance matrix of the variable 
* vec2 - second variable 
*/
// distMahalanovbis = (ux-uy)*(1/(Lx+Ly))*(ux-uy) 
#include "stats.h"

void GaussMixture::setCovariance(Matrix *mat_in){
    Matrix *mat=new Matrix();
    (*mat)=(*mat_in);
    covMat[act_k++]=mat;    
}
void GaussMixture::setProportions(float *prop){
    for(int i=0;i<K;i++)
	p[i]=prop[i];	
}

void GaussMixture::setCenters(float **centers_in){
    for(int j=0;j<K;j++) 
	for(int i=0;i<size;i++) 
	    centers[j][i]=centers_in[j][i];
}

void GaussMixture::init(const int k, const int size_in){
    if(k<=0||size_in<=0)return;
    K=k;
    size=size_in;
    p=new float[K];
    covMat = new Matrix*[K];
    centers= new float*[K];
    centers[0]= new float[K*size];
    for(int i=1;i<K;i++)centers[i]=centers[0]+i*size;	    
    act_k=0;
}

int GaussMixture::isOK(){
    if(act_k==K)return 1;
    else return 0;
}

void GaussMixture::loadProportions(const char *filename){
    Matrix *props = new Matrix(filename);
    Kp=props->nbCols();
    p=new float[Kp];
    for(int i=0;i<Kp;i++)
      p[i]=props->tabMat[1][i+1];
}

void GaussMixture::loadCenters(const char *filename){
    Matrix *cent=new Matrix(filename);
    Kc=cent->nbCols();
    sc=cent->nbRows();
    centers= new float*[Kc];
    centers[0]= new float[Kc*sc];
    for(int i=1;i<Kc;i++)centers[i]=centers[0]+i*sc;
	    
    for(int j=0;j<Kc;j++){
	for(int i=0;i<sc;i++){
	    centers[j][i]=cent->tabMat[i+1][j+1];
	}	
    }
    delete cent;
}

void GaussMixture::loadCovariances(const char *filename){
    Matrix *cov=new Matrix(filename);
    sv=cov->nbCols();
    Kv=(int)((cov->nbRows())/sv);
    covMat = new Matrix*[Kv];
    invCovMat = new Matrix*[Kv];
    int row=0;
    for(int j=0;j<Kv;j++){
      covMat[j]=new Matrix(sv,sv);
      for(int i=1;i<=sv;i++){	    
	row++;
	for(int l=1;l<=sv;l++){
	  covMat[j]->tabMat[i][l]=cov->tabMat[row][l];	
	}
      }
      invCovMat[j]=new Matrix(covMat[j]->inverse());
      //cout << (*invCovMat[j])<< endl;
    }    
    fact=new float[Kv];
    for(int j=0;j<Kv;j++){fact[j]=(1/(sqrt(pow((double)(2*M_PI),(double)size)*(covMat[j]->determinant()))));    
    }
   
}
float GaussMixture::probMAP(float *var){
    float *prob=new float[K];
    for(int i=0;i<K;i++){
	prob[i]=(p[i]*densityMahal(var,i));
    }
    float sum=0;
    for(int i=0;i<K;i++)sum+=prob[i];
    float max=0,tmp=0;
    for(int i=0;i<K;i++){
	tmp=prob[i]/sum;
	if(tmp>max){
	  max=tmp;
	  act_cluster=i;
	}
    }
    act_prob=prob[act_cluster];
    act_prob_map=max;
    return act_prob;
}

float GaussMixture::densityMahal(float *var,int cluster){
  return exp(-0.5*(distMahal(var, cluster)))*fact[cluster];
}
float GaussMixture::distMahal(float *var){
    float *dist=new float[K];
    for(int i=0;i<K;i++){
	dist[i]=(distMahal(var,i));
    }
    float min=10000;
    for(int i=0;i<K;i++){
      if(dist[i]<min){
	min=dist[i];
	act_cluster=i;
      }
    }
    act_dist=min;
    return min;
}
float GaussMixture::distMahal(float *var,int cluster){      
   float *A = new float[size+1];
   float *B = new float[size+1];
   for(int a=0;a<size;a++){
     A[a+1]=var[a]-centers[cluster][a];
   }    
      
   for(int row=1;row<=size;row++){
     B[row]=0;
     for(int col=1;col<=size;col++){
       B[row]+=invCovMat[cluster]->tabMat[row][col]*A[col];
     }
   }   
   float dist=0;
   for(int col=1;col<=size;col++){
     dist+=A[col]*B[col];
   }
   delete A; delete B;
   
   return (dist);    
}

void GaussMixture::loadMixture(const char *prop, const char *cent,
			       const char *covar){
    loadProportions(prop);
    loadCenters(cent);
    loadCovariances(covar);
    if(Kp!=Kc || Kc!=Kv || sc!=sv)act_k=-1;
    else {
	K=Kp;
	size=sc;
	act_k=K;
    }
}



float densityMahalanobis(float *vec1,  float *mu_vec2, Matrix *covMat){
    float dens=0;
    float size = (float)covMat->nbCols();
    Matrix *invMat =  new Matrix(covMat->inverse());
    dens=exp(-0.5*(distMahalanobis(vec1, mu_vec2, invMat)))*
	(1/(sqrt(pow((double)2*M_PI,(double)size)*(covMat->determinant()))));
    delete invMat;
    return dens;//+(*covMat2)
}

float distMahalanobis(float *vec1, float *vec2, Matrix *covMat){
   float dist=0;
   
   int n = covMat->nbCols();
    
  
   //     cout << "dOK1 " << endl;
   // A=(ux-uy)
   //Matrix A(n,1);
   
   float *A = new float[n+1];
   float *B = new float[n+1];
   for(int a=0;a<n;a++){
     //cout << vec1[a] << " " << vec2[a]<< endl;
     A[a+1]=vec1[a]-vec2[a];
   }    
   //getchar();
   // Sin = 1/CovMat
   //Matrix *Sin=new Matrix(covMat->inverse());
   
   for(int row=1;row<=n;row++){
     B[row]=0;
     for(int col=1;col<=n;col++){
       B[row]+=covMat->tabMat[row][col]*A[col];
     }
   }
   
   for(int col=1;col<=n;col++){
     dist+=A[col]*B[col];
   }
   delete A; delete B;//delete Sin;
   
   return (dist);    
} 


float distEuc(float* vec1,float* vec2,int size){
    float dist=0;
    for(int i=0;i<size;i++){
	dist+=((vec1[i]-vec2[i])*(vec1[i]-vec2[i]));
    }
    return dist;
}
inline float square(float a){
  return a*a;
}
float distCSift(float* vec1,float* vec2, int size){
   float dist=0,tmp;
   float adist=0;
   for(int i=0;i<size;i+=2){
    adist=fabs(vec1[i+1]-vec2[i+1]);
    adist=(adist<M_PI)?adist:(M_2PI-fabs(vec1[i+1]-vec2[i+1]));
    adist/=M_PI;adist=1-adist;//adist=0.5;
    if(adist>1 || adist<0)cout << endl << "angle error in distCSift" << endl;
    tmp=((square(vec1[i]-adist*vec2[i])+square(adist*vec1[i]-vec2[i]))/(1+square(adist)));
    dist+=tmp;
    //cout << "o1 "<< vec1[i+1]<< " o2 " <<vec2[i+1] << "v1 "<< vec1[i]<< " v2 " <<vec2[i] << " adist " << adist<< endl;
   }
   //cout << "dist " << dist<< endl;getchar();
  return dist;
}

float distCSift1(float* vec1,float* vec2, int size){
   float dist=0, tmp=0;
   float adist=0;
   for(int i=0;i<size;i+=2){
    adist=fabs(vec1[i+1]-vec2[i+1]);
     adist=(adist<M_PI)?adist:(M_2PI-fabs(vec1[i+1]-vec2[i+1]));
    adist/=M_PI;adist=1-adist;
    if(adist>1 || adist<0)cout << endl << "angle error in distCSift" << endl;
    if(adist>0.8)tmp=square(vec1[i]-vec2[i]); 
    else tmp=(square(vec1[i])+square(vec2[i]));
    dist+=tmp;
  }
  return dist;
}


float distHistogram(float* histos1,float* histos2,int taillehisto){

  float distance;
 
  distance = 0.0;
 
  for(int i = 0 ; i < taillehisto ; ++i)
    {
      //  cerr << " histos1[i] " << histos1[i] << " histos2[i] " << histos2[i] << endl;
 
      distance += (int)fabs(histos1[i] - histos2[i]);
    }
 
  return distance;
}

float distChi(float* histos1,float* histos2,int taillehisto){

  float somme;
  float distance;
 
  distance = 0.0;
 
  for(int i = 0 ; i < taillehisto ; ++i)
    {
      //  cerr << " histos1[i] " << histos1[i] << " histos2[i] " << histos2[i] << endl;
 
      somme = (histos1[i] + histos2[i])*(histos1[i] + histos2[i]);
 
      if(somme)
        distance += ((histos1[i] - histos2[i])*(histos1[i] - histos2[i]))/somme;
    }
 
  return distance;
}

float mean(vector<float> &vec){
  float mean=0;
  for(int i=0;i<(int)vec.size();i++)mean+=vec[i];
  return (mean/vec.size());
  
}

float max(vector<float> &vec){
  float max=vec[0];
  for(int i=1;i<(int)vec.size();i++){
    if(max<vec[i])max=vec[i];
  }
  return (max); 
}
float min(vector<float> &vec){
  float min=vec[0];
  for(int i=1;i<(int)vec.size();i++){
    if(min>vec[i])min=vec[i];
  }
  return (min); 
}

float median2(vector<float> &vec, float thres){
  
  float med=thres*vec.size();
  float fl=vec[(int)floor(med)];
  float cl=vec[(int)ceil(med)];
  float median= fl+(cl-fl)*(med-floor(med));
  return median;
}

float stdev(vector<float> &vec, float center){
  return sqrt(variance(vec,center));
}

float variance(vector<float> &vec, float center){
  float  var=0;
  float  tmp=0;
  for(int i=0;i<(int)vec.size();i++){
    tmp=(vec[i]-center);
    var+=(tmp*tmp);
  }  
  
  return (var/(vec.size()-1));
}

void sort(vector<float> &vec){
  float min;
  int imin;
  vector<float> vec_out;
  do{
    min=vec[0];imin=0;
    for(int i=1;i<(int)vec.size();i++){
      if(vec[i]<min){min=vec[i];imin=i;}
    } 
    vec_out.push_back(min);
    vec.erase((std::vector<float>::iterator)&vec[imin]);
  }while((int)vec.size()>0);
  vec=vec_out;
}

float median(vector<float> &vec){
  float min;
  int imin;
  vector<float> vec_out;
  int siz = (int)vec.size(); 
  if(siz<2)return 0;
  do{
    min=vec[0];imin=0;
    for(int i=1;i<(int)vec.size();i++){
      if(vec[i]<min){min=vec[i];imin=i;}
    }
    vec_out.push_back(min);
    vec.erase((std::vector<float>::iterator)&vec[imin]);
  }while((int)vec.size()>(siz>>1));
  float out = (vec_out[vec_out.size()-1]+vec_out[vec_out.size()-2])/2.0;
  vec=vec_out; 
  return out; 
}


float gamma(float n){    
    float tmp=0;
    n=rint(2*n)/2;
    if(rint(n)==n){
        tmp=1;
	for(int i=1;i<(int)n;i++)tmp*=i;
    }else if(rint(n)!=n){
	int ni=(int)floor(n);
        tmp=0.5;
	for(int i=1;i<ni;i++)tmp*=(i+0.5);
	tmp*=sqrt(M_PI);
    }
    return tmp;
}

float chi2(float x, float n){
  float a = 1/(sqrt(pow((double)2,(double)n))*gamma(0.5*n));    
    return (a*pow((double)x,(double)(0.5*(n-2)))*exp(-0.5*x));
}

float round(float x, int n){
    float mult=pow((double)10,(double)n);
    return rint(mult*x)/mult;
}

float probChi2(float thres, float n){
    float sum=0;
    float i=0;
    while(i<thres){
	sum+=(chi2(i,n));
	i+=0.1;
    }
    return sum/10.0;
}

float thresChi2(float prob, float n){
    float sum=0;
    float thres=0;
    prob*=10;
    while(sum<prob){
	sum+=(chi2(thres,n));
	thres+=0.1;
    }
    return thres;
}

vector<float> hist(vector<float> &vec,float min, float max, int bins){
  float db = (max-min)/bins;
  int i=0;while(vec[i]<min)i++;
  float thres=min+db;
  float bin=0;
  vector<float> hist;
  while( vec[i]<max){
    while(vec[i]<thres && i<(int)vec.size() && vec[i]<max){bin++;i++;}
    hist.push_back(((bin/(int)vec.size())*100));
    bin=0;
    thres+=db;
  }
  return hist;
}

