#include "edgeDetector.h"


float bilin(DARY *grad, float x, float y){

  int xi = (int)floor(x);
  float a = x-xi;
  int yi = (int)floor(y);
  float b = y-yi;
  
  float a1,b1,v00,v10,v01,v11;
  a1=1.0-a;
  b1=1.0-b;
   
  v00 = a1*grad->fel[yi][xi];
  v10 = a*grad->fel[yi][xi+1];
  v01 = a1*grad->fel[yi+1][xi];
  v11 = a*grad->fel[yi+1][xi+1];
  return b1*(v00+v10)+b*(v01+v11);
}


void followEdge(int x,int y, DARY *edge,Segment *se, int dir){
  float  dori_max=100;
  int flag=1,isNew=0;
  int actx = x,acty=y, x_2=x,y_2=y,x_1=x,y_1=y,nx=0,ny=0,nb=0;
  float  dx=0,dy=0,da_2=0,da_1=0,da=0,da_max,da_act=0;
  do{
    flag=0;da_max=0;isNew=0;dori_max=10;
    x_2=x_1;y_2=y_1;x_1=actx;y_1=acty;da_2=da_1;da_1=da_act;

    if(edge->fel[acty][actx+1]>0){
      dy=acty-y_2;dx=actx+1-x_2;
      da=atan2(dy,dx);
      da_max=fabs(da-da_2);
      if(da_max>M_PI)da_max=M_2PI-da_max;
      if(da_max<dori_max){
	dori_max=da_max;
	da_act=da;
	nx=actx+1;ny=acty;
	isNew=1;
      }      
    }
    if(edge->fel[acty+1][actx+1]>0){
      dy=acty+1-y_2;dx=actx+1-x_2;
      da=atan2(dy,dx);
      da_max=fabs(da-da_2);
      if(da_max>M_PI)da_max=M_2PI-da_max;
      if(da_max<dori_max){
	dori_max=da_max;
	da_act=da;
	nx=actx+1;ny=acty+1;
	isNew=1;
      }      
    }
    if(edge->fel[acty+1][actx]>0){
      dy=acty+1-y_2;dx=actx-x_2;
      da=atan2(dy,dx);
      da_max=fabs(da-da_2);
      if(da_max>M_PI)da_max=M_2PI-da_max;
      if(da_max<dori_max){
	dori_max=da_max;
	da_act=da;
	nx=actx;ny=acty+1;
	isNew=1;
      }      
    }
    if(edge->fel[acty+1][actx-1]>0){
      dy=acty+1-y_2;dx=actx-1-x_2;
      da=atan2(dy,dx);
      da_max=fabs(da-da_2);
      if(da_max>M_PI)da_max=M_2PI-da_max;
      if(da_max<dori_max){
	dori_max=da_max;
	da_act=da;
	nx=actx-1;ny=acty+1;
	isNew=1;
      }      
    }
    if(edge->fel[acty][actx-1]>0){
      dy=acty-y_2;dx=actx-1-x_2;
      da=atan2(dy,dx);
      da_max=fabs(da-da_2);
      if(da_max>M_PI)da_max=M_2PI-da_max;
      if(da_max<dori_max){
	dori_max=da_max;
	da_act=da;
	nx=actx-1;ny=acty;
	isNew=1;
      }      
    }
    if(edge->fel[acty-1][actx-1]>0){
      dy=acty-1-y_2;dx=actx-1-x_2;
      da=atan2(dy,dx);
      da_max=fabs(da-da_2);
      if(da_max>M_PI)da_max=M_2PI-da_max;
      if(da_max<dori_max){
	dori_max=da_max;
	da_act=da;
	nx=actx-1;ny=acty-1;
	isNew=1;
      }      
    }
    if(edge->fel[acty-1][actx]>0){
      dy=acty-1-y_2;dx=actx-x_2;
      da=atan2(dy,dx);
      da_max=fabs(da-da_2);
      if(da_max>M_PI)da_max=M_2PI-da_max;
      if(da_max<dori_max){
	dori_max=da_max;
	da_act=da;
	nx=actx;ny=acty-1;
	isNew=1;
      }      
    }
    if(edge->fel[acty-1][actx+1]>0){
      dy=acty-1-y_2;dx=actx+1-x_2;
      da=atan2(dy,dx);
      da_max=fabs(da-da_2);
      if(da_max>M_PI)da_max=M_2PI-da_max;
      if(da_max<dori_max){
	dori_max=da_max;
	da_act=da;
	nx=actx+1;ny=acty-1;
	isNew=1;
      }      
    }


   if(isNew){
     //cout <<nb<< "dx "<<nx-actx<< " dy "<<ny-acty<<   " d_ori " <<  dori_max*180/M_PI<< endl;
      edge->fel[acty][actx]=0;
      if(nx>0 && nx<edge->x() && ny>0 && ny<edge->y() ){
	flag=1;
	if(dir)se->chain.push_back(new ChainPoint(nx,ny,da_act)); 
	else se->chain.insert((std::vector<ChainPoint*>::iterator)&(se->chain[0]),new ChainPoint(nx,ny,da_act));

	acty=ny;actx=nx;
	nb++;
      }
   }//else cout << "stopping"<< endl;    
    
  }while(flag);
  
}

