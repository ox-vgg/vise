#include "homography.h"

void convertEllips(float a, float b, float c, float &scale, float &m11, float &m12, float &m21, float &m22){
  Matrix U(2,2),Vi,V,D;
   U(1,1)=a;
   U(2,1)=b;U(1,2)=b;
   U(2,2)=c;
   U.svd(Vi,D,V);
   D(1,1)=(1.0/sqrt(D(1,1)));
   D(2,2)=(1.0/sqrt(D(2,2)));
   scale=sqrt(D(2,2)*D(1,1));

   D.tabMat[2][2]/=scale;
   D.tabMat[1][1]/=scale;
   U=V*D*V.transpose();

   m11=U(1,1);
   m12=U(1,2);
   m21=U(2,1);
   m22=U(2,2);
}


void loadVibesCorners(const char* points1, vector<CornerDescriptor*> &cor1){

    CornerDescriptor* cor;
    ifstream input1(points1);
    if(!input1)return;
    float cor_nb1,size; 
    float x,y,a,b,c;
    input1 >> x;//cout <<x<< endl;
    input1 >> cor_nb1; //cout<< x<< endl;getchar();  
    if(cor_nb1==0)return;
    float scale,m11,m12,m21,m22;
    do{
      cor = new CornerDescriptor();
      input1>>x; input1>>y;input1>>a;input1>>b;input1>>c;
      //cout << a  << "  " << b << "  "<< c << endl;
      convertEllips(a,b,c,scale,m11,m12,m21,m22);
      //cout << m11 <<" "<<  m12 << " "<< m21 << " " << m22<< endl;getchar();
      cor->setX_Y(x,y);cor->setCornerScale(scale);
      cor->setMi(m11,m12,m21,m22);
      cor1.push_back(cor);      
    }while(!input1.eof());
    delete cor1[(int)cor1.size()-1];
    cor1.erase((std::vector<CornerDescriptor*>::iterator) &cor1[(int)cor1.size()-1]);
    if((int)cor_nb1!=(int)cor1.size()){
      cout << "warning:"<< endl<<"in header: "<<cor_nb1 << ", in file: "<< cor1.size()<< endl; 
    }

}



void getAffMat(CornerDescriptor * cor, Matrix *H,  Matrix &AFF);
/*************computes homography correspondence**************/
void getHPoint(float x_in, float y_in, Matrix *H, float &x_out, float &y_out){
  x_out = H->tabMat[1][1]*x_in + H->tabMat[1][2]*y_in +H->tabMat[1][3];
  y_out = H->tabMat[2][1]*x_in + H->tabMat[2][2]*y_in +H->tabMat[2][3];
  float t= H->tabMat[3][1]*x_in + H->tabMat[3][2]*y_in +H->tabMat[3][3];
  x_out /=t;
  y_out /=t; 
}
 

int checkIfCorrect(float x, float y, float scale, CornerDescriptor *cd,
		   float dist_max, float orig_scale, float scale_error){
  float dist_x=cd->getX()-x;
  float dist_y=cd->getY()-y;
  float dist=sqrt(dist_x*dist_x+dist_y*dist_y);
  float sc = scale/(cd->getCornerScale());
  if(dist<dist_max && (fabs(sc-orig_scale)/orig_scale) < scale_error)
    return 1;
  else 
    return 0;
}


/*********** returns angle and scale from homography**************/
void getScaleAngle(Matrix *H, float &scale, float &angle){
  
    float x3=(100*H->tabMat[1][1]+H->tabMat[1][3])/(100*H->tabMat[3][1]+H->tabMat[3][3]);
    float y3=(100*H->tabMat[2][1]+H->tabMat[2][3])/(100*H->tabMat[3][1]+H->tabMat[3][3]);
    float x2=H->tabMat[1][3]/H->tabMat[3][3];
    float y2=H->tabMat[2][3]/H->tabMat[3][3];
    
    float dx=x3-x2;
    float dy=y3-y2;
    scale=fabs(100.0/(sqrt(dx*dx+dy*dy)));
    angle=atan2(dy,dx);   
    
}
/*********** returns angle and scale from homography**************/
void getScaleAngle( float x, float y, Matrix *H, float &scale, float &angle){
  
  float x2=x+10;
  float y2=y;
  float xH,yH,x2H,y2H;
  getHPoint(x,y,H,xH,yH);
  getHPoint(x2,y2,H,x2H,y2H);
  
  float dxH=x2H-xH;
  float dyH=y2H-yH;
  scale=(10.0/(sqrt(dxH*dxH+dyH*dyH)));
  angle=atan2(dyH,dxH);
}


/*********** returns angle and scale from homography**************/
void getAngle(CornerDescriptor* cor1,  Matrix *H, float &angle){
  
    float x,y,x2,y2;
    float dx=2*cor1->getCornerScale();
    getHPoint(cor1->getX(), cor1->getY(), H, x, y);
    getHPoint(cor1->getX()+dx, cor1->getY(), H, x2, y2);
    float dx2=x2-x;
    float dy2=y2-y;    
    angle=atan2(dy2,dx2);   
}


/**************check if angle is correct******************************/
int  checkAngle(CornerDescriptor* cor1, CornerDescriptor* cor2, float orig_angle, float max_error){
  int score=0; 
  float da=cor2->getAngle()-cor1->getAngle();
  if(da<(-M_PI))da+=M_2PI;
  if(da>(M_PI))da-=M_2PI;
  if(fabs(da-orig_angle)<max_error)score=1;//max_error=0.17
  return score;          
}


/************check if matches agree with homography******************/
int checkHomog(CornerDescriptor* cor1,CornerDescriptor* cor2,Matrix *H, float dist_max, float &dist){
  float x,y;//cout << "OK"<< endl;
  getHPoint(cor1->getX(),cor1->getY(),H,x,y);
  float dist_x=cor2->getX()-x;
  float dist_y=cor2->getY()-y;
  dist=sqrt(dist_x*dist_x+dist_y*dist_y);
  if(dist<dist_max)return 1;
  else return 0;
}

/************check if is near to epipolar line******************/
int checkEpip(CornerDescriptor* cor1,CornerDescriptor* cor2,Matrix *F,float dist_max, float &dist){
    float *point = new float[3];
    float *abc = new float[3];
    abc[0]=0;abc[1]=0;abc[2]=0;
    point[0]=cor1->getX();
    point[1]=cor1->getY();
    point[2]=1;
    for (int j=0;j<3;j++){
      for (int i=0;i<3;i++){
	abc[j]+=F->tabMat[j+1][i+1]*point[i];
      }
    }
    dist=fabs((abc[0]*cor2->getX()+abc[1]*cor2->getY()+abc[2])/sqrt(abc[0]*abc[0]+abc[1]*abc[1]));
    delete [] point; delete [] abc;
    //    cout << point[0] << " " <<point[1] << "    " << cor2->getX() <<  " " << cor2->getY();
    //    cout << " dist "<<dist <<" a  "<< abc[0] <<" b  "<<abc[1]<< "  c  "<<abc[2]<< endl;    
    if(dist<dist_max)return 1;
    else return 0;
}

void removeFloatMatches(vector<CornerDescriptor*> &cor1, 
			vector<CornerDescriptor*> &cor2){

  for(unsigned int i=0;i<cor2.size()-1;i++){
    for(unsigned int j=i+1;j<cor2.size();j++){
      if(cor2[i]==cor2[j] || cor1[i]==cor1[j]){
	cor2.erase((std::vector<CornerDescriptor*>::iterator)&cor2[j]);
	cor1.erase((std::vector<CornerDescriptor*>::iterator)&cor1[j]);j--;
      }
    }
  }
}

void removeOutLiers(vector<CornerDescriptor*> &cor1, Matrix *H, float width, float height, float margin){
  
  float x,y;

  for(unsigned int c1=0;c1<cor1.size();c1++){
    getHPoint(cor1[c1]->getX(),cor1[c1]->getY(),H,x,y);
    if(x<margin || y<margin ||
       x>(width-margin) || y>(height-margin)){
      cor1.erase((std::vector<CornerDescriptor*>::iterator)&cor1[c1]);c1--;
    }       
  }  
  
}


/**************check angles, removes wrong angles, computes mean error*****************/
float  checkAngleByHomography(vector<CornerDescriptor*> &cor1,  
			       vector<CornerDescriptor*> &cor2, 
			       Matrix *H,float max_angle_error){
  
  if(cor1.size()!=cor2.size()){cout << " different list size " << endl;return 0.0;}
  float scale,orig_angle;
 
  float da;
  float sum=0;
  for(unsigned int c=0;c<cor1.size();c++){
    getScaleAngle(cor1[c]->getX(),cor1[c]->getY(),H, scale , orig_angle);
    da=cor2[c]->getAngle()-cor1[c]->getAngle();
    if(da<(-M_PI))da+=M_2PI;
    if(da>(M_PI))da-=M_2PI;
    sum+=fabs(da-orig_angle);
    if(fabs(da-orig_angle) > max_angle_error){
      cor2.erase((std::vector<CornerDescriptor*>::iterator)&cor2[c]);
      cor1.erase((std::vector<CornerDescriptor*>::iterator)&cor1[c]);
      c--;
    }
  }
  sum/=cor1.size();
  return sum;          
}


