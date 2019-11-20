#include "edgeDescriptor.h"

int getS(int x, int y, int size){
  int s=0;
  if(y==0 && x<size){
    s=x;
  }else if(x==size && y<size){
    s=size+y;
  }else if(y==size && x>0){
    s=3*size-x;
  }else if(x==0 && y>0){
    s=4*size-y;
  }
  return s;
}

void getXY(int s, int &x, int &y,int size){
  //cout << s << "  " << size << "  " <<s/size << "  "<< s%size<< endl;
  int mod=s/size;
  int rest=s%size;
  if(mod==0){
    x=rest;y=0;
  }else if(mod==1){
    x=size;y=rest;
  }else if(mod==2){
    y=size;x=size-rest;
  }else if(mod==3){
    x=0;y=size-rest;
  }
  //getchar();
}

inline float square(float a){return a*a;}

void voteLine2(int x, int y, int x1, int y1, DARY *edge, DARY *h){
  float dx=x1-x;
  float dy=y1-y;
  float a,b,c, ab=0,cb=0;//ax+by+c=0, y=-a/b x -c/b, x=-b/a y -c/a
  if(dx==0){
    b=0;a=1;c=-x;
  } else if(dy==0){
    a=0;b=1;c=-y;
  }else {
    ab=dy/dx;    
    cb=((float)y)-ab*x;
    a=-ab;b=1;c=-cb;
  }
  //cout << 180*atan2(3.0,1.0)/M_PI<< endl;
  
  float ab2=sqrt(square(a)+square(b));
  float angle =atan2(dy,dx);
  //if(angle<0)angle=2*M_PI+angle;
  int s2=(int)rint((360.0*angle/M_PI))-180;
  if(s2<0)s2+=360;
  int s1=(int)rint((c/ab2))+((h->y())>>1);
  if(s1<0 || s1>h->y() || s2<0 || s2>h->x()){
    cout << "x " << x << ", y " << y << ", x1 " << x1 << ", y1 " << y1  <<  ", s1 " <<s1;
    cout  << ", s2 "<< s2 << ", angle " << 180*angle/M_PI << endl;getchar();
  }
  
  //cout << "dist1 "<< (a*x1+b*y1+c)/sqrt(square(a)+square(b))<< endl;
  //cout << "dist2 "<< (a*x2+b*y2+c)/sqrt(square(a)+square(b))<< endl;

  //cout << "x=" << x << ", y=" << y << ", x1=" << x1 << ", y1=" << y1;
  //cout <<", dx=" <<dx << ", dy="<<dy<< ", s1=" <<s1 << ", s2="<< s2 << ", angle=" << s2 << endl;
  //cout << " a=" << a << ", b=" << b << ", c=" << c<< ", ab=" << ab << ", cb=" << cb <<endl;getchar();
  
  if(h->fel[s1][s2]>0){return;}
  int size=edge->x();
  for(int y3=1;y3<size-1;y3++){
    for(int x3=1;x3<size-1;x3++){
      if(fabs((a*(x3+size)+b*(y3+size)+c)/ab2)<0.9){
	if(edge->fel[y3][x3]>0){
	  h->fel[s1][s2]+=edge->fel[y3][x3];
	}//else h->fel[s1][s2]-=1;
      }
    }
  }
  
}