void drawSegments(DARY *segments ,vector<Segment*> seg, const char *filename){
  //  cout << "NB SEG" << seg.size() << endl;
  int x,y;
  for(uint s=0;s<seg.size();s++){
    for(uint p=0;p<seg[s]->chain.size();p++){
      x=seg[s]->chain[p]->x;y=seg[s]->chain[p]->y;
      if(x>0 && x<segments->x() && y>0 && y< segments->y())
	segments->fel[y][x]=255;//s*255.0/seg.size();
    }
  }
  segments->write(filename);
}

//const static float gs[5]={0.0545, 0.2442, 0.4026, 0.2442, 0.0545};
const static float gs[5]={0.2, 0.2, 0.2, 0.2, 0.2};

void smoothSegment(Segment * seg){
  
  for(uint p=2;p<seg->chain.size()-2;p++){
    seg->chain[p]->x=(int)rint(gs[0]*(seg->chain[p-2]->x)+gs[1]*(seg->chain[p-1]->x)+
			       gs[2]*(seg->chain[p]->x)+gs[3]*(seg->chain[p+1]->x)+gs[4]*(seg->chain[p+2]->x));
    seg->chain[p]->y=(int)rint(gs[0]*(seg->chain[p-2]->y)+gs[1]*(seg->chain[p-1]->y)+
			       gs[2]*(seg->chain[p]->y)+gs[3]*(seg->chain[p+1]->y)+gs[4]*(seg->chain[p+2]->y));    
    seg->chain[p]->ddori=(gs[0]*(seg->chain[p-2]->ddori)+gs[1]*(seg->chain[p-1]->ddori)+
			  gs[2]*(seg->chain[p]->ddori)+gs[3]*(seg->chain[p+1]->ddori)+gs[4]*(seg->chain[p+2]->ddori)); 
  }   
  
}

float checkVariation(Segment * seg){
  float var=0,ddori;
  //cout << "FOUND NEW SEGMENT "<<  endl;
  for(uint p=2;p<seg->chain.size()-2;p++){
    ddori=fabs(seg->chain[p-2]->ori - seg->chain[p+2]->ori);
    if(ddori>M_PI)ddori=M_2PI-ddori;
    seg->chain[p]->ddori=ddori;
    //   cout << ddori<< endl;
    var+=ddori;      
  }
  var=var/(float)seg->chain.size();
  //cout << var<< endl;
  
  return var;
}

void detectEdgeSegments(DARY *edge_in, vector<Segment*> &seg){
  
  DARY *edge = new DARY(edge_in);
  Segment *se; 
  for(int j=0;j<edge->y();j++){
    for(int i=0;i<edge->x();i++){
      if(edge->fel[j][i]>0){
	//cout << "found segment " << i << " " << j<<  endl;
	se = new Segment(i,j,0.0);
	followEdge(i,j,edge,se,1);
	followEdge(i,j,edge,se,0);
	if(se->chain.size()>30 && checkVariation(se)<1){
	smoothSegment(se);
	  seg.push_back(se);
	}else delete se;
      }  
    }  
  }
  delete edge;
}


