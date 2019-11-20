#include "detect_points.h"

#include "../../descriptor/descriptor.h"
#include "../../homography/homography.h"
#include "../../ttime/ttime.h"

#include <algorithm>
#include <iostream>
#include <map>
#include <cmath>

using std::pair;
using std::make_pair;
using std::sqrt;
using std::cout;
using std::cerr;
using std::endl;
using std::max;
using std::min;
using std::abs;

#define GRANULARITY 0.1

// [overlap_perc] = jp_calc_ellipse_intersection(x1,y1,a1,b1,c1,x2,y2,a2,b2,c2)

inline double
min_pair(const pair<double, double> &pr)
{
  if (pr.first<pr.second) return pr.first;
  else return pr.second;
}

inline double
max_pair(const pair<double, double> &pr)
{
  if (pr.first>pr.second) return pr.first;
  else return pr.second;
}

inline pair<double, double>
evaluate_root(const double &a, const double &b, const double &c)
{
  double val = -a/(b*b - a*c);
  if (val<0) {
    printf("Non-real soln: %f %f %f", a, b, c);
  }
  val = sqrt(val);
  return make_pair(-val,val);
}

inline pair<double, double>
evaluate_soln(const double &a, const double &b, const double &c,
              const double &y)
{
  double val = y*y*(b*b - a*c) + a;
  if (val<0) {
    return make_pair(0,0);
  }
  val = sqrt(val);
  return make_pair((-b*y - val)/a, (-b*y + val)/a);
}

inline double
calc_ellipse_area(const double &a,
                  const double &b,
                  const double &c)
{
  return M_PI/sqrt(a*c - b*b);
}

double
calc_ellipse_intersection(const double &x1,
                          const double &y1,
                          const double &a1,
                          const double &b1,
                          const double &c1,
                          const double &x2,
                          const double &y2,
                          const double &a2,
                          const double &b2,
                          const double &c2)
{
  pair<double, double> ell1_ys = evaluate_root(a1,b1,c1);
  pair<double, double> ell2_ys = evaluate_root(a2,b2,c2);

  double min_y = max_pair(make_pair(min_pair(ell1_ys)+y1, min_pair(ell2_ys)+y2));
  double max_y = min_pair(make_pair(max_pair(ell1_ys)+y1, max_pair(ell2_ys)+y2));

  if (min_y>max_y) return 0.0;

  double pixel_intersection = 0.0;
  //double last_width = 0.0;
  for (double y_line = min_y; y_line < max_y; y_line+=GRANULARITY) {
    pair<double, double> vals1 = evaluate_soln(a1, b1, c1, y_line-y1);
    pair<double, double> vals2 = evaluate_soln(a2, b2, c2, y_line-y2);
    double min_x = max(min_pair(vals1)+x1, min_pair(vals2)+x2);
    double max_x = min(max_pair(vals1)+x1, max_pair(vals2)+x2);
    if (min_x<max_x) {
      pixel_intersection += (max_x - min_x);
    }
  }
  pixel_intersection *= GRANULARITY;

  double a_area = calc_ellipse_area(a1,b1,c1);
  double b_area = calc_ellipse_area(a2,b2,c2);

  return pixel_intersection/(a_area + b_area - pixel_intersection);
}

void
get_ellipse_params(CornerDescriptor* cor, double& x, double& y, double& a, double& b, double& c)
{
  x = cor->getX();
  y = cor->getY();

  Matrix U(2,2),Vi,V,D;

  U(1,1)=cor->getMi11();
  U(1,2)=cor->getMi12();
  U(2,1)=cor->getMi21();
  U(2,2)=cor->getMi22();
  U.svd(Vi,D,V);
  D(1,1)=D(1,1)*cor->getCornerScale();
  D(2,2)=D(2,2)*cor->getCornerScale();
  D(1,1)=1.0/(D(1,1)*D(1,1));
  D(2,2)=1.0/(D(2,2)*D(2,2));      
  U=V*D*V.transpose();

  a = U(1,1);
  b = U(2,1);
  c = U(2,2);
}

double
compute_overlap(CornerDescriptor* cor1, CornerDescriptor* cor2)
{
  double x1, y1, a1, b1, c1, x2, y2, a2, b2, c2;

  get_ellipse_params(cor1, x1, y1, a1, b1, c1);
  get_ellipse_params(cor2, x2, y2, a2, b2, c2);

  return calc_ellipse_intersection(x1, y1, a1, b1, c1, x2, y2, a2, b2, c2);
}