void checkScaleByHomography(vector<CornerDescriptor *> &cor1, vector<CornerDescriptor *> &cor2, Matrix *H, float max_scale_error){
  if(cor1.size()!=cor2.size()){cout << " different list size " << endl;return;}
    float sc;
    float c1_scale;
    float orig_scale,orig_angle; 
    float scale_error=0;

    for(unsigned int c1=0;c1<cor1.size();c1++){
      cout << "\r  " << c1 << " of "<< cor1.size() << flush;
      getScaleAngle(cor1[c1]->getX(),cor1[c1]->getY(),H,orig_scale, orig_angle);
	c1_scale=cor1[c1]->getCornerScale();
	sc = (cor1[c1]->getCornerScale())/(cor2[c1]->getCornerScale());
	scale_error=sc/orig_scale;
	scale_error=scale_error<1?scale_error:(1.0/scale_error);
	scale_error=1-scale_error;
	if(scale_error > max_scale_error){
	  cor2.erase((std::vector<CornerDescriptor*>::iterator)&cor2[c1]);
	  cor1.erase((std::vector<CornerDescriptor*>::iterator)&cor1[c1]);
	  c1--;
	}
    }	                   
}


void checkPointsByEpipolar(vector<CornerDescriptor *> &cor1, vector<CornerDescriptor *> &cor2, 
			     Matrix *F, float max_dist){
  float dist;
  if(cor1.size()!=cor2.size()){cout << " different list size " << endl;return;}
    for(unsigned int c1=0;c1<cor1.size();c1++){
      if(!checkEpip(cor1[c1],cor2[c1],F,max_dist,dist)){
	  cor1.erase((std::vector<CornerDescriptor*>::iterator)&cor1[c1]);
	  cor2.erase((std::vector<CornerDescriptor*>::iterator)&cor2[c1]);	  
	  c1--;
	}                
    }     
}

void checkPointsByHomography(vector<CornerDescriptor *> &cor1, vector<CornerDescriptor *> &cor2, 
			     Matrix *F, float max_dist){
  float dist;
  if(cor1.size()!=cor2.size()){cout << " different list size " << endl;return;}
    for(unsigned int c1=0;c1<cor1.size();c1++){
	if(!checkHomog(cor1[c1],cor2[c1],F,max_dist,dist)){
	  cor1.erase((std::vector<CornerDescriptor*>::iterator)&cor1[c1]);
	  cor2.erase((std::vector<CornerDescriptor*>::iterator)&cor2[c1]);	  
	  c1--;
	}                
    }     
}
  
 

void matchPointsByHomography(vector<CornerDescriptor *> &cor1, vector<CornerDescriptor *> &cor2,
			     vector<CornerDescriptor *> &cor1_tmp, vector<CornerDescriptor *> &cor2_tmp, 
			     Matrix *H, float max_dist){
    float x,y;
    float dist,dist_x,dist_y;
    
    unsigned int size1=cor1.size();
    unsigned int size2=cor2.size();
    max_dist=max_dist*max_dist;
    for(unsigned int c1=0;c1<size1;c1++){
      cout << "\r  " << c1 << " of "<< size1 << flush;
	getHPoint(cor1[c1]->getX(),cor1[c1]->getY(),H,x,y);
	//cout << cor1[c1]->getX()<< " " <<cor1[c1]->getY()<< " to  "<<x<< " " << y<< endl;
	//getchar();
	for(unsigned int c2=0;c2<size2;c2++){	
	    dist_x=cor2[c2]->getX()-x;
	    dist_y=cor2[c2]->getY()-y;
	    dist=(dist_x*dist_x+dist_y*dist_y);	    
	    if(dist<max_dist){
		cor1_tmp.push_back(cor1[c1]);
		cor2_tmp.push_back(cor2[c2]);
	    }
	}                
    }     
}

/************COMPUTES HOMOGRAPHY MATRIX******************/
void computeHomography2(vector<CornerDescriptor *> cor1, 
		       vector<CornerDescriptor *> cor2, Matrix *H){
   

    int nb_of_Tokens=cor1.size();    
    Matrix *A = new Matrix(nb_of_Tokens*2,8);
    Matrix *B = new Matrix(nb_of_Tokens*2,1);

    int row=0;
    for(int i=0;i<nb_of_Tokens;i++){
	row=2*i+1;
	A->tabMat[row][1]=cor1[i]->getX();
 	A->tabMat[row][2]=cor1[i]->getY();
	A->tabMat[row][3]=1.0;
	A->tabMat[row][4]=0.0;
	A->tabMat[row][5]=0.0;
	A->tabMat[row][6]=0.0;
	A->tabMat[row][7]=-(cor1[i]->getX()*cor2[i]->getX());
	A->tabMat[row][8]=-(cor1[i]->getY()*cor2[i]->getX());
	B->tabMat[row][1]=cor2[i]->getX();
	row++;
	A->tabMat[row][1]=0.0;
	A->tabMat[row][2]=0.0;
	A->tabMat[row][3]=0.0;
	A->tabMat[row][4]=cor1[i]->getX();
	A->tabMat[row][5]=cor1[i]->getY();
	A->tabMat[row][6]=1.0;
	A->tabMat[row][7]=-(cor1[i]->getX()*cor2[i]->getX());
	A->tabMat[row][8]=-(cor1[i]->getY()*cor2[i]->getX());
	B->tabMat[row][1]=cor2[i]->getY();
    }
    //Eigenvalues and eigenvectors
    Matrix U,W,V;
    A->svd(U,W,V);

    float val;
    float *vec = new float[9];
    
    //Di-inverse * U-transpose * B
    for(int nb=1; nb<9; nb++){
      val=0;
      for(int i=1; i<=row; i++)
	val+=(U.tabMat[i][nb]*B->tabMat[i][1]);
      vec[nb]=val/W(nb,nb);
    }

    //V*vec
    float *X = new float[10];
    for(int j=1; j<9; j++){
	 val=0;
	 for(int i=1; i<9; i++)
	   val+=(V.tabMat[j][i]*vec[i]);
		   
	 X[j]=val;
     }
     X[9]=1;
     
     for(int j=1,k=0;j<=3;j++,k+=3)
	 for(int i=1;i<=3;i++)
	     H->tabMat[j][i]=X[k+i]; //write the homography matrix
         
     delete []vec; delete []X;//delete &U;delete &V;delete &W; 
     delete A, delete B;

  }
/************COMPUTES HOMOGRAPHY MATRIX******************/
void computeHomography(vector<CornerDescriptor *> cor1, 
		       vector<CornerDescriptor *> cor2, Matrix *H){
   

    int nb_of_Tokens=cor1.size();    
    Matrix *A = new Matrix(nb_of_Tokens*2,9);

    int row=0;
    for(int i=0;i<nb_of_Tokens;i++){
	row=2*i+1;
	A->tabMat[row][1]=cor1[i]->getX();
 	A->tabMat[row][2]=cor1[i]->getY();
	A->tabMat[row][3]=1.0;
	A->tabMat[row][4]=0.0;
	A->tabMat[row][5]=0.0;
	A->tabMat[row][6]=0.0;
	A->tabMat[row][7]=-(cor1[i]->getX()*cor2[i]->getX());
	A->tabMat[row][8]=-(cor1[i]->getY()*cor2[i]->getX());
	A->tabMat[row][9]=-cor2[i]->getX();
	row++;
	A->tabMat[row][1]=0.0;
	A->tabMat[row][2]=0.0;
	A->tabMat[row][3]=0.0;
	A->tabMat[row][4]=cor1[i]->getX();
	A->tabMat[row][5]=cor1[i]->getY();
	A->tabMat[row][6]=1.0;
	A->tabMat[row][7]=-(cor1[i]->getX()*cor2[i]->getY());
	A->tabMat[row][8]=-(cor1[i]->getY()*cor2[i]->getY());
	A->tabMat[row][9]=-cor2[i]->getY();
    }
    //Eigenvalues and eigenvectors
    Matrix U,W,V;
    A->svd(U,W,V);
    
    //   cout << "OK"<< U << W << V;
    for(int j=1,k=0;j<=3;j++,k+=3)
      for(int i=1;i<=3;i++)
	H->tabMat[j][i]=V.tabMat[i+k][9]/V.tabMat[9][9]; //write the homography matrix
    
   delete A; 

}
/************COMPUTES HOMOGRAPHY MATRIX******************/
void computeAffine(vector<CornerDescriptor *> cor1, 
		   vector<CornerDescriptor *> cor2, Matrix *AFF){
   

    int nb_of_Tokens=cor1.size();    
    Matrix *A = new Matrix(nb_of_Tokens*2,7);

    int row=0;
    for(int i=0;i<nb_of_Tokens;i++){
	row=2*i+1;
	A->tabMat[row][1]=cor1[i]->getX();
 	A->tabMat[row][2]=cor1[i]->getY();
	A->tabMat[row][3]=1.0;
	A->tabMat[row][4]=0.0;
	A->tabMat[row][5]=0.0;
	A->tabMat[row][6]=0.0;
	A->tabMat[row][7]=-cor2[i]->getX();
	row++;
	A->tabMat[row][1]=0.0;
	A->tabMat[row][2]=0.0;
	A->tabMat[row][3]=0.0;
	A->tabMat[row][4]=cor1[i]->getX();
	A->tabMat[row][5]=cor1[i]->getY();
	A->tabMat[row][6]=1.0;
	A->tabMat[row][7]=-cor2[i]->getY();
    }
    //Eigenvalues and eigenvectors
    Matrix U,W,V;
    A->svd(U,W,V);
    
    for(int j=1,k=0;j<=2;j++,k+=3)
      for(int i=1;i<=2;i++)
	AFF->tabMat[j][i]=V.tabMat[i+k][7]/V.tabMat[7][7]; //write the homography matrix
    //  AFF->tabMat[3][1]=0;
    //AFF->tabMat[3][2]=0;
    // AFF->tabMat[3][3]=1;
    
   delete A; 

}