void detectPointsOnEdges( vector<Segment*> &seg, vector<CornerDescriptor*> &cor){
  
  vector<uint> ppp;
  float cornerness = 100,scale=10,x,y;
  uint p0;
  for(uint s=0;s<seg.size();s++){
    ppp.clear();
    if(seg[s]->chain.size()>10){
      for(uint p=2;p<seg[s]->chain.size()-2;p++){
	//cout << "ori  "<<seg[s]->chain[p]->ddori<< endl;
	if(seg[s]->chain[p]->ddori>1){ /*Threshold on the second derivative of the curvature*/ 
	  ppp.push_back(p);
	}
      }
      p0=0;
      while(p0+1<ppp.size()){
	if(ppp[p0+1]-ppp[p0]<15){
	  //cout << p0<< " "<<ppp[p0]<< endl;
	  if(seg[s]->chain[ppp[p0]]->ddori < seg[s]->chain[ppp[p0+1]]->ddori){	  
	    //out << " erasing " << ppp[p0]<< endl;
	    ppp.erase((std::vector<uint>::iterator)&ppp[p0]);
	  }
	  else {
	    //cout << " erasing " << ppp[p0+1]<< endl;
	    ppp.erase((std::vector<uint>::iterator)&ppp[p0+1]);
	  }	  	  
	}else p0++;	
      }

      ppp.push_back(seg[s]->chain.size()>>1);
      ppp.push_back(seg[s]->chain.size()-1);
      ppp.push_back(0);
      for(uint p=0;p<ppp.size();p++){
	x=seg[s]->chain[ppp[p]]->x;y=seg[s]->chain[ppp[p]]->y;
	cor.push_back(new CornerDescriptor(x,y,cornerness,scale));
      }
      for(uint p=1;p<ppp.size();p++){
	if(ppp[p]-ppp[p-1]>20){
	  x=seg[s]->chain[(ppp[p]+ppp[p-1])>>1]->x;y=seg[s]->chain[(ppp[p]+ppp[p-1])>>1]->y;
	cor.push_back(new CornerDescriptor(x,y,cornerness,scale));
	}
      }
      //getchar();
    }    
  }  
}


void cannyEdges(DARY *img, DARY *edge,  float sigma, float lower_threshold,
	   float higher_threshold){

  float color=115;

  DARY *dx = new DARY(img->y(),img->x(),0.0);
  DARY *dy = new DARY(img->y(),img->x(),0.0);
  DARY *grad = new DARY(img->y(),img->x(),0.0);
  DARY *tmp_edge = new DARY(img->y(),img->x(),0.0);

  dX(img,dx,sigma);
  dY(img,dy,sigma);
  for(int j=0;j<grad->y();j++){
    for(int i=0;i<grad->x();i++){
      grad->fel[j][i]=sigma*sqrt(dx->fel[j][i]*dx->fel[j][i]+dy->fel[j][i]*dy->fel[j][i]); 
      if(grad->fel[j][i]<1){
	grad->fel[j][i]=1;dx->fel[j][i]=0;dy->fel[j][i]=0;
      }
	
    }
  }
  vector<int> cor_edge;cor_edge.push_back(0);
  float x1,y1,x2,y2,ux,uy,g,g1,g2,nb=1;
  for(int j=1;j<grad->y()-2;j++){
    for(int i=1;i<grad->x()-2;i++){
      ux=dx->fel[j][i];
      uy=dy->fel[j][i];
      g=grad->fel[j][i];
      x1=i+ux/g;
      y1=j+uy/g;
      x2=i-ux/g;
      y2=j-uy/g;
      //cout <<g<< " " << ux/g << " " << uy/g <<" "<<i<< " " << j << " " << x1 << " " << y1 << " " << x2 << " " << y2 << endl;
      g1=bilin(grad,x1,y1);
      g2=bilin(grad,x2,y2);    
      if(g<g1 || g<=g2 || g<lower_threshold)continue;
      else if(g>higher_threshold)edge->fel[j][i]=color;
      else if(g>lower_threshold && (edge->fel[j-1][i-1]==color  ||
				    edge->fel[j-1][i]==color ||
				    edge->fel[j-1][i+1]==color ||
				    edge->fel[j][i-1]==color)){	
	edge->fel[j][i]=color;}
      else if(g>lower_threshold){
	if(tmp_edge->fel[j-1][i-1]>0){
	  tmp_edge->fel[j][i]=tmp_edge->fel[j-1][i-1];
	}
	else if(tmp_edge->fel[j-1][i]>0){
	  tmp_edge->fel[j][i]=tmp_edge->fel[j-1][i];
	}
	else if(tmp_edge->fel[j-1][i+1]>0){
	  tmp_edge->fel[j][i]=tmp_edge->fel[j-1][i+1];
	}
	else if(tmp_edge->fel[j][i-1]>0){
	  tmp_edge->fel[j][i]=tmp_edge->fel[j][i-1];
	}
	else if(tmp_edge->fel[j][i+1]>0){
	  tmp_edge->fel[j][i]=tmp_edge->fel[j][i+1];
	}else {
	  tmp_edge->fel[j][i]=nb;cor_edge.push_back(0);nb++;
	}
      }
    }
  }

  for(int j=edge->y()-2;j>0;j--){
    for(int i=edge->x()-2;i>0;i--){
	if((edge->fel[j-1][i-1]==color ||
	    edge->fel[j-1][i]==color ||
	    edge->fel[j-1][i+1]==color ||
	    edge->fel[j][i-1]==color ||
	    edge->fel[j][i+1]==color ||
	    edge->fel[j+1][i-1]==color ||
	    edge->fel[j+1][i]==color ||
	    edge->fel[j+1][i+1]==color))cor_edge[(int)tmp_edge->fel[j][i]]=1;	
    }
  }  
  for(int j=edge->y()-2;j>0;j--){
    for(int i=edge->x()-2;i>0;i--){
      if(tmp_edge->fel[j][i]>0 && cor_edge[(int)tmp_edge->fel[j][i]]==1){	
	edge->fel[j][i]=color;
      }
    }
  } 
  for(int j=edge->y()-2;j>0;j--){
    for(int i=edge->x()-2;i>0;i--){
      if(edge->fel[j][i]>0){	
	edge->fel[j][i]=0;//5*grad->fel[j][i];
      }else edge->fel[j][i]=255;          

    }
  } 
  delete dx;delete dy;delete grad;delete tmp_edge;
  cor_edge.clear();
}

