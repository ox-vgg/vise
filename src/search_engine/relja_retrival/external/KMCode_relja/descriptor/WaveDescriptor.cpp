#include "waveDescriptor.h"

int WaveDescriptor::code(DARY *img, int x, int y){
    int plus=0,minus=0,zero=0,code_=0;
    for(int j=y-1;j<=y;j++){
	for(int i=x-1;i<=x;i++){
	    if(img->fel[j][i]==0)minus++;
	    else if(img->fel[j][i]==1)zero++;
	    else plus++;
	}
    }

    //if(zero!=4)cout << " minus " << minus << " zero " << zero << " plus " << plus;
    if(minus==4)code_=0;
    else if(zero==4)code_=1;
    else if(plus==4)code_=2;
    else if(minus==3){
	if(zero==1)code_=3;
	else code_=4;//plus==1
    }else if(zero==3){
	if(minus==1)code_=5;
	else code_=6;//plus==1
    }else if(plus==3){
	if(minus==1)code_=7;
	else code_=8;//zero==1
    }else if(minus==2){
	if(zero==2)code_=9;
	else if(plus==2)code_=10;
	else code_=11;//zero==1, plus==1
    }else if(zero==2){
	if(plus==2)code_=12;
	else code_=13;//zero==1, minus==1
    }else code_=14;//plus==2, minus=1, zero=1
    return code_;
}

void WaveDescriptor::computeCodeHistogram(DARY *img, int sxs, int sys, int sw, 
					  int level){    
    int hs=0;
    if(level==2)hs=90;
    else if(level==3)hs=180;
    int code_=0;
    int dist=0;
    int rxc = sw>>1;
    int max_dist=rxc-1;
    int dw=0;
    /*_____
      |    |
      |  o |
      |____|
     */
    if((sw>>1)>6)dw=1;
    for(int j=1;j<sw;j++){
	for(int i=1;i<sw;i++){
	    dist=(int)rint(sqrt((i-rxc)*(i-rxc)+(j-rxc)*(j-rxc)));
	    dist=(dist<rxc)?dist:max_dist;
	    //cout << " dw "<< dw << " dist " << dist;
	    dist>>=dw;
	    //cout << " dist_r "<< dist << " code " << code(img,sxs+i,sys+j)  << endl;getchar(); 
	    code_=code(img,sxs+i,sys+j);
	    if(code_!=1)vec[hs+dist*15+code_]++;
	}
    }    
}
void WaveDescriptor::computeCodeHistogram(DARY *img){
    int sxs = img->x(),sw = (img->x());
    int sys = img->y();
    sxs>>=1;sys>>=1;sw>>=1;
    computeCodeHistogram(img,0,sys,sw,1);
    computeCodeHistogram(img,sxs,0,sw,1);
    computeCodeHistogram(img,sxs,sys,sw,1);
    sxs>>=1;sys>>=1;sw>>=1;
    computeCodeHistogram(img,0,sys,sw,2);
    computeCodeHistogram(img,sxs,0,sw,2);
    computeCodeHistogram(img,sxs,sys,sw,2);
    sxs>>=1;sys>>=1;sw>>=1;
    computeCodeHistogram(img,0,sys,sw,3);
    computeCodeHistogram(img,sxs,0,sw,3);
    computeCodeHistogram(img,sxs,sys,sw,3);
}
void WaveDescriptor::computeComponents(DARY *img,  Wavelet *wave){
	if(img==NULL){return;}
	int mins = (int)(GAUSS_CUTOFF*c_scale+2);
	if(!isOK(mins,img->x()-mins,img->y()-mins))return;

	const static double threshold[4]={30,30,40,50};
	d_scale=c_scale;
	allocVec(240);
	if(vec==NULL)return;


	double lecos=cos(-angle);
        double lesin=sin(-angle);
	
	double scmean=5.0;
	double scal=3*d_scale/scmean;
	
	double m11=scal*(mi11*lecos-mi12*lesin);
	double m12=scal*(mi11*lesin+mi12*lecos);
	double m21=scal*(mi21*lecos-mi22*lesin);
	double m22=scal*(mi21*lesin+mi22*lecos);

	//int im_size=2*(int)rint(GAUSS_CUTOFF*d_scale)+3;
        int im_size=33;//2*(int)rint(GAUSS_CUTOFF*scmean)+3;
        //im_size=2*(int)rint(3*scmean)+3;
        DARY * imgn = new DARY(im_size,im_size);      
       	imgn->interpolate(img,x,y,m11,m12,m21,m22);
	x=imgn->x()>>1;y=imgn->y()>>1;
       	normalize(imgn,(int)x,(int)y,x);
	double c_scalet=c_scale;
        c_scale=d_scale=scmean;

	int xi = (int)x;
	int yi = (int)y;
	//double r_scale = 24.0/(d_scale*5.0);
	int levels=3;
	double angle=0;
	DARY * qregion = new DARY(im_size,im_size,0.0);
	imgn->write("image.pgm");
	wave->wt(imgn,levels);
	imgn->write("q2image.pgm");getchar();
	wave->quantize(imgn,qregion,threshold,1,levels);       
	qregion->normalize(0,2);qregion->write("q2image.pgm");getchar();
	//computeCodeHistogram(qregion);
	if(d_scale>0){
	    //region->write_double("wimage.pgm");
	    //qregion->write_double("qnimage.pgm");
	    cout << " xi "<<xi << " yi " << yi << " sc "<<d_scale<< " v0 "<<vec[0]<< endl;getchar();
	    state=0;
	}
	
	state=1;
	delete imgn;delete qregion;
} 

void computeWaveDescriptors(DARY *image,  vector<CornerDescriptor *> &desc){
    WaveDescriptor * ds=new WaveDescriptor();
    Wavelet *wave = new Wavelet("  ");
    for(unsigned int c=0;c<desc.size();c++){
	cout << "\rcompute "<< c<< " of "<< desc.size() << "    " << flush;;
	ds->copy(desc[c]);
	ds->computeComponents(image, wave);
 	desc[c]->copy((CornerDescriptor*)ds); 
    }
    for(unsigned int c=0;c<desc.size();c++){
	if(!desc[c]->isOK()){
	    desc.erase(&desc[c]);
	    c--;
	}
    }
    cout<<endl;
}