/*
 *Compute the fundamental matrix
 */
void computeEpipolarMatrix(vector<CornerDescriptor *> cor1, vector<CornerDescriptor *> cor2, Matrix *F){

      int nb_of_Tokens = cor1.size();
      Matrix *matA = new Matrix(nb_of_Tokens,9);
      Matrix *matAT = new Matrix(9,nb_of_Tokens);
      Matrix *a = new Matrix(9,9);
      
      Matrix *leftTokensXY  =new Matrix(3,nb_of_Tokens);
      Matrix *rightTokensXY =new Matrix(3,nb_of_Tokens);
      for(int nb=1; nb<=nb_of_Tokens;nb++){
	leftTokensXY->tabMat[1][nb]=cor1[nb-1]->getX();
	leftTokensXY->tabMat[2][nb]=cor1[nb-1]->getY();
	leftTokensXY->tabMat[3][nb]=1;
	rightTokensXY->tabMat[1][nb]=cor2[nb-1]->getX();
	rightTokensXY->tabMat[2][nb]=cor2[nb-1]->getY();
	rightTokensXY->tabMat[3][nb]=1;
      }
    // A = TOKENS_LEFT * TOKENS_RIGHT
    for(int nb=1; nb<=nb_of_Tokens;nb++){
      for(int j=1,k=1; j<=3; j++){
	for(int i=1; i<=3; i++,k++){
	  matA->tabMat[nb][k] = leftTokensXY->tabMat[j][nb] * rightTokensXY->tabMat[i][nb];
	}
      }
    }

    //  A_TRANSPOSE
    for(int nb=1; nb<= nb_of_Tokens; nb++){
      for(int i=1; i<=9; i++){
	matAT->tabMat[i][nb] = matA->tabMat[nb][i];
      }
    }
    //a = A_TRANSPOSE * A
    for(int j=1; j<=9; j++){
      for(int i=1; i<=9; i++){
	for(int nb=1; nb<= nb_of_Tokens; nb++){
	  a->tabMat[j][i]+=matAT->tabMat[j][nb]*matA->tabMat[nb][i];
	}
      }
    }

    //Eigenvalues and eigenvectors

    Matrix U,W,V;
    a->svd(U,W,V);

   for(int j=1,k=0;j<=3;j++,k+=3)
      for(int i=1;i<=3;i++)
	F->tabMat[i][j]=V.tabMat[i+k][9]/V.tabMat[9][9]; //write the fundamental matrix
}



/********HOMOGRAPHY FROM IMAGES AND COARSE HOMOGRAPHY**************/
void computeHomographyPrec(vector<CornerDescriptor *> &cor1,vector<CornerDescriptor *> &cor2, Matrix *H,float max_dist){
  
  vector<CornerDescriptor *> cor1_tmp;
  vector<CornerDescriptor *> cor2_tmp;
  vector<CornerDescriptor *> cor1_tmp_;
  vector<CornerDescriptor *> cor2_tmp_;
  
  //  float max_scale_error=0.2;
  for(int i=0;i<15;i++){
    matchPointsByHomography(cor1, cor2,cor1_tmp,cor2_tmp, H, max_dist);
    //matchScaleByHomography(cor1_tmp_, cor2_tmp_,cor1_tmp,cor2_tmp, H, max_scale_error);
    cout << "max dist :"<< max_dist<< endl;
    cout << "points1 :" << cor1.size()<< ",  matched point: "<< cor1_tmp.size()<< endl; 
    cout << "points2 :" << cor2.size()<< ",  matched point: "<< cor2_tmp.size()<< endl<< endl;
    //writeCorners(cor1_tmp,"cor0.cor");
    //writeCorners(cor2_tmp,"cor1.cor");
    if(cor1_tmp.size()>10){
      computeHomography(cor1_tmp,cor2_tmp, H);
      if(max_dist>1)max_dist--;
    }
    else {
      max_dist+=2;
    }
    if(getchar()=='y')break;
    cor1_tmp.clear();
    cor2_tmp.clear();    
    cor1_tmp_.clear();
    cor2_tmp_.clear();    
  }
}


void getAffMat(CornerDescriptor * cor, Matrix *H,  Matrix &AFF){
  
  Matrix Mcor(2,2),Mcor2(2,2),U,D,V;
    Mcor(1,1)=cor->getMi11();
    Mcor(1,2)=cor->getMi12();
    Mcor(2,1)=cor->getMi21();
    Mcor(2,2)=cor->getMi22();

    Mcor.svd(U,D,V);
    float scale1=3*cor->getCornerScale();
    float a=D(1,1)*scale1;
    float b=D(2,2)*scale1;
    float xe=cor->getX();
    float ye=cor->getY();
    vector<CornerDescriptor *> cor1;
    vector<CornerDescriptor *> cor2;
    float px,py,ax,ay;
    float da=a/5.0;
    float db=b/5.0;
    for(float j=-b;j<=b;j+=db){
      for(float i=-a;i<=a;i+=da){	
	px=xe+V(1,1)*i+V(1,2)*j;
	py=ye+V(2,1)*i+V(2,2)*j;	
	getHPoint(px,py,H,ax,ay);	
	cor1.push_back(new CornerDescriptor(px,py));
	cor2.push_back(new CornerDescriptor(ax,ay));
	//    writeCorners(cor1,"aff1.cor");
	//    writeCorners(cor2,"aff2.cor");
	//    cout << "OK"<< endl;
	//   getchar();
      }
    } 
    computeAffine(cor1,cor2, &AFF);
    cor1.clear();
    cor2.clear();
}


void displayEllipse(DARY *im, Matrix *M, int x, int y, int xc, int yc, float color){
  Matrix Mi,U,V,D;
  Mi =(*M);// M->inverse(); 
  Mi.svd(U,D,V);
  float sc=sqrt(D(1,1)*D(2,2));
  D(1,1)/=sc;
  D(2,2)/=sc;
  //cout << "DE " << sc << endl << D;
  Mi=U*D*V.transpose();
  int  size=(int)(rint(sc)),xi,yi;
  //  float color=255;
  Matrix X(2,1),Xe;
  for(int i=-2;i<=2;i++){
      im->fel[y+i][x]=im->fel[y][x+i]=color;
  } 
  for(int i=-4;i<=4;i++){
      im->fel[y+yc+i][x+xc]=im->fel[y+yc][x+xc+i]=color;
      im->fel[y+yc+i][x-xc]=im->fel[y+yc][x-xc+i]=color;
      im->fel[y-yc+i][x+xc]=im->fel[y-yc][x+xc+i]=color;
      im->fel[y-yc+i][x-xc]=im->fel[y-yc][x-xc+i]=color;
  }
  for(int i=-size;i<=size;i++){
      X(1,1)=i;
      X(2,1)=(sqrt((float)(size*size-i*i)));
      Xe=Mi*X;
      xi=(int)rint(Xe(1,1));
      yi=(int)rint(Xe(2,1));
      if(y+yi>0 && y+yi<im->y() && x+xi>0 && x+xi<im->x())
	im->fel[y+yi][x+xi]=color;
      if(y-yi>0 && y-yi<im->y()&& x-xi>0 && x-xi<im->x())
	im->fel[y-yi][x-xi]=color;

      X(1,1)=X(2,1);
      X(2,1)=i;
      Xe=Mi*X;
      xi=(int)rint(Xe(1,1));
      yi=(int)rint(Xe(2,1));
      if(y+yi>0 && y+yi<im->y() && x+xi>0 && x+xi<im->x())
	im->fel[y+yi][x+xi]=color;
      if( y-yi>0 && y-yi<im->y()&& x-xi>0 && x-xi<im->x())
	im->fel[y-yi][x-xi]=color;

      X(1,1)=i;
      X(2,1)=0;
      Xe=V*X;
      xi=(int)rint(Xe(1,1));
      yi=(int)rint(Xe(2,1));
      if(y+yi>0 && y+yi<im->y() && x+xi>0 && x+xi<im->x())
	im->fel[y+yi][x+xi]=color;      
  }
   
}