void cannyEdgesGrad(DARY *img, DARY *edge,  float sigma, float lower_threshold,
	   float higher_threshold){

  DARY *dx = new DARY(img->y(),img->x(),0.0);
  DARY *dy = new DARY(img->y(),img->x(),0.0);
  DARY *grad = new DARY(img->y(),img->x(),0.0);
  DARY *tmp_edge = new DARY(img->y(),img->x(),0.0);

  dX6(img,dx);
  dY6(img,dy);
  for(int j=0;j<grad->y();j++){
    for(int i=0;i<grad->x();i++){
      grad->fel[j][i]=sigma*sqrt(dx->fel[j][i]*dx->fel[j][i]+dy->fel[j][i]*dy->fel[j][i]); 
      if(grad->fel[j][i]<1){
	grad->fel[j][i]=1;dx->fel[j][i]=0;dy->fel[j][i]=0;
      }
	
    }
  }
  vector<int> cor_edge;cor_edge.push_back(0);
  float color=185;
  float x1,y1,x2,y2,ux,uy,g,g1,g2,nb=1;
  for(int j=1;j<grad->y()-2;j++){
    for(int i=1;i<grad->x()-2;i++){
      ux=dx->fel[j][i];
      uy=dy->fel[j][i];
      g=grad->fel[j][i];
      x1=i+ux/g;
      y1=j+uy/g;
      x2=i-ux/g;
      y2=j-uy/g;
      //cout <<g<< " " << ux/g << " " << uy/g <<" "<<i<< " " << j << " " << x1 << " " << y1 << " " << x2 << " " << y2 << endl;
      g1=bilin(grad,x1,y1);
      g2=bilin(grad,x2,y2);    
      if(g<g1 || g<=g2 || g<lower_threshold)continue;
      else if(g>higher_threshold)edge->fel[j][i]=color;
      else if(g>lower_threshold && (edge->fel[j-1][i-1]==color  ||
				    edge->fel[j-1][i]==color ||
				    edge->fel[j-1][i+1]==color ||
				    edge->fel[j][i-1]==color)){	
	edge->fel[j][i]=color;}
      else if(g>lower_threshold){
	if(tmp_edge->fel[j-1][i-1]>0){
	  tmp_edge->fel[j][i]=tmp_edge->fel[j-1][i-1];
	}
	else if(tmp_edge->fel[j-1][i]>0){
	  tmp_edge->fel[j][i]=tmp_edge->fel[j-1][i];
	}
	else if(tmp_edge->fel[j-1][i+1]>0){
	  tmp_edge->fel[j][i]=tmp_edge->fel[j-1][i+1];
	}
	else if(tmp_edge->fel[j][i-1]>0){
	  tmp_edge->fel[j][i]=tmp_edge->fel[j][i-1];
	}
	else if(tmp_edge->fel[j][i+1]>0){
	  tmp_edge->fel[j][i]=tmp_edge->fel[j][i+1];
	}else {
	  tmp_edge->fel[j][i]=nb;cor_edge.push_back(0);nb++;
	}
      }
    }
  }

  for(int j=edge->y()-2;j>0;j--){
    for(int i=edge->x()-2;i>0;i--){
	if((edge->fel[j-1][i-1]==color ||
	    edge->fel[j-1][i]==color ||
	    edge->fel[j-1][i+1]==color ||
	    edge->fel[j][i-1]==color ||
	    edge->fel[j][i+1]==color ||
	    edge->fel[j+1][i-1]==color ||
	    edge->fel[j+1][i]==color ||
	    edge->fel[j+1][i+1]==color))cor_edge[(int)tmp_edge->fel[j][i]]=1;	
    }
  }  
  for(int j=edge->y()-2;j>0;j--){
    for(int i=edge->x()-2;i>0;i--){
      if(tmp_edge->fel[j][i]>0 && cor_edge[(int)tmp_edge->fel[j][i]]==1){	
	edge->fel[j][i]=color;
      }
    }
  } 

  for(int j=0;j<grad->y();j++){
    for(int i=0;i<grad->x();i++){
      if(edge->fel[j][i]>0)edge->fel[j][i]=grad->fel[j][i];
    }
  }
  delete dx;delete dy;delete grad;delete tmp_edge;
  cor_edge.clear();
}