void voteLine(int x, int y, int x1, int y1,DARY *edge, DARY *h){
  int x2, y2,x3,y3,x4,y4,x5,y5;  
  float dx=x1-x;
  float dy=y1-y;
  int s1=-1,s2=-1;
  float a,b,c, ab=0,cb=0;//ax+by+c=0, y=-a/b x -c/b, x=-b/a y -c/a
  int size = edge->x();
  int flag=1;
  cout << " x " << x << " y "<< y << " x1 " << x1 << " y1 " << y1 << endl;
  if(dx==0){
    x2=x;
    y2=0;    
    s1=x2;s2=3*size-3-x2;
    b=0;a=1;c=-x2;
    cout << "a x2 " << x2 << " y2 "<< y2 << " s1 " << s1 << " s2 " << s2 << endl;
  } else if(dy==0){
    y2=y;
    x2=0;    
    s1=size-1+y2;s2=4*size-4-y2;
    a=0;b=1;c=-y2;
    cout << "a x2 " << x2 << " y2 "<< y2 << " s1 " << s1 << " s2 " << s2 << endl;
  }else {
    ab=dy/dx;    
    cb=((float)y1)-ab*x1;
    a=-ab;b=1;c=-cb;
    x2=0;y2=(int)rint(ab*x2+cb);
    x3=size-1;y3=(int)rint(ab*x3+cb);
    y4=0;x4=(int)rint(-cb/ab);
    y5=size-1;x5=(int)rint((y5-cb)/ab);
    if(x4>=0 && x4<size-1){
      s1=x4;      
      cout << "a x4 " << x4 << " y4 "<< y4 << " s1 " << s1 << " s2 " << s2 << endl;
    }
    if(y3>=0 && y3<size-1){
      if(s1!=-1)s2=size-1+y3;
      else s1=size-1+y3;
      cout << "a x3 " << x3 << " y3 "<< y3 << " s1 " << s1 << " s2 " << s2 << endl;
    }
    if(x5>0 && x5<size){
      if(s1!=-1)s2=3*size-3-x5;  
      else s1=3*size-3-x5;
      cout << "a x5 " << x5 << " y5 "<< y5 << " s1 " << s1 << " s2 " << s2 << endl;      
    } 
    if(y2>0 && y2<size){
      s2=4*size-4-y2;      
      cout << "a x2 " << x2 << " y2 "<< y2 << " s1 " << s1 << " s2 " << s2 << endl;
    }
    if(s1<0 || s1>=h->x() || s2<0 || s2>=h->y()){
      cout << " x "<<  x << " y "<< y << " x1 "<< x1 << " y1 "<< y1 << " s1 "<< s1 << " s2 "<< s2 << endl;
      return;
    }    
  }
  cout << "OUT" << endl;
  getXY(s1,x4,y4,size);
  cout << "b x4 " << x4 << " y4 "<< y4 << " s1 " << s1 << " s2 " << s2 << endl;
  getXY(s2,x4,y4,size);
  cout << "b x4 " << x4 << " y4 "<< y4 << " s1 " << s1 << " s2 " << s2 << endl;

  getchar();
  //cout << "x=" << x << ", y=" << y << ", x1=" << x1 << ", y1=" << y1  << ", x2=" << x2;
  //cout << ", y2=" << y2  <<", dx=" <<dx << ", dy="<<dy<< ", s1=" <<size-s1 << ", s2="<< s2 << ", angle=" << -105-s2 << endl;
  //cout << " a=" << a << ", b=" << b << ", c=" << c<< ", ab=" << ab << ", cb=" << cb <<endl;getchar();
  
  //for(int )
  if(h->fel[s1][s2]>0){return;}
  int size1=edge->x();
  float ab2=sqrt(square(a)+square(b));
  for(int y3=1;y3<size1-1;y3++){
    for(int x3=1;x3<size1-1;x3++){
      if(edge->fel[y3][x3]>0){
	if(fabs((a*(x3+size1)+b*(y3+size1)+c)/ab2)<1){
	//cout << "x3 " <<x3<<  " y3 " <<y3 << " val "<< edge->fel[y3][x3]<< " dist "<< ((a*(x3+size1)+b*(y3+size1)+c)/ab2) << endl;
	  h->fel[s1][s2]+=edge->fel[y3][x3];
	}
      }
    }
  }
  
}


static const float MPI2= M_PI/2;



int hough_flag=0;
DARY *hough;