float ellipsArea(Matrix D,Matrix I, int &xc, int &yc){
    float a2=(D(1,1)*D(1,1));
    float b2=(D(2,2)*D(2,2));
    float r2=I(1,1)*I(1,1);
    float y2=(r2-a2)/(1-a2/b2);
    if(y2>0){
	yc= (int)rint(sqrt((float)y2));
    }else yc=0;  
 	
    float x2=(a2-r2*(a2/b2))/(1-a2/b2);
    if(x2>0 && yc>0){
	xc= (int)rint(sqrt((float)x2));
    }else {
	yc=0;   	
	xc=0;
    }       

    float CA,EA,diff,angle,relativ;
    CA=M_PI*r2;
    EA=M_PI*D(1,1)*D(2,2);  
    if(yc==0 && xc==0){/****no intersection points******/
	diff=fabs(EA-CA);
	relativ=100.0*diff/((EA<CA)?CA:EA);       
       	cout << "CA "<<CA << " EA " << EA << "diff " <<diff<< endl;
	return relativ;
    }
    else{
	float EA_aplha=asin(xc/D(1,1));
	float CA_aplha=asin(xc/I(1,1));
	angle=asin(xc/sqrt((float)(xc*xc+yc*yc)));
	//cout << "angle " << 180*angle/M_PI<< endl;
	float EA_integ=0.5*D(1,1)*D(2,2)*(EA_aplha+0.5*sin(2*EA_aplha));
	float CA_integ=           0.5*r2*(CA_aplha+0.5*sin(2*CA_aplha));
	float CA_out=4*(CA_integ-EA_integ);
	float CA_in=CA-CA_out;
	float EA_out=EA-CA_in;
	diff=(EA_out+CA_out)/(CA_in+EA_out+CA_out);
	relativ=100.0*diff;       
	//  	cout << "CA "<<CA << " EA " << EA << " CA_integ " <<CA_integ << " EA_integ " <<EA_integ<< " CA_in "<<CA_in<<" CA_out "<< CA_out<<endl;
	return relativ;		
    
    }
    return 0;
}

/********COMPARE AFFINE REGIONS**************/
float matchAffinePoints(CornerDescriptor * cor1,CornerDescriptor *cor2, Matrix AFF){
    
    Matrix Mcor1(2,2),Mcor1q(2,2),Mcor2(2,2),U1,D1,D1q(2,2),V1,U2,D2,V2,U3,D3,V3,MAFF(2,2),U4,D4,V4,DIFF,BtMB,rot,I(2,2,0.0);
    I(1,1)=I(2,2)=1;
    
    float angle;
    MAFF(1,1)=AFF(1,1);
    MAFF(1,2)=AFF(1,2);
    MAFF(2,1)=AFF(2,1);
    MAFF(2,2)=AFF(2,2);
    Mcor1(1,1)=cor1->getMi11();
    Mcor1(1,2)=cor1->getMi12();
    Mcor1(2,1)=cor1->getMi21();
    Mcor1(2,2)=cor1->getMi22();
    Mcor2(1,1)=cor2->getMi11();
    Mcor2(1,2)=cor2->getMi12();
    Mcor2(2,1)=cor2->getMi21();
    Mcor2(2,2)=cor2->getMi22();  

    /*computing M pow(-1)*/
    Mcor1.svd(U1,D1,V1);
    D1=D1*cor1->getCornerScale();
    Mcor1=U1*D1*V1.transpose();
    D1(1,1)=(D1(1,1)*D1(1,1));
    D1(2,2)=(D1(2,2)*D1(2,2));
    Mcor1q=U1*D1*V1.transpose();
    
    /*computing M pow(-1/2)*/
    Mcor2.svd(U2,D2,V2);
    D2=D2*cor2->getCornerScale();
    Mcor2=U2*D2*V2.transpose();
    float angle2=acos(V2(1,1));
    /*       
    cout << "X " << cor1->getX() << " Y " <<  cor2->getY() << endl;
    cout <<endl<<  "Mcor2\n" << Mcor2 << endl; 
    cout << "U2\n" << U2; 
    cout << "D2\n" << D2;
    cout << "V2\n" << V2;
    cout << "angle " << 180*acos(rot(1,1))<< endl;
    */
    /*computing sqrt(AFF*Mcorq1*AFF.transpose)   
      sqrt(M pow-1)=sqrt(AFF*Mcorq1*AFF.transpose)*/
    BtMB=(MAFF*Mcor1q*MAFF.transpose());
    BtMB.svd(U1,D1,V1);
    D1(1,1)= sqrt(D1(1,1));
    D1(2,2)= sqrt(D1(2,2));
    BtMB=U1*D1*V1.transpose();
    float angle1=acos(V1(1,1));
    /*    cout << "sqrt(BtMB)\n " << BtMB << endl;
    cout << "U1a2\n" << U1;
    cout << "D1a2\n" << D1;
    cout << "V1a2\n" << V1;
    cout << "angle " << 180*acos(rot(1,1))<< endl;
    */
    /*computing difference */
    DIFF=Mcor2.inverse()*BtMB;
    DIFF.svd(U1,D1,V1);
    angle=180*fabs(angle1-angle2)/M_PI;
    float diff1 = fabs(D1(1,1)-1);
    float diff2 = fabs(D1(2,2)-1);
    float diff = ((diff1<diff2)?diff2:diff1);

    D1*=80;
    I*=80;
    int xc,yc;
    float da=ellipsArea( D1,I, xc, yc);

    if(da<1000){//angle <2 && angle && diff<0.1){

	/*	
    DARY *im = new DARY(300,300,0.0);
    DARY *im2 = new DARY(300,300,0.0);
    displayEllipse(im, &(10*Mcor2),150, 150,0,0,255);
    displayEllipse(im, &(10*BtMB),150, 150,0,0,160);
    cout <<endl <<  xc << " " << yc<< endl;
    displayEllipse(im2, &D1,150, 150,0,0,255);
    displayEllipse(im2, &I,150, 150,xc,yc,160);
    im->write("comp.pgm");
    im2->write("comp2.pgm");
    delete im;delete im2;
    
    cout << "relativ " << da<< endl;
    cout << "DIFF\n" << DIFF << endl;
    cout << "U1\n" << U1;
    cout << "D1\n" << D1;
    cout << "V1\n" << V1;
    cout << "U1*V1.transpose\n" << rot;
    cout << "angle " << angle<< endl;              
    cout << "diff1 " << diff1<< endl;
    cout << "diff2 " << diff2<< endl;
    cout << "diff " << diff<< endl;
    
    getchar();
	*/
    return da;
   } else
    return (100);
}
float square(float a){
    return a*a;
}

void getMeanAffine(vector<CornerDescriptor *> &cor1){
  Matrix U(2,2),D(2,2),V(2,2),M(2,2),Dm(2,2,0.0);
  //cout << "nb points "<<cor1.size()<<endl;
  float nb=0,angle;
  float  scales=0;
  float  anV=0;
  float  anU=0;
  float  escales=0;
  float x=0,y=0;
  for(uint j=0;j<cor1.size();j++){
    M(1,1)=cor1[j]->getMi11();
    M(1,2)=cor1[j]->getMi12();
    M(2,1)=cor1[j]->getMi21();
    M(2,2)=cor1[j]->getMi22(); 
    M.svd(U,D,V); 
    escales+=(D(1,1)/D(2,2));
    scales+=(cor1[j]->getCornerScale());
    angle=acos(V(1,1));
    anV+=(angle);
    /*
    cout << U << endl;
    cout << D << endl;
    cout << V << endl;
    cout <<cor1[j]->getX()<< " "<< cor1[j]->getY()<<  " angle cos "<< (180*angle/M_PI) << endl;
    */
    angle=acos(U(1,1));
    anU+=(angle);  
    x+=cor1[j]->getX();
    y+=cor1[j]->getY();
    nb++;
  }  
  scales/=nb;
  escales/=nb;
  anV/=nb;
  anU/=nb;
  x/=nb;
  y/=nb;
  D(1,1)=sqrt(escales);
  D(2,2)=1.0/D(1,1);
  V(1,1)=(V(1,1)>0)?fabs(cos(anV)):(-fabs(cos(anV)));
  V(1,2)=(V(1,2)>0)?fabs(sin(anV)):(-fabs(sin(anV)));
  V(2,1)=(V(2,1)>0)?fabs(sin(anV)):(-fabs(sin(anV)));
  V(2,2)=(V(2,2)>0)?fabs(cos(anV)):(-fabs(cos(anV)));
  U(1,1)=(U(1,1)>0)?fabs(cos(anU)):(-fabs(cos(anU)));
  U(1,2)=(U(1,2)>0)?fabs(sin(anU)):(-fabs(sin(anU)));
  U(2,1)=(U(2,1)>0)?fabs(sin(anU)):(-fabs(sin(anU)));
  U(2,2)=(U(2,2)>0)?fabs(cos(anU)):(-fabs(cos(anU)));
  /*
  cout << endl<< U << endl;
  cout << D << endl;
  cout << V << endl;  
  cout <<endl << x << "  "<<y<< "angle "<< (180*anV/M_PI)<<  endl;
  */
  M=U*D*V.transpose();
  cor1[0]->setCornerScale(scales);
  cor1[0]->setX_Y(x,y);
  cor1[0]->setMi(M(1,1),M(1,2),M(2,1),M(2,2));
}