void cannyEdgesGrad(DARY *dy, DARY *dx, DARY *edge,  float lower_threshold,
	   float higher_threshold){

  DARY *grad = new DARY(dx->y(),dx->x(),0.0);
  DARY *tmp_edge = new DARY(dx->y(),dx->x(),0.0);

  for(int j=0;j<grad->y();j++){
    for(int i=0;i<grad->x();i++){
      grad->fel[j][i]=sqrt(dx->fel[j][i]*dx->fel[j][i]+dy->fel[j][i]*dy->fel[j][i]); 
      if(grad->fel[j][i]<1){
	grad->fel[j][i]=1;dx->fel[j][i]=0;dy->fel[j][i]=0;
      }
	
    }
  }
  vector<int> cor_edge;cor_edge.push_back(0);
  float color=255;
  float x1,y1,x2,y2,ux,uy,g,g1,g2,nb=1;
  for(int j=1;j<grad->y()-2;j++){
    for(int i=1;i<grad->x()-2;i++){
      ux=dx->fel[j][i];
      uy=dy->fel[j][i];
      g=grad->fel[j][i];
      x1=i+ux/g;
      y1=j+uy/g;
      x2=i-ux/g;
      y2=j-uy/g;
      //cout <<g<< " " << ux/g << " " << uy/g <<" "<<i<< " " << j << " " << x1 << " " << y1 << " " << x2 << " " << y2 << endl;
      g1=bilin(grad,x1,y1);
      g2=bilin(grad,x2,y2);    
      if(g<g1 || g<=g2 || g<lower_threshold)continue;
      else if(g>higher_threshold)edge->fel[j][i]=color;
      else if(g>lower_threshold && (edge->fel[j-1][i-1]==color  ||
				    edge->fel[j-1][i]==color ||
				    edge->fel[j-1][i+1]==color ||
				    edge->fel[j][i-1]==color)){	
	edge->fel[j][i]=color;}
      else if(g>lower_threshold){
	if(tmp_edge->fel[j-1][i-1]>0){
	  tmp_edge->fel[j][i]=tmp_edge->fel[j-1][i-1];
	}
	else if(tmp_edge->fel[j-1][i]>0){
	  tmp_edge->fel[j][i]=tmp_edge->fel[j-1][i];
	}
	else if(tmp_edge->fel[j-1][i+1]>0){
	  tmp_edge->fel[j][i]=tmp_edge->fel[j-1][i+1];
	}
	else if(tmp_edge->fel[j][i-1]>0){
	  tmp_edge->fel[j][i]=tmp_edge->fel[j][i-1];
	}
	else if(tmp_edge->fel[j][i+1]>0){
	  tmp_edge->fel[j][i]=tmp_edge->fel[j][i+1];
	}else {
	  tmp_edge->fel[j][i]=nb;cor_edge.push_back(0);nb++;
	}
      }
    }
  }

  for(int j=edge->y()-2;j>0;j--){
    for(int i=edge->x()-2;i>0;i--){
	if((edge->fel[j-1][i-1]==color ||
	    edge->fel[j-1][i]==color ||
	    edge->fel[j-1][i+1]==color ||
	    edge->fel[j][i-1]==color ||
	    edge->fel[j][i+1]==color ||
	    edge->fel[j+1][i-1]==color ||
	    edge->fel[j+1][i]==color ||
	    edge->fel[j+1][i+1]==color))cor_edge[(int)tmp_edge->fel[j][i]]=1;	
    }
  }  
  for(int j=edge->y()-2;j>0;j--){
    for(int i=edge->x()-2;i>0;i--){
      if(tmp_edge->fel[j][i]>0 && cor_edge[(int)tmp_edge->fel[j][i]]==1){	
	edge->fel[j][i]=color;
      }
    }
  } 

  for(int j=0;j<grad->y();j++){
    for(int i=0;i<grad->x();i++){
      if(edge->fel[j][i]>0)edge->fel[j][i]=grad->fel[j][i];
    }
  }
  delete grad;delete tmp_edge;
}