void getLine(DARY *edge,float &al, float &bl, float &cl){
  int size = edge->x();
  int sizeh =2*10*(int)rint(sqrt(8.0*size*size)/10.0);//4*size; distance from (0,0)
  int sizea = 361;//nb of angles
  if(!hough_flag){
    hough_flag=1;
    hough= new DARY(sizeh,sizea);//cout << "init "<< endl;
  }//else cout << "not init"<< endl;
  hough->set(0.0);
  DARY *ed=new DARY(edge);
  int x1,y1,x2,y2;
  float sizes=size*size/9.0;
  for(int y1=1;y1<edge->x()-1;y1++){
    for(int x1=1;x1<edge->y()-1;x1++){
      if(ed->fel[y1][x1]>0){
	ed->fel[y1][x1]=0;
	for(int y2=y1;y2<edge->x()-1;y2++){
	  for(int x2=0;x2<edge->y()-1;x2++){
	    if(ed->fel[y2][x2]>0 && (square((float)x1-x2)+square((float)y1-y2))>sizes){
	      voteLine2(x1+size,y1+size,x2+size,y2+size,edge, hough);	      
	      //voteLine(x1,y1,x2,y2,edge, hough);	      
	    }
	  }      
	}        
      }
    }
  }
  int size1=hough->y();
  int size2=hough->x();
  int s1=0,s2=0;float max=0;
  for(int j=0;j<size1;j++){
    for(int i=0;i<size2;i++){
      if(max<hough->fel[j][i]){
	max=hough->fel[j][i];s1=j;s2=i;
      }
    }
  }

  s1-=(hough->y()>>1);
  float angle = s2+180;
  if(angle>360)angle-=360;
  //cout << ", s1=" <<s1 << ", s2="<< s2 << " max " << max << ", angle=" << angle << endl;
  
  float a=0,b=0,c=0,ab=0,cb=0;
  int x3,y3,x4,y4;

  


    if(angle==0 || angle==360){
    a=0;b=1;c=-s1;
    //x1=0;y1=-s1;
    //x2=size-1;y2=y1;
  }else if(angle==180){
    a=1;b=0;c=-s1;
    //y1=0;x1=s1;
    //y2=size-1;x2=x1;
  }else{
    angle=(M_PI*angle)/360.0;
    ab=tan(angle);
    if(angle>MPI2)angle=M_PI-angle;
    cb=-s1/cos(angle);
    b=1;
    c=-cb;
    a=-ab;
    //x1=size;y1=ab*x1+cb;x1-=size;y1-=size;
    //x2=2*size-1;y2=ab*x2+cb;x2-=size;y2-=size;
    //y3=size;x3=(y3-cb)/ab;x3-=size;y3-=size;
    //y4=2*size-1;x4=(y4-cb)/ab;x4-=size;y4-=size;
  }
  


  
  float ab2=sqrt(square(a)+square(b)); 
  float dist=0;
  for(int j=1;j<edge->y()-1;j++){
    for(int i=1;i<edge->x()-1;i++){
      dist=fabs((a*(i+size)+b*(j+size)+c)/ab2);
      if(edge->fel[j][i]>0 && 
	 (dist<1 || (dist<2 && (edge->fel[j][i-1]==255 ||
				edge->fel[j-1][i-1]==255||
				edge->fel[j-1][i]==255||
				edge->fel[j-1][i+1]==255)))){
	edge->fel[j][i]=255;
      }    
    }
  }
  for(int j=edge->y()-2;j>0;j--){
    for(int i=edge->x()-2;i>0;i--){
      dist=fabs((a*(i+size)+b*(j+size)+c)/ab2);
      if(edge->fel[j][i]>0 && 
	 (dist<1 || (dist<2 && (edge->fel[j][i+1]==255 ||
				edge->fel[j+1][i-1]==255||
				edge->fel[j+1][i]==255||
				edge->fel[j+1][i+1]==255)))){
	edge->fel[j][i]=255;
      }    
    }
  }


    

  if(x1>=0 && x1<size && y1>=0 && y1<size){
    //edge->fel[y1][x1]=255;
  }
  if(x2>=0 && x2<size && y2>=0 && y2<size){
    //edge->fel[y2][x2]=255;
  }
  if(x3>=0 && x3<size && y3>=0 && y3<size){
    //edge->fel[y3][x3]=255;
  }
  if(x4>=0 && x4<size && y4>=0 && y4<size){
    //edge->fel[y4][x4]=255;
  }

 
  //cout << "ab "<< ab << " cb "<< cb<< " size " << size <<endl;
  //cout << "x1 " << x1 << " y1 " << y1 << endl;
  //cout << "x2 " << x2 << " y2 " << y2 << endl;
  //cout << "x3 " << x3 << " y3 " << y3 << endl;
  //cout << "x4 " << x4 << " y4 " << y4 << endl;
  
  //hough->normalize(0,max);
  //hough->write("hough.pgm");
}