void removeSimilarPoints(vector<CornerDescriptor *> &cor1){
  int csize=cor1.size();
  CornerDescriptor *c1;
  vector<CornerDescriptor *> csim;
  vector<CornerDescriptor *> cmean;
  float dist,sc_dif;
  Matrix M(2,2),U,V,D;
  float angle1,sc1,angle,sc,dist_max;
  while(cor1.size()>0){
    cout << "\rRemoving similar points " << cor1.size() << "   "<<flush;
    c1=cor1[0];
    csim.push_back(cor1[0]);
    cor1.erase((std::vector<CornerDescriptor*>::iterator)&cor1[0]);
    M(1,1)=c1->getMi11();
    M(1,2)=c1->getMi12();
    M(2,1)=c1->getMi21();
    M(2,2)=c1->getMi22(); 
    M.svd(U,D,V);
    angle=acos(V(1,1));
    sc=D(1,1)/D(2,2);
    dist_max=0.6*c1->getCornerScale();
    for(uint j=0;j<cor1.size();j++){
      dist=sqrt(square(cor1[j]->getX()-c1->getX())+square(cor1[j]->getY()-c1->getY()));
      sc_dif=fabs(1-(c1->getCornerScale()/c1->getCornerScale()));  
      M(1,1)=cor1[j]->getMi11();
      M(1,2)=cor1[j]->getMi12();
      M(2,1)=cor1[j]->getMi21();
      M(2,2)=cor1[j]->getMi22(); 
      M.svd(U,D,V);
      angle1=acos(V(1,1));
      sc1=D(1,1)/D(2,2);
      if(dist<dist_max && sc_dif<0.3 && fabs(angle1-angle)<0.2 && fabs(1-sc/sc1)<0.2){
	csim.push_back(cor1[j]);
	cor1.erase((std::vector<CornerDescriptor*>::iterator)&cor1[j]);j--;
      }	
    }
    if(csim.size()>1){
      getMeanAffine(csim);
    }
    cmean.push_back(csim[0]);
    for (size_t i= 1; i<csim.size(); ++i) delete csim[i];
    csim.clear();
  }
  cout <<endl<< "size bef: " <<csize<<" aft: "<< cmean.size()<< endl;
  cor1=cmean;
}

/********COMPARE AFFINE REGIONS of corresponding points**************/
void matchAffinePoints(vector<CornerDescriptor *> &cor1,vector<CornerDescriptor *> &cor2,
		       vector<CornerDescriptor *> &cor1_match,vector<CornerDescriptor *> &cor2_match, 
		       Matrix *H, float max_error){

  Matrix H1,U,V,D,AFF(2,2,0.0),N; 
  float error=1000,error2;
  float dist,sc_dif;
  float min_sc=100,max_sc=0;
  CornerDescriptor *c1,*c2;
  //  for(uint j=0;j<cor1.size();j++){
  while(cor1.size()>0){
      cout << "\r" << " of " << cor1.size()<< flush;
      getAffMat(cor1[0],H,AFF);
      error=matchAffinePoints(cor1[0], cor2[0], AFF);
      c1=cor1[0];
      c2=cor2[0];
      cor1.erase((std::vector<CornerDescriptor*>::iterator)&cor1[0]);
      cor2.erase((std::vector<CornerDescriptor*>::iterator)&cor2[0]);
      if(error<max_error){
	  for(uint i=0;i<cor1.size();i++){
	      dist=sqrt(square(cor1[i]->getX()-c1->getX())+square(cor1[i]->getY()-c1->getY()));
	      sc_dif=fabs(cor1[i]->getCornerScale()-c1->getCornerScale())/(c1->getCornerScale());	      
	      if(dist<4 && sc_dif<0.3){
		  //getAffMat(cor1[i],H,AFF2);
		  error2=matchAffinePoints(cor1[i], cor2[i], AFF);
		  if(error2 < error){
		      c1=cor1[i];
		      c2=cor2[i];
		      cor1.erase((std::vector<CornerDescriptor*>::iterator)&cor1[i]);
		      cor2.erase((std::vector<CornerDescriptor*>::iterator)&cor2[i]);
		      error=error2;
		      i--;
		  }else {
		      cor1.erase((std::vector<CornerDescriptor*>::iterator)&cor1[i]);
		      cor2.erase((std::vector<CornerDescriptor*>::iterator)&cor2[i]);
		      i--;
		  }
	      }
	  }
	  if(min_sc>c1->getCornerScale())min_sc=c1->getCornerScale();
	  else if(max_sc<c1->getCornerScale())max_sc=c1->getCornerScale();
	  cor1_match.push_back(c1);	
	  cor2_match.push_back(c2);	
      }
  }  
  //cout <<endl<< "max sc " << max_sc <<  "min sc " << min_sc << endl;
}

void projectAffineCorners(vector<CornerDescriptor *> &cor, vector<CornerDescriptor *> &cor2, Matrix *H){  
    CornerDescriptor *c2;
    float x,y,c;
    Matrix H1,U,V,D,AFF(2,2,0.0),N; 
    H1=(*H);
    H1=H1/H1(3,3);
    AFF(1,1)=H1(1,1);
    AFF(1,2)=H1(1,2);
    AFF(2,1)=H1(2,1);
    AFF(2,2)=H1(2,2);
    Matrix Mcor1(2,2,0.0), Mcor2,U1,V1,D1;
    H1=(*H);
    for(uint i=0;i<cor.size();i++){
      getHPoint(cor[i]->getX(),cor[i]->getY() , H, x, y);
      getAffMat(cor[i],H,AFF);    
      c2=new CornerDescriptor(x,y);
      Mcor1(1,1)=cor[i]->getMi11();
      Mcor1(1,2)=cor[i]->getMi12();
      Mcor1(2,1)=cor[i]->getMi21();
      Mcor1(2,2)=cor[i]->getMi22();
      Mcor1.svd(U1,D1,V1);
      D1=D1*cor[i]->getCornerScale();
      D1(1,1)=(D1(1,1)*D1(1,1));
      D1(2,2)=(D1(2,2)*D1(2,2));      
      Mcor1=U1*D1*V1.transpose();
      Mcor2=AFF*Mcor1*AFF.transpose();      

      Mcor2.svd(U1,D1,V1);
      D1(1,1)=sqrt(D1(1,1));
      D1(2,2)=sqrt(D1(2,2));	      
      c=sqrt(D1(1,1)*D1(2,2));
      D1(1,1)=D1(1,1)/c;
      D1(2,2)=D1(2,2)/c;	
      Mcor2=U1*D1*V1.transpose();
      c2->setMi(Mcor2(1,1),Mcor2(1,2),Mcor2(2,1),Mcor2(2,2));
      c2->setCornerScale(c);
      cor2.push_back(c2);
    }
    //getchar();
}






/***************MATCHING DESCRIPTORS***************************/
/***************MATCHING DESCRIPTORS***************************/
/***************MATCHING DESCRIPTORS***************************/






void getNormPoint(DARY *img_in, CornerDescriptor *cor,DARY *imgn, float scmean){

    float angle = cor->getAngle();
    float lecos=cos(-angle);
    float lesin=sin(-angle);	
    
    float scal=cor->getCornerScale()/scmean;
    float m11=scal*(cor->getMi11()*lecos-cor->getMi12()*lesin);
    float m12=scal*(cor->getMi11()*lesin+cor->getMi12()*lecos);
    float m21=scal*(cor->getMi21()*lecos-cor->getMi22()*lesin);
    float m22=scal*(cor->getMi21()*lesin+cor->getMi22()*lecos);
    
    float x = (cor->getX());
    float y = (cor->getY());

    imgn->interpolate(img_in,x,y,m11,m12,m21,m22);
    x=imgn->x()>>1;y=imgn->y()>>1;
    normalize(imgn,(int)x,(int)y,x);

}

float SSD(DARY *img_in1, CornerDescriptor *cor1, DARY *img_in2, CornerDescriptor *cor2){

    float scmean=10.0;
    int im_size=2*(int)rint(GAUSS_CUTOFF*scmean)+3;
    DARY * imgn1 = new DARY(im_size,im_size);      
    getNormPoint(img_in1, cor1, imgn1,scmean);    
    DARY * imgn2 = new DARY(im_size,im_size);      
    getNormPoint(img_in2, cor2, imgn2,scmean);
    float sum=0;//,P=0,X=0;
    for(int i=0;i<im_size;i++){
	for(int j=0;j<im_size;j++){ 
	    //P+=square(imgn1->fel[i][j]);
	    //X+=square(imgn2->fel[i][j]);
	  //sum+=(imgn1->fel[i][j]*imgn2->fel[i][j]);
	    sum+=square(imgn1->fel[i][j]-imgn2->fel[i][j]);
	}
    }
    /*
    float NCC = sum/(P*X);
    imgn1->write("cor1.pgm");
    imgn2->write("cor2.pgm");
    cout << "norm "<< sqrt(sum)<< endl;getchar();
    
    */
    delete imgn1;delete imgn2;
    return sqrt(sum);

}



