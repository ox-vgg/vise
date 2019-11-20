// Ici les histogrammes seront repr'edent'es comme de simples vecteurs.
#include <stream.h>
#include <fstream.h>
#include <math.h>
#include <stdlib.h>
#include <dbary.h>
#include <descriptors.h>



#include <CalculC8Codes.h>

// tH correspond a la moitie du cote de la fenetre de calcul de l'histogramme

int distBits(int a, int b){
  int t= ((a | b)-(a & b));
  int sum=0;
  for(int i=0;i<9;i++){
    sum+=(t>>i)&0x01;
  }  
  return sum;
}

void HistoC4OIncrem(double** image,int x,int y,int tH,int tzone,double* histo)
{
  int i,j;
  int code=0;

  for( i = -tH ; i <= tH ; ++i)
    for( j = -tH ; j <= tH ; ++j)
      {
	  //code = CodeC4O(x+i,y+j,image,tzone);

	histo[code] ++;
      }
}
void HistoC4O(double** image,int x,int y,int tH,int tzone,double*& histo)
{
  int i,j;
  int code=0;

  histo = new double [256];
  for( i=0 ; i < 256 ; ++i) histo[i] = 0;


  for( i = -tH ; i <= tH ; ++i)
    for( j = -tH ; j <= tH ; ++j)
      {
	  //code = CodeC4O(x+i,y+j,image,tzone);

	histo[code] ++;
      }
}


// ATTENTION on suppose que l'image passee en parametre a deja ete lissee
// conformement au mode de calcul des codes C8_?

void HistoC8_3(double** image,int x,int y,int tH,int tzone,int mu,double*& histo)
{

  int i,j;
  int code;

  histo = new double [6561]; 

 for( i = 0 ; i < 6561 ; ++i)
    histo[i] = 0;

  for( i = -tH ; i <= tH ; ++i)
    for( j = -tH ; j <= tH ; ++j)
      {
	code = CodeC8_3(x+i,y+j,image,tzone,mu);

	histo[code] ++;
      }
}

int HistoC8_3Robuste(double** image,int x,int y,int tH,int tzone,int mu,double*& histo)
{

  int i,j;
  int code;
  int nbsatures =0;

  histo = new double [6561]; 

  for( i = -tH ; i <= tH ; ++i)
    for( j = -tH ; j <= tH ; ++j) 
      if(image[y+j][x+i] == 255)
	nbsatures++;

  if( nbsatures > tH*tH*0.5) 
    return 0;



 for( i = 0 ; i < 6561 ; ++i)
    histo[i] = 0;

  for( i = -tH ; i <= tH ; ++i)
    for( j = -tH ; j <= tH ; ++j)
      {
	code = CodeC8_3(x+i,y+j,image,tzone,mu);

	histo[code] ++;
      }

  return 1;
}

void HistoC8_3_2(double** image,int x,int y,int tH,int tzone,int mu,double* histo)
{

  int i,j;
  int code;

 for( i = 0 ; i < 6561 ; ++i)
    histo[i] = 0;

  for( i = -tH ; i <= tH ; ++i)
    for( j = -tH ; j <= tH ; ++j)
      {
	code = CodeC8_3(x+i,y+j,image,tzone,mu);

	histo[code] ++;
      }
}

void HistoC8_3Increm(double** image,int x,int y,int tH,int tzone,int mu,double* histo)
{

  int i,j;
  int code;

  for( i = -tH ; i <= tH ; ++i)
    for( j = -tH ; j <= tH ; ++j)
      {
	code = CodeC8_3(x+i,y+j,image,tzone,mu);

	histo[code] ++;
      }
}
void HistoC8_2Increm(double** image,int x,int y,int tH,int tzone,double* histo)
{

  int i,j;
  int code;

  for( i = -tH ; i <= tH ; ++i)
    for( j = -tH ; j <= tH ; ++j)
      {
	code = CodeC8_2(x+i,y+j,image,tzone);
	
	for(int c=0;c<256;c++){
	    if(distBits(c,code)<3)
		histo[c] ++;
	}
      }
}

void HistoC8_2(double** image,int x,int y,int tH,int tzone,double*& histo)
{

  int i,j;
  int code;

  histo = new double [256]; 

 for( i = 0 ; i < 256 ; ++i)
    histo[i] = 0;

 for( i = -tH ; i <= tH ; ++i)
    for( j = -tH ; j <= tH ; ++j)
      {
	code = CodeC8_2(x+i,y+j,image,tzone);

	histo[code] ++;
      }
}

int HistoC8_2Robuste(double** image,int x,int y,int tH,int tzone,double*& histo)
{

  int i,j;
  int code;
  int nbsatures;

  histo = new double [256]; 

 for( i = 0 ; i < 256 ; ++i)    histo[i] = 0;

 nbsatures = 0;

  for( i = -tH ; i <= tH ; ++i)
    for( j = -tH ; j <= tH ; ++j) 
      if(image[y+j][x+i] == 255)
	nbsatures++;

  if( nbsatures > tH*tH*0.2) // ce qui represente 12% de la surface (a peu pres)
    return 0;


 for( i = -tH ; i <= tH ; ++i)
    for( j = -tH ; j <= tH ; ++j)
      {
	code = CodeC8_2(x+i,y+j,image,tzone);

	histo[code] ++;
      }

 return 1;
}



void HistoC8_2_2(double** image,int x,int y,int tH,int tzone,double* histo)
{

  int i,j;
  int code;

 for( i = 0 ; i < 256 ; ++i)
    histo[i] = 0;

 for( i = -tH ; i <= tH ; ++i)
    for( j = -tH ; j <= tH ; ++j)
      {
	code = CodeC8_2(x+i,y+j,image,tzone);

	histo[code] ++;
      }
}


void HistoNP(DARY *image, CornerDescriptor *desc, int tH, int tzone, int bits, int dist){
    int code;
    double angle=0;
    double x_,y_;
    double x= desc->getCorner()->getX();
    double y= desc->getCorner()->getY();   
    double *histo = desc->getVec();
    int max=(int)pow(2,bits);
    double da=0;
    for(int i = 2 ; i <= tH ; i++){
	angle = 0;
	da=(M_PI/(i*4.0));
	for(int j = 1 ; j <= i*8 ; j++){
	    y_=y+sin(angle)*i;
	    x_=x+cos(angle)*i;
	    angle+=da;
	    code = CodeNP(x, y, x_, y_, image->fel, tzone, bits);
	    for(int c=0;c<max;c++){
		if(distBits(c,code)<=dist)
		    histo[c] ++;
	    }
	} 
    }
}

void HistoC8_2(DARY *image, CornerDescriptor *desc, int tH,int tzone, int dist)
{

  int i,j;
  int code;
  int x= (int)rint(desc->getCorner()->getX());
  int y= (int)rint(desc->getCorner()->getY());   
  double *histo = desc->getVec();
  int max=(int)pow(2,8);
  for( i = -tH ; i <= tH ; ++i)
    for( j = -tH ; j <= tH ; ++j)
      {
	//code = CodeC8_2(x+i,y+j,image->fel,tzone);
	code = CodeNP(x, y, x+i, y+j, image->fel, tzone, 8);
	    for(int c=0;c<max;c++){
		if(distBits(c,code)<=dist)
		    histo[c] ++;
	    }
      }
}