namespace KM_detect_points {
int lib_main(int argc, char **argv){

  //int c='C';
  //cout << c<< endl;
  if(argc<2){ 
    cout << "Interest point detectors developed by Krystian.Mikolajczyk@inrialpes.fr\n";
    cout << "at INRIA Rhone-Alpes.[ref. Mikolajczyk and Schmid,IJCV'2004, http://www.robots.ox.ac.uk/~vgg/research/affine]" <<endl;
    cout << "     -i image.pgm  - input image pgm, ppm, png or jpg" << endl;
    cout << "Detectors:"<< endl;
    cout << "     -har - harris detector " << endl;
    //cout << "     -hes - hessian detector " << endl;
    cout << "     -harlap -   harris-laplace detector " << endl;
    cout << "     -heslap -   hessian-laplace detector " << endl;
    /*cout << "     -fmhar -  fast multi-scale harris detector " << endl;
    cout << "     -fmhar -  fast multi-scale harris detector " << endl;*/
    cout << "     -haraff -   harris-affine detector " << endl;
    cout << "     -hesaff -  hessian-affine detector " << endl;
    //cout << "     -edge -   multi-scale edge feature detector " << endl;
    //cout << "     -fmhes -  fast multi-scale hessian detector " << endl;
    //cout << "     -fmharhes - fast multi-scale harris+hessian detector " << endl;
    cout << "     -dog - DoG detector [D. Lowe]" << endl;
    /*cout << "     -dog - DoG detector [D. Lowe]" << endl;
    cout << "Laplacian based scale selection for detected points:"<< endl;
    cout << "     -lap -  laplacian scale selection with -har, -hes, -mhar or -mhes" << endl;
    cout << "Estimation of affine point neighbourhood:"<< endl;
    cout << "     -aff - estimation of affine neighborhood  with -har, -hes, -mhar or -mhes" << endl;
    */
    cout << "Other optional arguments:"<< endl;
    cout << "     -thres - cornerness threshold " << endl; 
    //cout << "     -s - detection scale (-har, -hes)" << endl; 
    cout << "     -o out.har - saves points in out.har" << endl; 
    cout << "     -DR - draws points in out.har.pgm" << endl; 
    cout << "     -remove-overlapping - JP: Removes regions with very high overlap" << endl;
    //cout << "     -c 255 -  grayvalue for drawing [0,...,255]" << endl; 
    cout <<"example:\n\t"<< argv[0]<< " -haraff  -i image.pgm -o out.har -thres 500 -DR" <<endl<< endl;
    //cout <<"\t" << argv[0]<<" -mhar -i image.ppm -lap -aff -DR > report"<< endl<< endl;
    //    cout <<"nb_of_points"<< endl;
    //    cout <<"0"<< endl;
    //    cout <<"x y cornerness scale angle point_type laplacian extremum_type mi11 mi12 mi21 mi22 "<< endl << endl;
    cout <<"Regions are defined by:"<< endl<<" a(x-u)(x-u)+2b(x-u)(y-v)+c(y-v)(y-v)=1"<< endl << "with (0,0) at image top left corner"<< endl << endl;
    cout <<"output file format:"<< endl;
    cout <<"1.0"<< endl;
    cout <<"nb_of_points"<< endl;
    cout <<"u v a b c"<< endl;
    cout <<"  :"<< endl;
    cout <<"  :"<< endl;

    return -1;
  }
  char input[512];
  char output[512];
  char draw[512];
  int detector=0,aff=0,in=0,out=0,dr=0,color=255, lap=0,faff=0, remove_overlapping=0;
  float scale=1.2;
  for(int i=1;i<argc;i++){ 
    if(!strcmp(argv[i],"-i")){
      in = 1;sprintf(input,argv[i+1]);
    }
  }
  float threshold = 100;

  for(int i=1;i<argc;i++){ 
    if(!strcmp(argv[i],"-har")){   
      detector = 7;sprintf(output,"%s.har",input);
      }else if(!strcmp(argv[i],"-hes")){
	detector = 6;sprintf(output,"%s.hes",input);
      }else if(!strcmp(argv[i],"-harlap")){
	detector = 4;sprintf(output,"%s.harlap",input);
      }else if(!strcmp(argv[i],"-heslap")){
	detector = 5;sprintf(output,"%s.heslap",input);
      }else if(!strcmp(argv[i],"-dog")){
	detector = 8;sprintf(output,"%s.dog",input);
      }else if(!strcmp(argv[i],"-edge")){
	detector = 9;sprintf(output,"%s.edge",input);
      }else if(!strcmp(argv[i],"-haraff")){
	detector = 12;sprintf(output,"%s.haraff",input);
      }else if(!strcmp(argv[i],"-hesaff")){
	detector = 13;sprintf(output,"%s.hesaff",input);
      }else if(!strcmp(argv[i],"-fmhar")){
	detector = 10;sprintf(output,"%s.har",input);
      }else if(!strcmp(argv[i],"-fmhes")){
	detector = 11;sprintf(output,"%s.hes",input);
      }else if(!strcmp(argv[i],"-all")){
	detector = 3;sprintf(output,"%s.all",input);
      }else if(!strcmp(argv[i],"-aff")){
	aff = 1;
      }else if(!strcmp(argv[i],"-faff")){
	faff = 1;
      }else if(!strcmp(argv[i],"-lap")){
	lap = 1;
      }else if(!strcmp(argv[i],"-DR")){
	dr=1;
      }else if(!strcmp(argv[i],"-c")){
	color=atoi(argv[i+1]);
      }else if(!strcmp(argv[i],"-s")){
	scale=atof(argv[i+1]);
      }else if(!strcmp(argv[i],"-thres")){
	threshold=atof(argv[i+1]);
      }else if(!strcmp(argv[i],"-remove-overlapping")){
        remove_overlapping = 1;
      }
  }
  if(in==0 || detector==0){
    cout << "no input file or detector not specified."<< endl;
    return -1;
  }

  for(int i=1;i<argc;i++){ 
    if(!strcmp(argv[i],"-o1") || !strcmp(argv[i],"-o")){
      out=1;sprintf(output,argv[i+1]);
      i++;
    }
    else if(!strcmp(argv[i],"-o2")){
      out=2;sprintf(output,argv[i+1]);
      i++;
    }
  }

  //cout << "detecting points in: "<< input << endl;

  //return -1;

  vector<CornerDescriptor *> cor1;  

  DARY *image = new DARY(input);
  image->toGRAY(); 
  image->char2float();     
  long ti;  
  init_time(&ti);
  if(detector==1){
    cout<< "fast multi-scale harris detector..."<< endl;
    multi_harris_fast(image, cor1, threshold);
  }else if(detector==2){
    cout<< "fast multi-scale hessian detector..."<< endl;
    multi_hessian_fast(image, cor1, threshold);
  }else if(detector==3){
    cout<< "fast multi-scale harris+hessian+dog detector..."<< endl;
    multi_harris_hessian_fast(image, cor1, threshold, threshold);
    dog(image,cor1);   
  }else if(detector==41){
    cout<< "multi-scale harris detector..."<< endl;
    multi_harris(image, cor1,threshold, 1,30,1.4);
  }else if(detector==51){
    cout<< "multi-scale hessian detector..."<< endl;
    multi_hessian(image, cor1,threshold,1,30,1.4);
  }else if(detector==6){
    cout<< "hessian detector..."<< endl;
    hessian(image, cor1,threshold,scale);
  }else if(detector==7){
    cout<< "harris detector..."<< endl;
    harris(image, cor1,threshold,scale,scale*1.4,0.06);
    for(uint i=0;i<cor1.size();i++){
      cor1[i]->setCornerScale(2.5);
    }
  }else if(detector==9){
    cout<< "edge detector..."<< endl;
    edgeFeatureDetector(image, cor1);
  }else if(detector==8){
    cout<< "dog detector..."<< endl;
    dog(image,cor1);  
    for(uint i=0;i<cor1.size();i++){
      cor1[i]->setAngle(1000);
    }
  }else if(detector==10){
    cout<< "multi-scale  NI harris detector..."<< endl;
    multi_scale_har(image, cor1,threshold,1.2,0);     
  }else if(detector==11){
    cout<< "multi-scale NI affine hessian detector..."<< endl;
    multi_scale_hes(image, cor1,threshold,1.1,0);     
  }else if(detector==12){
    #ifndef QUIET
    cout<< "harris affine  detector..."<< endl;
    #endif
    multi_scale_har(image, cor1,threshold,1.1,16);      
  }else if(detector==13){
    #ifndef QUIET
    cout<< "hessian affine  detector..."<< endl;
    #endif
    multi_scale_hes(image, cor1,threshold,1.2,16);     
  }else if(detector==4){
    cout<< "harris laplace  detector..."<< endl;
    multi_scale_har(image, cor1,threshold,1.1,0);      
  }else if(detector==5){
    cout<< "hessian laplace  detector..."<< endl;
    multi_scale_hes(image, cor1,threshold,1.2,0);     
  }else {
    cout << "wrong detector" << endl; return 0;
  }
  #ifndef QUIET
  cout << "detection time: " << tell_time(ti)<< endl;
  #endif

  if(lap){
    cout<< "laplacian scale selection..."<< endl;
    computLaplacian(image, cor1, 3, 1.4);
  }

  #ifndef QUIET
  cout <<endl << "number of points : " << cor1.size()<< endl;
  #endif
  if(aff){
    sprintf(output,"%s.aff",output);
    cout<< "estimating affine point neighborhood..."<< endl;
    //init_time(&ti);
    findAffineRegion_fast(image, cor1);
    //cout << "affine detection time " << tell_time(ti)<< endl;
    //removeSimilarPoints(cor1);
    cout <<endl << "number of affine points : " << cor1.size()<< endl;
  } 


  if(faff){
    sprintf(output,"%s.faff",output);
    cout<< "estimating affine point neighborhood..."<< endl;
    //init_time(&ti);
    findAffineRegion(image, cor1);
    cout << "affine detection time " << tell_time(ti)<< endl;
    removeSimilarPoints(cor1);
    cout <<endl << "number of affine points : " << cor1.size()<< endl;
  }

  #ifndef QUIET
  cout << "output file: " << output<< endl;
  #endif

  // Remove overlapping regions.
  if (remove_overlapping) {
    cout << "Before removing overlap: " << cor1.size() << endl;
    for (size_t i=0; i<cor1.size(); ++i) {
      for (size_t j=i+1; j<cor1.size(); ++j) {
        float overlap = compute_overlap(cor1[i], cor1[j]);
        if (overlap>=0.95) {
          delete cor1[j];
          cor1.erase(cor1.begin() + j);
          j--;
        }
      }
    }
    cout << "After removing overlap: " << cor1.size() << endl;
  }

  if(out==2)
    writeCorners(cor1, output,0);
  if(out==1 || !out){
   ofstream outfile(output);
   if(!outfile){
     cout << "error opening: "<<output<< endl;
     return -1;
   }
   Matrix U(2,2),Vi,V,D;
   outfile << "1.0"<< endl;  
   outfile << cor1.size()<< endl;  
   for(uint i=0;i<cor1.size();i++){
     U(1,1)=cor1[i]->getMi11();
     U(1,2)=cor1[i]->getMi12();
     U(2,1)=cor1[i]->getMi21();
     U(2,2)=cor1[i]->getMi22();
     U.svd(Vi,D,V);
     D(1,1)=D(1,1)*cor1[i]->getCornerScale();
     D(2,2)=D(2,2)*cor1[i]->getCornerScale();
     D(1,1)=1.0/(D(1,1)*D(1,1));
     D(2,2)=1.0/(D(2,2)*D(2,2));      
     U=V*D*V.transpose();
     //coutfile << cor1[i]->getX()<< " " << cor1[i]->getX()<< " " << U(1,1)<< " " << U(1,2)<< " " << U(2,1)<< " " << U(2,2)<< endl;
     outfile << cor1[i]->getX()<< " " << cor1[i]->getY()<< " " << U(1,1)<< " " << U(2,1)<< " " << U(2,2)<< endl;
   }                   
   outfile.close();    
  }
  //  writeCorners(cor1, output);
  /*
   */
  
  
  
  if(dr){
    sprintf(draw,"%s.png",output);  
    cout << "drawing points in: " << draw<< endl;
    drawAffineCorners(image, cor1, draw,color);
  }
  delete image;  
  for(size_t i= 0; i<cor1.size(); ++i)
      delete cor1[i];
  cor1.clear();
  
  return 0; 
}
  
}



/*
int main(int argc, char **argv){
    int ret= KM_detect_points::lib_main(argc,argv);
    if (ret!=0)
        exit(ret);
    return 0;
}
*/