void SSD(DARY *img_in1, vector<CornerDescriptor *> &cor1, 
	 DARY *img_in2, vector<CornerDescriptor *> &cor2, float dist_cc){
  float ssd;
    for(uint i=0;i<cor1.size();i++){
      ssd=SSD(img_in1,cor1[i],img_in2,cor2[i]);
      cout << "\rSSD " << i << " of "<< cor1.size()<< " ssd  "<<ssd <<"   "<< flush;//getchar();
	//cout << cor1[i]->getX()<< " " << cor1[i]->getY()<< " to ";
	//cout << cor2[i]->getX()<< " " << cor2[i]->getY()<<endl; 
      if(ssd > dist_cc){
	    cor1.erase((std::vector<CornerDescriptor*>::iterator)&cor1[i]);
	    cor2.erase((std::vector<CornerDescriptor*>::iterator)&cor2[i]);i--;
	}
    }


}

void checkMatches( vector<CornerDescriptor*> &cor1, vector<CornerDescriptor*> &cor2, Matrix *H, 
		     float dist_max,int model){
    
    vector<CornerDescriptor*> match1;
    vector<CornerDescriptor*> match2;
    if(cor1.size()!=cor2.size()){cout << "size not equal "<< endl;exit(0);}
    float matched=0;
    float dist;
    for(unsigned int c1=0;c1<cor1.size();c1++){
      if(model){
	if(checkHomog(cor1[c1],cor2[c1],H,dist_max,dist)){
	  match1.push_back(cor1[c1]);
	  match2.push_back(cor2[c1]);
	  matched++;	    
	}
      }else{
	if(checkEpip(cor1[c1],cor2[c1],H,dist_max,dist)){
	  match1.push_back(cor1[c1]);
	  match2.push_back(cor2[c1]);
	  matched++;	    
	}
      }
    }   
    cor1.clear();
    cor2.clear();
    cor1=match1;
    cor2=match2;
}

void matchDescriptors( vector<CornerDescriptor*> &cor1, 
		       vector<CornerDescriptor*> &cor2, Matrix * invCovMat, Matrix * geom,
		       vector<CornerDescriptor*> &match1_tmp, 
		       vector<CornerDescriptor*> &match2_tmp, 
		       vector<CornerDescriptor*> &match3_tmp, 
		       const float d_dist_max){
  
  float d_dist,d_dist_min; 
  unsigned int size1=cor1.size();
  unsigned int size2=cor2.size();
  int c2_min,c2_min2;
  float dist_max=5;
  float dist;
  for(unsigned int c1=0;c1<size1;c1++){
    cout << "\rdescriptor " << c1 << flush;	
    d_dist_min=d_dist_max;
    c2_min2=c2_min=-1;
    for(unsigned int c2=0;c2<size2;c2++){	
      d_dist = distEuc(cor1[c1]->getVec(), cor2[c2]->getVec(),cor2[c2]->getSize());
      //d_dist = distCSift(cor1[c1]->getVec(), cor2[c2]->getVec(),cor2[c2]->getSize());
      //d_dist = distMahalanobis(cor1[c1]->getVec(), cor2[c2]->getVec(), invCovMat);
	//cout << "d_dist " << d_dist_max<< endl;
      if(d_dist<d_dist_min && checkEpip(cor1[c1],cor2[c2],geom,dist_max,dist)){	    
	d_dist_min=d_dist;
	c2_min2=c2_min;
	c2_min=c2;
      }	
    }
    if(c2_min>=0){
      match1_tmp.push_back(cor1[c1]);
      match2_tmp.push_back(cor2[c2_min]);
      if(c2_min2>=0)match3_tmp.push_back(cor2[c2_min2]);
      else match3_tmp.push_back(cor2[c2_min]);
    }
  }
}

void checkMatchDescriptors( vector<CornerDescriptor*> &cor1, 
		       vector<CornerDescriptor*> &cor2, Matrix * invCovMat, Matrix * geom,
		       const float d_dist_max){
  vector<CornerDescriptor*> match1_tmp;
  vector<CornerDescriptor*> match2_tmp;  
  float d_dist,d_dist_min; 
  unsigned int size1=cor1.size();
  unsigned int size2=cor2.size();
  int c2_min,c2_min2;
  float dist_max=5;
  float dist;
  for(unsigned int c1=0;c1<size1;c1++){
    cout << "\rdescriptor " << c1 << flush;	
    d_dist_min=d_dist_max;
    c2_min=-1;
    for(unsigned int c2=0;c2<size2;c2++){	
      d_dist = distEuc(cor1[c1]->getVec(), cor2[c2]->getVec(),cor2[c2]->getSize());
      //d_dist = distCSift(cor1[c1]->getVec(), cor2[c2]->getVec(),cor2[c2]->getSize());
      //d_dist = distMahalanobis(cor1[c1]->getVec(), cor2[c2]->getVec(), invCovMat);
	//cout << "d_dist " << d_dist_max<< endl;
      if(d_dist<d_dist_min && checkHomog(cor1[c1],cor2[c2],geom,dist_max,dist)){	    
	d_dist_min=d_dist;
	c2_min=c2;
      }	
    }
    if(c2_min>=0){
      match1_tmp.push_back(cor1[c1]);
      match2_tmp.push_back(cor2[c2_min]);
      //cor1.erase((std::vector<CornerDescriptor*>::iterator)&cor1[c1]);
      //cor2.erase((std::vector<CornerDescriptor*>::iterator)&cor2[c2_min]);
    }
  }

  cor2.clear();
  cor1.clear();
  cor1=match1_tmp;
  cor2=match2_tmp;
}



void matchDescriptorsSimilar( vector<CornerDescriptor*> &cor1, 
			      vector<CornerDescriptor*> &cor2,  
			      Matrix * invCovMat, Matrix * geom,
			      const float d_dist_max){
  
  vector<CornerDescriptor*> match1_tmp;
  vector<CornerDescriptor*> match2_tmp;
  float d_dist,dist_max=3; 
  unsigned int size1=cor1.size(); 
  unsigned int size2=cor2.size();
  for(unsigned int c1=0;c1<size1;c1++){
    cout << "\rdescriptor " << c1 << flush;	
    for(unsigned int c2=0;c2<size2;c2++){	
      	d_dist = distEuc(cor1[c1]->getVec(), cor2[c2]->getVec(), cor2[c2]->getSize());
      //d_dist = distMahalanobis(cor1[c1]->getVec(), cor2[c2]->getVec(), invCovMat);
	if(d_dist<d_dist_max /*&& checkEpip(cor1[c1],cor2[c2],geom,dist_max)*/){	    
	    match1_tmp.push_back(cor1[c1]);
	    match2_tmp.push_back(cor2[c2]);
      }	
    }
  }
  removeFloatMatches(match1_tmp,match2_tmp);
  removeFloatMatches(match2_tmp,match1_tmp);
  cor1=match1_tmp;
  cor2=match2_tmp;
}