const int xbin2=50; //nb of location bins in x and y coordinates
const int xobin2=12; // xbin*obin
const int hsize2=xbin2>>1;//half of the location bin size
const int obin2 = 4;//nb of orientation bins
const float fraco2=obin2/M_PI; //scaling factor for the orientation 

const int xbin1=5; //nb of location bins in x and y coordinates
const int xobin1=40; // xbin*obin
const int hsize1=xbin1>>1;//half of the location bin size
const int obin1 = 8;//nb of orientation bins
const float fraco1 = obin1/M_2PI; //scaling factor for the orientation 

const int vsize2=36;	
const int vsize1=400;
const int vsize=436;				
const int vsizeg=236;				
const int vsizel=436;				

void EdgeDescriptor::computeComponents(DARY *gr1, DARY *ogr1,DARY *lap1, DARY *olap1,
				       int x1, int y1, int size1,
				       DARY*gr2, DARY*ogr2, DARY*lap2, DARY*olap2, 
				       int x2, int y2, int size2){
  if(gr1==NULL || gr2==NULL || lap1==NULL || lap2==NULL ||
     ogr1==NULL || ogr2==NULL || olap1==NULL || olap2==NULL){return;} 
  if(x1-size1<0 || y1-size1<0 || x1+size1>=gr1->x() || y1+size1>=gr1->y())return;
  if(x2-size2<0 || y2-size2<0 || x2+size2>=gr2->x() || y2+size2>=gr2->y())return;
  

  allocVec(vsize);//3x3x4+5x5x8+5x5x8= 436
  d_scale=c_scale;
  
  float gval=0;
  float ogval=0;
  float tmp,tmp1,xt,yt,dxt,dyt,dot,v000,v010,v001,v011,v100,v110,v101,v111;
  int xti,yti,ix,iy,iog,index,imx,imy;
  
  /* if(c_scale<3)return;
     DARY * tgr1 = new DARY(lap1);
     tgr1->fel[y1][x1]=255;
     tgr1->fel[y1-size1][x1-size1]=255;
     tgr1->fel[y1-size1][x1+size1]=255;
     tgr1->fel[y1+size1][x1+size1]=255;
     tgr1->fel[y1+size1][x1-size1]=255;
     tgr1->write("gr1.pgm"); 
     DARY * tgr2 = new DARY(gr2);
     // tgr2->fel[y2][x2]=255;
     // tgr2->fel[y2-size2][x2-size2]=255;
     // tgr2->fel[y2-size2][x2+size2]=255;
     // tgr2->fel[y2+size2][x2+size2]=255;
     // tgr2->fel[y2+size2][x2-size2]=255;
  tgr2->write("gr2.pgm");
  cout << "OK " << c_scale<< endl;
 */
  
  float frac = 0.6*xbin2/size2;//scaling factor for the location
  int inda0,inda1,indx0, indy0,indx1, indy1, indxo, indyxo;

  DARY *patch = new DARY(xbin2,xbin2,0.0);
  DARY *patch2 = new DARY(xbin2,xbin2,0.0);
  //angle=0;
  float lecos = cos(angle);
  float lesin = sin(angle);	        
  float vec0x = frac*lecos; 
  float vec0y = frac*lesin;
  float vec1x = -lesin*frac;
  float vec1y = lecos*frac;
  //cout << vec0x << " " << vec0y << " " << vec1x << " " << vec1y << endl;
  for(int yi=-size2;yi<=size2;yi++){
    for(int xi=-size2;xi<=size2;xi++){
      imx=x2+xi;imy=y2+yi; 
      gval=gr2->fel[imy][imx];

      ogval=ogr2->fel[imy][imx]-angle;
      if(ogval<-M_PI)ogval+=M_2PI;
      else if(ogval<0)ogval+=M_PI;
      if(gval<1)continue;

      ogval*=fraco2;
      iog=floorf(ogval);
      dot=ogval-iog;            
      inda0=iog;inda1=((iog+1)<obin2)?iog+1:0;

      xt = xi*vec0x + yi*vec0y;
      yt = xi*vec1x + yi*vec1y;

      xti=floorf(xt);yti=floorf(yt);
      dxt=xt-xti;dyt=yt-yti;

      tmp1=dyt*gval;
      tmp=dxt*tmp1;
      v111=dot*tmp;
      v011=tmp-v111;
      
      tmp=(1-dxt)*tmp1;
      v110=dot*tmp;
      v010=tmp-v110;
      

      tmp1=(1-dyt)*gval;
  	    tmp=dxt*tmp1;
      v101=dot*tmp;
      v001=tmp-v101;
      
      tmp=(1-dxt)*tmp1;
      v100=dot*tmp;
      v000=tmp-v100;

      indx0 = xti+hsize2;indy0=yti+hsize2;//to strt from 0 instead of -1
      indx1 = indx0+1;indy1=indy0+1;
	
      int an=3;
      if(indy0>=0 && indy0<xbin2){
	indyxo = indy0*xobin2;
	if(indx0>=0 && indx0<xbin2){
	  indxo=indyxo+indx0*obin2;
	  //vec[indxo+inda0]+=v000;
	  //vec[indxo+inda1]+=v100;
	  if(inda0==an)patch->fel[indy0][indx0]+=v000;
	  else if(inda1==an)patch->fel[indy0][indx0]+=v100;
	  if(inda0==1)patch2->fel[indy0][indx0]+=v000;
	  else patch2->fel[indy0][indx0]+=v100;
	  
	} 
	if(indx1>=0 && indx1<xbin2){
	  indxo=indyxo+indx1*obin2;
	  //vec[indxo+inda0]+=v001;
	  //vec[indxo+inda1]+=v101;
	  if(inda0==an)patch->fel[indy0][indx1]+=v001;
	  else  if(inda1==an)patch->fel[indy0][indx1]+=v101;
	  if(inda1==0)patch2->fel[indy0][indx1]+=v001;
	  else  patch2->fel[indy0][indx1]+=v101;
	  
	}
      }
      if(indy1>=0 && indy1<xbin2){
	indyxo = indy1*xobin2;
	if(indx0>=0 && indx0<xbin2){
	  indxo=indyxo+indx0*obin2;
	  //vec[indxo+inda0]+=v010;
	  //vec[indxo+inda1]+=v110;
	  if(inda0==an)patch->fel[indy1][indx0]+=v010;
	  else  if(inda1==an)patch->fel[indy1][indx0]+=v110;
	  if(inda1==0)patch2->fel[indy1][indx0]+=v010;
	  else  patch2->fel[indy1][indx0]+=v110;
	  
	}
	if(indx1>=0 && indx1<xbin2){
	  indxo=indyxo+indx1*obin2;
	  //vec[indxo+inda0]+=v011;
	  //vec[indxo+inda1]+=v111;
	  
	  if(inda0==an)patch->fel[indy1][indx1]+=v011;
	  else  if(inda1==an)patch->fel[indy1][indx1]+=v111;
	  if(inda1==0)patch2->fel[indy1][indx1]+=v011;
	  else  patch2->fel[indy1][indx1]+=v111;
	  
	}
        }
      
      /*
       cout <<" xi " << xi << ", yi "<< yi << ", xt " <<xt  << ", yt " <<yt << ", angle " << (180/M_PI)*ogr2->fel[imy][imx]<< " ogval " << ogval<<endl;
      cout <<" xti " << xti << " yti " << yti << endl;
      cout << "gval " << gval << endl;

      cout << "dxt " << dxt << " dyt " << dyt <<  " iog " << iog << " dot " << dot << endl;
      cout <<v000 <<   "  " <<v001 <<  endl;
      cout <<v010 <<   "  " <<v011 <<endl <<endl;       
      cout <<v100 <<   "  " <<v101 <<  endl;
      cout <<v110 <<   "  " <<v111 <<endl <<endl;     
      cout << "sum dot " << v000+v010+v001+v011<<endl<<  "sum 1-dot " << v100+v110+v101+v111<<endl;
   	   cout << " sum "<< v000+v010+v001+v011+v100+v110+v101+v111<<endl;
                      
      cout << "ind angle1 " <<  inda0 <<  " ind angle2 " <<  inda1 << endl;
      cout << "ind x " <<  indx0 <<  " ind y " <<  indy0 << endl;
      getchar();
      */  
    } 
    }
  patch->write("patch.pgm");patch2->write("patch2.pgm");cout << "OK "<< endl;getchar();
  double sqlen=0;
  for(int i=0;i<vsize2;i++){
    sqlen+=(vec[i]*vec[i]);
  }
  if(sqlen<10)return;
  float fac=1.0/sqrt(sqlen);
  for (int i = 0; i < vsize2; i++)
    vec[i] *= fac;
  
  // state=1;return;
  frac=0.6*xbin1/size1;

   vec0x = frac*lecos; 
   vec0y = frac*lesin;
   vec1x = -lesin*frac;
   vec1y = lecos*frac;

   float lval,l111,l011,l110,l010,l101,l001,l100,l000,iol,dol,olval;
   int linda0,linda1;
  for(int yi=-size1;yi<size1;yi++){
    for(int xi=-size1;xi<size1;xi++){
      imx=x1+xi;imy=y1+yi; 
      gval=gr1->fel[imy][imx];
      lval=fabsf(lap1->fel[imy][imx]);
      
      ogval=ogr1->fel[imy][imx]+M_PI-angle;
      olval=olap1->fel[imy][imx]+M_PI-angle;
      if(ogval<0)ogval+=M_2PI;
      if(olval<0)olval+=M_2PI;
      if(ogval>M_2PI)ogval-=M_2PI;
      if(olval>M_2PI)olval-=M_2PI;
      if(gval<1 || lval<1)continue;
      
      ogval*=fraco1;
      olval*=fraco1;
      
      iog=floorf(ogval);
      dot=ogval-iog;
      iol=floorf(olval);
      dol=olval-iol;
      inda0=iog;inda1=((iog+1)<obin1)?iog+1:0;
      if(inda0<0 || inda1<0)continue;
      linda0=iol;linda1=((iol+1)<obin1)?iol+1:0;
      if(linda0<0 || linda1<0)continue;
      
      xt = xi*vec0x + yi*vec0y;
      yt = xi*vec1x + yi*vec1y;
      xti=floorf(xt);yti=floorf(yt);
      dxt=xt-xti;dyt=yt-yti;

      tmp1=dxt*dyt;
      tmp=tmp1*gval;
      v111=dot*tmp;
      v011=tmp-v111;
      tmp=tmp1*lval;
      l111=dol*tmp;
      l011=tmp-l111;

      tmp1=(1-dxt)*dyt;
      tmp=tmp1*gval;
      v110=dot*tmp;
      v010=tmp-v110;
      tmp=tmp1*lval;
      l110=dol*tmp;
      l010=tmp-l110;
     
      tmp1=(1-dxt)*dyt;
      tmp=tmp1*gval;
      v101=dot*tmp;
      v001=tmp-v101;
      tmp=tmp1*lval;
      l101=dol*tmp;
      l001=tmp-l101;
     
      tmp1=(1-dyt)*(1-dxt);
      tmp=tmp1*gval;
      v100=dot*tmp;
      v000=tmp-v100;
      tmp=tmp1*lval;
      l100=dol*tmp;
      l000=tmp-l100;
            
      indx0 = xti+hsize1;indy0=yti+hsize1;//to strt from 0 instead of -1
      indx1 = indx0+1;indy1=indy0+1;
      
      if(indy0>=0 && indy0<xbin1){
	indyxo = indy0*xobin1;
	if(indx0>=0 && indx0<xbin1){
	  indxo=indyxo+indx0*obin1;
	  vec[vsize2+indxo+inda0]+=v000;
	  vec[vsize2+indxo+inda1]+=v100;
	  vec[vsizeg+indxo+linda0]+=l000;
	  vec[vsizeg+indxo+linda1]+=l100;
	  /*if(inda0==0)patch->fel[indy0][indx0]+=l000;
	  else if(inda1==0)patch->fel[indy0][indx0]+=l100;
	  if(inda0==2)patch2->fel[indy0][indx0]+=l000;
	  else if(inda1==2)patch2->fel[indy0][indx0]+=l100;
	  */
	  
	} 
	if(indx1>=0 && indx1<xbin1){
	  indxo=indyxo+indx1*obin1;
	  vec[vsize2+indxo+inda0]+=v001;
	  vec[vsize2+indxo+inda1]+=v101;
	  vec[vsizeg+indxo+linda0]+=l001;
	  vec[vsizeg+indxo+linda1]+=l101;
	  /*if(inda0==0)patch->fel[indy0][indx1]+=l001;
	  else  if(inda1==0)patch->fel[indy0][indx1]+=l101;
	  if(inda0==2)patch2->fel[indy0][indx1]+=l001;
	  else  if(inda1==2)patch2->fel[indy0][indx1]+=l101;	    
	  */
	}
      }
      if(indy1>=0 && indy1<xbin1){
	indyxo = indy1*xobin1;
	if(indx0>=0 && indx0<xbin1){
	  indxo=indyxo+indx0*obin1;
	  vec[vsize2+indxo+inda0]+=v010;
	  vec[vsize2+indxo+inda1]+=v110;
	  vec[vsizeg+indxo+linda0]+=l010;
	  vec[vsizeg+indxo+linda1]+=l110;
	  /*if(inda0==0)patch->fel[indy1][indx0]+=l010;
	  else if(inda1==0)patch->fel[indy1][indx0]+=l110;
	  if(inda0==2)patch2->fel[indy1][indx0]+=l010;
	  else  if(inda1==2)patch2->fel[indy1][indx0]+=l110;	  
	  */
	}
	if(indx1>=0 && indx1<xbin1){
	  indxo=indyxo+indx1*obin1;
	  vec[vsize2+indxo+inda0]+=v011;
	  vec[vsize2+indxo+inda1]+=v111;	  
	  vec[vsizeg+indxo+linda0]+=l011;
	  vec[vsizeg+indxo+linda1]+=l111;	  
	  /*if(inda0==0)patch->fel[indy1][indx1]+=l011;
	  else if(inda1==0)patch->fel[indy1][indx1]+=l111;
	  if(inda0==2)patch2->fel[indy1][indx1]+=l011;
	  else  if(inda1==2)patch2->fel[indy1][indx1]+=l111;	  
	  */
	}
      }            
    }
  }
  
  sqlen=0;
  for(int i=vsize2;i<vsizeg;i++){
    sqlen+=(vec[i]*vec[i]);
  }
  fac=1.0/sqrt(sqlen);
  for (int i = vsize2; i < vsizeg; i++)
    vec[i] *= fac;
  sqlen=0;
  for(int i=vsizeg;i<vsizel;i++){
    sqlen+=(vec[i]*vec[i]);
  }
  fac=1.0/sqrt(sqlen);
  for (int i = vsizeg; i < vsizel; i++)
    vec[i] *= fac;
  
  //patch->write("patch.pgm");patch2->write("patch2.pgm");cout << "OK "<< endl;getchar();
  
  //for(int i=0;i<size;i++)cout << vec[i]<< endl;
  //imgn->write("point.pgm");
  //edge->write("edge.pgm");cout << "points.pgm wrote"<< endl;getchar();
  state=1;
} 