void newCorner(vector<CornerDescriptor *> &corners, float x, float y, float cornerness, float scale, float ext ){
  CornerDescriptor *cor;
  cor=new CornerDescriptor(x,y, cornerness, scale);
  if(ext==1)cor->setMax();
  else cor->setMin();
  corners.push_back(cor);
}

float getLap(DARY *lap, float fx, float fy){
  if(fx<lap->x()-1 && fy < lap->y()-1){
    int ix = (int)floor(fx);
    int iy = (int)floor(fy);
    float dx = fx-ix;
    float dy = fy-iy;
    float l =  (1.0 - dy)*((1.0 - dx)*lap->fel[iy][ix]+dx*lap->fel[iy][ix+1]) +
      dy*((1.0 - dx)*lap->fel[iy+1][ix] + dx *lap->fel[iy+1][ix+1]);
    return l;
  } else return 0.0;
}


float STEP=1.4;
void edge_points(vector<DARY *> edge,vector<DARY *> lap, vector<float> scales , vector<CornerDescriptor *> &corners){
  
  int row_nb;
  int col_nb, border=5;
  float fx,fy, lapthres=3, gradthres=3;
  CornerDescSequence all_desc;
  for(int i=0;i<scales.size();i++)all_desc.push_back(CornerDescList(0));
  vector<float > lapval;
  for(int i=1;i<edge.size()-1;i++){
    row_nb=edge[i]->y();col_nb=edge[i]->x();
    for (int row = border; row < row_nb-border; row++)
      for (int col = border; col < col_nb-border; col++){
	if(edge[i]->fel[row][col]>gradthres){
	  fx=col*STEP;
	  fy=row*STEP;
	  for(int l=i-1;l<lap.size();l++){
	    //cout << fx << " "<< fy << " "<<l << "   "<<lap[l]->x()<<"  "<<lap[l]->y()<<endl;
	    lapval.push_back(getLap(lap[l], fx, fy));
	    fx/=STEP;
	    fy/=STEP;
	  }
	  for(int l=1;l<lapval.size()-1;l++){
	    if(lapval[l]>lapval[l-1] && lapval[l]>lapval[l+1] && lapval[l]>lapthres){
	      newCorner(all_desc[i+l-1],scales[i]*col,scales[i]*row,edge[i]->fel[row][col],scales[i+l-1], 1);
	    }else if(lapval[l]<lapval[l-1] && lapval[l]<lapval[l+1] && lapval[l]<-lapthres){
	      newCorner(all_desc[i+l-1],scales[i]*col,scales[i]*row,edge[i]->fel[row][col],scales[i+l-1], -1);
	    }
	  }
	  lapval.clear();
	}	
      }        
  }
  float sc,grad;
  int dp,flag,nb=0;
  for(int i=1;i<all_desc.size()-1;i++){
    //cout << " scale "<< scales[i]<< " cor nb " << all_desc[i].size()<< endl;
    nb+=all_desc[i].size();
  }
  cout << " all points nb "<< nb<< endl;
  for(int i=1;i<all_desc.size()-1;i++){//REMOVE NEIGHBORING POINTS
    for(int c=0;c<all_desc[i].size();c++){
      cout << " \r  corner " << c << " of " << all_desc[i].size()<< "    "<< flush;
      sc=all_desc[i][c]->getCornerScale();
      dp=(int)(sc/2.0);
      fx=all_desc[i][c]->getX();
      fy=all_desc[i][c]->getY();
      grad=all_desc[i][c]->getCornerness();
      flag=1;
      for(int c2=c+1;c2<all_desc[i].size()&&flag;c2++){
	if(fabs(fx-all_desc[i][c2]->getX())<dp && fabs(fy-all_desc[i][c2]->getY())<dp){
	  if(grad>all_desc[i][c2]->getCornerness()){
	    all_desc[i].erase((std::vector<CornerDescriptor *>::iterator)&all_desc[i][c2]);c2--;
	  }else {
	    all_desc[i].erase((std::vector<CornerDescriptor *>::iterator)&all_desc[i][c]);c--;flag=0;
	  }
	}	
	
      }
      
    }     
  }
  nb=0;
  for(int i=1;i<all_desc.size()-1;i++){    
    for(int j=0;j<all_desc[i].size();j++){ 
      corners.push_back(all_desc[i][j]);
    }
    nb+=all_desc[i].size();
  }
  cout << " all points nb "<< nb<< endl;
}