float matchSimilarDescriptors( vector<CornerDescriptor*> &cor1, 
			      vector<CornerDescriptor*> &cor2,  
			      Matrix * geom,
			      const float d_dist_max, ofstream &out){
        
  vector<CornerDescriptor*> match1_tmp;
  vector<CornerDescriptor*> match2_tmp;
  vector<CornerDescriptor*> cor1_tmp;
  vector<CornerDescriptor*> cor2_tmp;
  float d_dist;
  unsigned int size1=cor1.size(); 
  unsigned int size2=cor2.size();
  out << "Similarity threshold: "<<d_dist_max<< endl;
  matchPointsByHomography(cor1, cor2,cor1_tmp,cor2_tmp,geom, 10);
  out << "corners size 1: "<<size1<< " corners size 2: "<<size2<< endl;
  out << "matched by loc.: "<<cor1_tmp.size()<< " of " << size1 << " " << 100.0*cor1_tmp.size()/size1 << "\%"<< endl;
  checkScaleByHomography(cor1_tmp,cor2_tmp,geom,0.3);
  out << "matched by loc., scale: "<< cor1_tmp.size() << " of " << size1 << " "<<100.0*cor1_tmp.size()/size1<< "\%"<< endl;
  checkAngleByHomography(cor1_tmp,cor2_tmp,geom,0.15);
  out << "matched by loc., scale, angle: "<< cor1_tmp.size() << " of " << size1 << " "<<100.0*cor1_tmp.size()/size1<< "\%"<< endl;
  Matrix Hi;
  Hi=geom->inverse();
  vector<CornerDescriptor*> cor1_aff;
  vector<CornerDescriptor*> cor2_aff;
  // matchAffinePoints(cor1_tmp, cor2_tmp,cor1_aff,cor2_aff, geom, 60);
  out <<endl<< "matched l-r "<< cor1_aff.size()<< endl;
  //matchAffinePoints(cor2_aff, cor1_aff,cor2_tmp,cor1_tmp, &Hi, 60);
  out <<endl<< "matched r-l "<< cor2_tmp.size()<< endl;
  out << "matched / detected "<< ((100.0*cor1_tmp.size())/((size1<size2)?size1:size2))<< "\%"<<endl;

  cor1_aff.clear();
  cor2_aff.clear();


  float correct=(float)cor1_tmp.size(); 

  for(unsigned int c1=0;c1<size1;c1++){
    cout << "\rdescriptor " << c1 << flush;	
    for(unsigned int c2=0;c2<size2;c2++){   	 
      d_dist = distEuc(cor1[c1]->getVec(), cor2[c2]->getVec(),cor2[c2]->getSize());
      //d_dist = distMahalanobis(cor1[c1]->getVec(), cor2[c2]->getVec(), invCovMat); 
      if(d_dist<d_dist_max /*&& checkEpip(cor1[c1],cor2[c2],geom,dist_max)*/){	    
	match1_tmp.push_back(cor1[c1]);
	match2_tmp.push_back(cor2[c2]);
      }	 
    }
  }
  
  cor1_tmp.clear();
  cor2_tmp.clear();
  out << "matched by descriptor: "<<match1_tmp.size()<< "  of all " << size1 << " "<<100.0*match1_tmp.size()/size1<< "\%"<< endl;
  cout <<endl<< "matched by descriptor: "<<match1_tmp.size()<< "  of all " << size1 << " "<<100.0*match1_tmp.size()/size1<< "\%"<< endl;
  if(match1_tmp.size()!=0){
    checkMatches(match1_tmp, match2_tmp,geom, 3,1);
    checkScaleByHomography(match1_tmp,match2_tmp,geom,0.3);
    checkAngleByHomography(match1_tmp,match2_tmp,geom,0.15);
    //removeFloatMatches(match1_tmp,match2_tmp);
    //removeFloatMatches(match2_tmp,match1_tmp);
    //matchAffinePoints(match1_tmp, match2_tmp,cor1_aff,cor2_aff, geom, 60);
    //matchAffinePoints(cor2_aff, cor1_aff,match2_tmp,match1_tmp, &Hi, 60);
  }
  
  out << "matched by descriptor: "<<match1_tmp.size()<< " correct of all " << size1 << " "<<100.0*match1_tmp.size()/size1<< "\%"<< endl;
  out << "matched by descriptor: "<<match1_tmp.size()<< " correct of possible " << correct << " "<<100.0*match1_tmp.size()/correct<< "\%"<< endl;
  cout << "matched by descriptor: "<<match1_tmp.size()<< " correct of possible " << correct << " "<<100.0*match1_tmp.size()/correct<< "\%"<< endl;
  return (match1_tmp.size()/correct);
}



void matchDescriptorsNearest(vector<CornerDescriptor*> &cor1, 
		      vector<CornerDescriptor*> &cor2, 	 
		      Matrix *covMat, Matrix *geom,
		      const float d_dist_max){
  
    vector<CornerDescriptor*> match3_tmp;
    vector<CornerDescriptor*> match2_tmp;
    vector<CornerDescriptor*> match1_tmp;
    
    vector<CornerDescriptor*> matchi3_tmp;
    vector<CornerDescriptor*> matchi2_tmp;
    vector<CornerDescriptor*> matchi1_tmp;

    Matrix invC;
    invC=covMat->inverse();
    matchDescriptors( cor1, cor2,&invC, geom, match1_tmp, match2_tmp, match3_tmp, d_dist_max);
    Matrix geom_i=geom->transpose();
    matchDescriptors( cor2, cor1,&invC, &geom_i, matchi1_tmp, matchi2_tmp, matchi3_tmp, d_dist_max);
    
    vector<CornerDescriptor*> matched1;
    vector<CornerDescriptor*> matched2;    
    
    for(unsigned int c1=0;c1<match1_tmp.size();c1++){
      for(unsigned int c2=0;c2<matchi2_tmp.size();c2++){
	if(match1_tmp[c1]==matchi2_tmp[c2]){
	  if(match2_tmp[c1]==matchi1_tmp[c2]){
	    matched1.push_back(match1_tmp[c1]);
	    matched2.push_back(match2_tmp[c1]);	    
	  }else if(match3_tmp[c1]==matchi1_tmp[c2]){
	    matched1.push_back(match1_tmp[c1]);
	    matched2.push_back(match2_tmp[c1]);		    
	  }else if(match1_tmp[c1]==matchi3_tmp[c2]){
	    matched1.push_back(match1_tmp[c1]);
	    matched2.push_back(match2_tmp[c1]);		    
	  }
	}
      }
    } 
    removeFloatMatches(matched1,matched2);
    removeFloatMatches(matched2,matched1);
    cor1=matched1;
    cor2=matched2;
    /*  for(unsigned int c1=0;c1<cor1.size();c1++){
      float d_dist = distMahalanobis(cor1[c1]->getVec(), cor2[c1]->getVec(), &invC);
      cout << cor1[c1]->getX()<< " " << cor1[c1]->getY()<<"   " << cor2[c1]->getX();
      cout << " " << cor2[c1]->getY()<< " "<<d_dist<< endl;
      }*/    
}

void angleOrder(vector<CornerDescriptor*> &cor1,int c1, vector<int> neigh1){

  
}

int computeVotes(vector<CornerDescriptor*> cor1, 
		 vector<CornerDescriptor*> &cor2, int cr1, int cr2,
		 vector<int> neigh1,vector<int> neigh2, 
		 DARY *rank){
  
  // DARY *im1 = new DARY(768,700,0.0);
  //DARY *im2 = new DARY(786,700,0.0);
 
  
  DARY *nrank= new DARY(neigh1.size(),neigh2.size(),0.0);
  CornerDescriptor *cd1, *cd2;
  float angle1=0;
  vector<int> n1;vector<int> n2;
  for(int c1=0;c1<neigh1.size();c1++){
    for(int c2=0;c2<neigh2.size();c2++){
      if(rank->fel[neigh1[c1]][neigh2[c2]]<50){
	n1.push_back(neigh1[c1]);n2.push_back(neigh2[c2]);	
      }
    }    
  }
  
  cout << neigh1.size()<< "  " << neigh2.size()<< endl;
  cout << n1.size()<< "  " << n2.size()<< endl;

  getchar();
  
  //drawCorners(im1, n1, "n1.pgm",255);
  //drawCorners(im2, n2, "n2.pgm",255);cout << "OK"<< endl;getchar();

  delete nrank;
  return 1;
}


void matchDescriptorsNearest(vector<CornerDescriptor*> &cor1, 
			     vector<CornerDescriptor*> &cor2,
			     const float d_dist_max){
  
  int size1=cor1.size();
  int size2=cor2.size();
  

  DARY *ddist = new DARY(size1,size2,0.0);
  DARY *votes = new DARY(size1,size2,0.0);
  DARY *rank = new DARY(size1,size2,200.0);
  DARY *p1dist = new DARY(size1,size1,0.0);
  DARY *p2dist = new DARY(size2,size2,0.0);

  for(unsigned int c1=0;c1<size1;c1++){
    //cout << "\rdescriptor " << c1 << flush;	
    for(unsigned int c2=0;c2<size2;c2++){   	 
      ddist->fel[c1][c2] = distEuc(cor1[c1]->getVec(), cor2[c2]->getVec(),cor2[c2]->getSize());
      //d_dist = distMahalanobis(cor1[c1]->getVec(), cor2[c2]->getVec(), invCovMat); 
    }
  }
  //ddist->write("dist.pgm");
  int flag=0;
  int maxrank=40;
  //ranking rows
  vector<int> rank10;
  for(unsigned int c1=0;c1<size1;c1++){
    cout << "\rdescriptor " << c1 << flush;
    rank10.push_back(0);	
    for(unsigned int c2=0;c2<size2;c2++){   	 
      if(ddist->fel[c1][c2]<d_dist_max){
	flag=1;
	for(int r=0;r<rank10.size() && r<maxrank && flag;r++){
	  if(ddist->fel[c1][c2] < ddist->fel[c1][rank10[r]]){
	    //cout << c2 << "  " << rank10[r] << "  "<<ddist->fel[c1][c2] <<"  "<< ddist->fel[c1][rank10[r]]<< endl;
	    rank10.insert((std::vector<int>::iterator)&(rank10[r]),c2);
	    flag=0;
	  }
	}
	//cout<< c2<< "  OK   "<< rank10.size()<< endl;
	if(flag && rank10.size()<maxrank)rank10.push_back(c2);
      }
    }
    for(int r=0;r<rank10.size() && r<maxrank ;r++){
      //cout << rank10.size()<< "  " << rank10[r]<< "  "<< ddist->fel[c1][rank10[r]]<< "  "<<d_dist_max<<endl;
      rank->fel[c1][rank10[r]]=r;
    }
    rank10.clear();
    //getchar();
  }
  cout << endl;


  //ranking columns
    rank10.clear();
  for(unsigned int c2=0;c2<size2;c2++){
    cout << "\rdescriptor " << c2 << flush;
    rank10.push_back(0);	
    for(unsigned int c1=0;c1<size1;c1++){   	 
      if(ddist->fel[c1][c2]<d_dist_max){
	flag=1;
	for(int r=0;r<rank10.size() && r<maxrank && flag;r++){
	  if(ddist->fel[c1][c2] < ddist->fel[rank10[r]][c2]){
	    //cout << c2 << "  " << rank10[r] << "  "<<ddist->fel[c1][c2] <<"  "<< ddist->fel[c1][rank10[r]]<< endl;
	    rank10.insert((std::vector<int>::iterator)&(rank10[r]),c1);
	    flag=0;
	  }
	}
	if(flag && rank10.size()<maxrank)rank10.push_back(c1);
      }
    }
    for(int r=0;r<rank10.size() && r<maxrank ;r++){
      rank->fel[rank10[r]][c2]+=(r);
    }
    rank10.clear();
    //getchar();
  }
  
  
  for(unsigned int c1=0;c1<size1;c1++){
    for(unsigned int c2=c1+1;c2<size1;c2++){
      p1dist->fel[c1][c2]=square(cor1[c1]->getX()-cor1[c2]->getX())+square(cor1[c1]->getY()-cor1[c2]->getY());
      p1dist->fel[c2][c1]=p1dist->fel[c1][c2];
    }
  }
  for(unsigned int c1=0;c1<size2;c1++){
    for(unsigned int c2=c1+1;c2<size2;c2++){
      p2dist->fel[c1][c2]=square(cor2[c1]->getX()-cor2[c2]->getX())+square(cor2[c1]->getY()-cor2[c2]->getY());
      p2dist->fel[c2][c1]=p2dist->fel[c1][c2];
    }
  }
  
  
  vector<int> neigh1;
  vector<int> neigh2;int nb=0;int c0=-1;
  vector<CornerDescriptor*> n1;
  vector<CornerDescriptor*> n2;
  int radius=0;

  for(unsigned int c1=0;c1<size1;c1++){
    cout << "\rdescriptor " << c1 << flush;
    radius=square(5.0*cor1[c1]->getCornerScale());
    for(unsigned int c=0;c<size1;c++){
      if(p1dist->fel[c1][c]<radius && c1!=c){
	neigh1.push_back(c);
      }
    }
    for(unsigned int c2=0;c2<size2;c2++){
      if(rank->fel[c1][c2]<50){
	radius=square(5.0*cor2[c2]->getCornerScale());
	for(unsigned int c=0;c<size2;c++){ 
	  if(p2dist->fel[c2][c]<radius && c2!=c){
	    neigh2.push_back(c);
	  }
	}
	votes->fel[c1][c2]=computeVotes(cor1, cor2,c1, c2,  neigh1, neigh2, rank);
	neigh2.clear();
      }
    }
    neigh1.clear();
  }


  // rank->normalize(0,10);
  //rank->write("rank.pgm");
  cout << nb<< endl;
  
}



void putVote(DARY *Hsimil, float fx, float fy, float vote){

    int ix = (int)floor(fx);
    int iy = (int)floor(fy);
    float dx = fx-ix;
    float dy = fy-iy;
    if(iy<Hsimil->y()-1 && ix<Hsimil->x()-1){
    Hsimil->fel[iy][ix]+=(1.0 - dy)*(1.0 - dx)*vote;
    Hsimil->fel[iy+1][ix]+=(dy)*(1.0 - dx)*vote;
    Hsimil->fel[iy][ix+1]+=(1-dy)*dx*vote;
    Hsimil->fel[iy+1][ix+1]+=dy*dx*vote;
    }				  
}
 

/*************MATCH BIKES**********************/
void matchScale(vector<DARY* > &Hsimil,vector<CornerDescriptor *> &corners1,
		vector<CornerDescriptor *> &corners2, vector<float> scales, int sc1){
  
  DARY *mahal = new DARY(corners1.size(),corners2.size(),0.0);
  int vsize =  corners1[0]->getSize();
  for(int c1=0;c1<corners1.size();c1++){
    cout << "\r corner " << c1 << " of " << corners1.size()<<"    "<< flush;
    for(int c2=0;c2<corners2.size();c2++){
      mahal->fel[c1][c2]=distEuc(corners1[c1]->getVec(),corners2[c2]->getVec(),vsize);
    }
  }
  float sc = 1.0/scales[sc1];
  float dx,dy, dx1,dy1, dxmax=0,dymax=0;
  int nb=0,nb1=0;;
  for(int c1=0;c1<corners1.size();c1++){
    dx1=sc*(corners1[c1]->getX()-80);
    dy1=sc*(corners1[c1]->getY()-80);
    for(int c2=0;c2<corners2.size();c2++){
      if(mahal->fel[c1][c2]<90000){
	dx=(corners2[c2]->getX()-dx1);
	dy=(corners2[c2]->getY()-dy1);
	if(dx>0 && dy>0){// && (corners1[c1]->getAngle()-corners2[c2]->getAngle())<0.2){
	  //(dxmax<dx)dxmax=dx;
	  //if(dymax<dy)dymax=dy;
	  putVote( Hsimil[sc1], dx/10.0,dy/10.0, 1.0/(1.0+mahal->fel[c1][c2]));
	  //cout << " dx " << dx << " dy " << dy << endl;
	  //nb++;
	}//else nb1++;
      }
    }
  }
  //cout << " nb- " << nb << " nb+ "<< nb1<< endl;
  //cout << " dx- " << dxmax << " dy+ "<< dymax<< endl;
  
  delete  mahal;
} 

void matchScale(vector<DARY* > &Hsimil,CornerDescSequence all_desc1,
		CornerDescSequence all_desc2, vector<float> scales){

  float d1sc,d2sc, sc, sc2;
  DARY *sv = new DARY(1,scales.size(),0.0);
  for(int d1=0;d1<all_desc1.size();d1++){
    d1sc=all_desc1[d1][0]->getCornerScale();
    for(int d2=0;d2<all_desc2.size();d2++){
      d2sc=all_desc2[d2][0]->getCornerScale();
      for(int i=0;i<scales.size();i++){
	sc=(d1sc/d2sc)/scales[i];
	if(sc>1)sc=1/sc;
	if(sc>0.9){
	  cout <<endl<< " d1 " << d1sc << " d2 " << d2sc << "  sc " << (d1sc/d2sc)<< endl;
	  matchScale(Hsimil,all_desc1[d1],all_desc2[d2],scales,i);
	  sv->fel[0][i]++;
	  //cout << " scales  "<<scales[i]<< "  " << i << endl;
	}
      }
      
    }  
  }
  
  //for(int i=0;i<scales.size();i++)
  //	cout << " scales  "<<scales[i]<< "  " << sv->fel[0][i] << endl;
  for(int i=0;i<Hsimil.size();i++){
    Hsimil[i]->normalize(0,0.01);
    Hsimil[i]->write("Hsimil.pgm");
    cout << "Hsimil " << scales[i] << endl;getchar();
  }
  
  delete sv;
}



void matchBikes(DARY *img1, DARY *img2, vector<CornerDescriptor *> &corners1 , vector<CornerDescriptor *> &corners2){
  
  vector<DARY* > Hsimil;
  vector<float > scales;
  float sc=0.5;
  
  
  for( sc=0.5;sc<2.5;sc*=1.4){
    Hsimil.push_back(new DARY(img2->y()/10,img2->x()/10,0.0));
    scales.push_back(sc);
  }
  
  CornerDescSequence all_desc1;
  sc=corners1[0]->getCornerScale();
  all_desc1.push_back(CornerDescList(0));
  for(int i=0;i<corners1.size();i++){
    if(corners1[i]->getCornerScale()!=sc){
      all_desc1.push_back(CornerDescList(0));
      sc=corners1[i]->getCornerScale();
    }
    all_desc1[all_desc1.size()-1].push_back(corners1[i]);
  }


  CornerDescSequence all_desc2;
  sc=corners2[0]->getCornerScale();
  all_desc2.push_back(CornerDescList(0));
  for(int i=0;i<corners2.size();i++){
    if(corners2[i]->getCornerScale()!=sc){
      all_desc2.push_back(CornerDescList(0));
      sc=corners2[i]->getCornerScale();
    }
    all_desc2[all_desc2.size()-1].push_back(corners2[i]);
  }


  /*
  int nb=0;
  for(int i=0;i<all_desc1.size();i++){
    cout << " cor nb " << all_desc1[i].size()<< " scale "<<  all_desc1[i][0]->getCornerScale() << endl;
    nb+=all_desc1[i].size();
  }
  cout << " all points nb "<< nb<< endl;
  nb=0;
  for(int i=0;i<all_desc2.size();i++){
    cout << " cor nb " << all_desc2[i].size()<< " scale "<<  all_desc2[i][0]->getCornerScale() << endl;
    nb+=all_desc2[i].size();
  }
  cout << " all points nb "<< nb<< endl;
  */
  
  matchScale(Hsimil, all_desc1,all_desc2, scales);

  Hsimil.clear();
  
}