void dog(DARY *img, DARY *simg, DARY *lap){

  for(int row=0;row<img->y();row++){
    for(int col=0;col<img->x();col++){
      lap->fel[row][col]=img->fel[row][col]-simg->fel[row][col];
    }
  }
}



void displayCirc(DARY *im, CornerDescriptor *cor, float color){

  int x=(int)cor->getX();
  int y=(int)cor->getY();
  for(int i=-2;i<=2;i++){
    if(y>0 && y<im->y() && y+i>0 && y+i<im->y() && x>0 && x<im->x() && x+i>0 && x+i< im->x())
      im->fel[y][x+i]=im->fel[y+i][x]=color;
  }
  int size=(int)(1*rint(cor->getCornerScale()));
  // cout << "OK1 " << x << "  " << y << "  " << size << endl;
  for(int i=-size;i<=size;i++){
    for(int j=0;j<=size;j++){
      int yi=(int)rint(sqrt((float)(size*size-i*i)));
      if(y+yi<im->y()&& y+yi>0 && y-yi>0 && y-yi<im->y() &&
	 y+i<im->y() && y+i>0 && x+i<im->x() && x+i>0 && 
	 x+yi<im->x()&& x+yi>0 && x-yi>0 && x-yi<im->x()){
	im->fel[y+yi][x+i]=color;
	im->fel[y-yi][x+i]=color;
	im->fel[y+i][x+yi]=color;
	im->fel[y+i][x-yi]=color;
      }
    }  
  }
}


    //edge_points(edge, cor2,  scale, (int)nb);

void edgeFeatureDetector(DARY *image, vector<CornerDescriptor *> &cor){
  DARY *dx,*dy,*edge,*lap;
  DARY *img = new DARY(image);
  vector<DARY *> vlap;
  vector<DARY *> vedge;
  vector<float > scales;
  float sizex=img->x();
  float sizey=img->y();
  vector<CornerDescriptor *> cor2;

  for(float scale = 1, nb=0;scale<30 && sizex>5 && sizey>5;scale*=STEP, nb++){
    dx= new DARY(img->y(),img->x());
    dy = new DARY(img->y(),img->x());
    lap = new DARY(img->y(),img->x());
    edge = new DARY(img->y(),img->x(),0.0);
    dX6(img,dx);
    dY6(img,dy);
    cannyEdgesGrad(dy, dx, edge, 2, 15);
    
    smooth3(img,dx);
    dog(img, dx, lap);
    scales.push_back(scale);
    vlap.push_back(lap);
    vedge.push_back(edge);
    cout << " sc "<< scale <<" size " << sizex <<  endl;
    //edge->write("edge.pgm");
    //lap->write("lap.pgm");
    //img->write("img.pgm");cout << scale << endl;getchar();
    
    delete img;
    sizex/=STEP;
    sizey/=STEP;
    img=new DARY((int)sizey+1,(int)sizex+1);
    img->scale(dx,STEP,STEP);
    
    delete dx;delete dy;
  }
  delete img;  

  edge_points(vedge,vlap, scales , cor);
  vlap.clear();
  vedge.clear();
  
}
