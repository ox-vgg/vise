#include <sstream>
#include "cornerDescriptor.h"

extern float scale_mult;


/**********************************************/
void CornerDescriptor::copy(CornerDescriptor* ds){
  x=ds->getX(); 
  y=ds->getY();
  type=ds->getType();
  cornerness=ds->getCornerness();
  angle=ds->getAngle();
  c_scale=ds->getCornerScale();
  d_scale=ds->getDescriptorScale();
  der_sig=ds->getDerSig();
  int_sig=ds->getIntSig();
  state=ds->isOK();
  model=ds->getModel();
  order=ds->getOrder();
  extr=ds->isExtr();
  l1=ds->getL1();
  l2=ds->getL2();
  mi11=ds->getMi11();
  mi12=ds->getMi12();
  mi21=ds->getMi21();
  mi22=ds->getMi22();
  allocVec(ds->getSize());
  for(int i=0;i<size;i++)vec[i]=ds->getV(i);
}


/**********************************************/
void CornerDescriptor::initd(void){
  vec=NULL;
  d_scale=0;
  order=4;
  size=0; 
} 

/**********************************************/
void CornerDescriptor::allocVec(int size_in){
    if(size_in >0){
	if(vec!=NULL)delete [] vec;
	size=size_in;
	vec = new float[size];
	for(int i=0;i<size;i++)vec[i]=0;
    }
}
  

/**********************************************/
void CornerDescriptor::read_database( ifstream &input){
    float *write_vec = new float[size+10];
    //input.read(write_vec,(int)((size+10)*sizeof(float)));
    model=(long unsigned int)write_vec[0];
    x=write_vec[1];
    y=write_vec[2]; 
    cornerness=write_vec[3]; 
    c_scale=d_scale=write_vec[4];
    angle=write_vec[5];
    mi11=write_vec[6];
    mi12=write_vec[7];
    mi21=write_vec[8];
    mi22=write_vec[9];
    for(int i=0;i<size;i++)
	vec[i]=write_vec[i+10];
    delete [] write_vec;
    
}
/**********************************************/
void CornerDescriptor::write_database( ofstream &output, long unsigned int model_in){
    model=model_in;
    float *write_vec = new float[size+10];
    write_vec[0]=(float)model;
    write_vec[1]=x;
    write_vec[2]=y; 
    write_vec[3]=cornerness; 
    write_vec[4]=c_scale;
    write_vec[5]=angle;
    write_vec[6]=mi11;
    write_vec[7]=mi12;
    write_vec[8]=mi21;
    write_vec[9]=mi22;
    for(int i=0;i<size;i++)
	write_vec[i+10]=vec[i];
    //output.write(write_vec,(size+10)*sizeof(float));
    delete [] write_vec;
   
}
 
/**********************************************/
void CornerDescriptor::read( ifstream &input, int size_in){
  input >> x;
  input >> y; 
  input >> cornerness; 
  input >> c_scale;
  input >> angle;
  input >> type;
  input >> lap;
  input >> extr;
  input >> mi11;
  input >> mi12;
  input >> mi21;
  input >> mi22;
  c_scale*=3;
  d_scale=c_scale;
  if(size_in>0){
      //input >> d_scale;
    allocVec(size_in);
    for(int j=0;j<size;j++){
      input >> vec[j];
    }
  } 
} 
/**********************************************/
void CornerDescriptor::readFred( ifstream &input, int size_in){
  input >> y;
  input >> x;
  input >> c_scale;
  input >> angle;
  if(size_in>0){
      //input >> d_scale;
    allocVec(size_in);
    for(int j=0;j<size;j++){
      input >> vec[j];
    }
  } 
} 
/**********************************************/
void CornerDescriptor::readCommon( ifstream &input, int size_in){
  double a,b,c;
  Matrix U(2,2,0.0),D,Vi,V;

  // -- JAMES --
  string line;
  getline(input, line);
  std::istringstream iss(line);
  if (line.size() < 2) return;
  // From this line on, input->iss.

  iss >> x;
  iss >> y;
  iss >> a;
  iss >> b;
  iss >> c;
  U(1,1)=a;
  U(1,2)=b;
  U(2,1)=b;
  U(2,2)=c;
  U.svd(Vi,D,V);
  D(1,1)=(1.0/sqrt(D(1,1)));
  D(2,2)=(1.0/sqrt(D(2,2)));
  a=sqrt(D(2,2)*D(1,1));
  //cout << D(1,1)<< " " <<  D(2,2)<< "  " << tmp1<< endl;
  D.tabMat[2][2]/=a;
  D.tabMat[1][1]/=a;
  U=V*D*V.transpose();
  // JAMES
  //c_scale=3.0*a;
  c_scale = scale_mult * a;
  // -----
  //cout << x << " " << y << " " << a << " "<< b << " "<< c << " " << size_in<<endl;getchar();
  mi11=U(1,1);
  mi12=U(1,2);
  mi21=U(2,1);
  mi22=U(2,2);
  
  if(size_in>0){
    allocVec(size_in);
    for(int j=0;j<size;j++){
      iss >> vec[j];
    }
  } 
} 



/**********************************************/
void CornerDescriptor::write(ofstream &output){
  output << x << " " << y << " " << cornerness << " " << (c_scale/3.0) << " ";
  output << angle<< " "<< type << " " << lap << " " <<extr;
  output << " " << mi11 << " " << mi12 << " " << mi21 << " " << mi22<< " ";
  if(size>0){
      //output  << "  " << d_scale << " ";
    for(int j=0; j<size;j++){
      output << vec[j] << " ";
    }
  }
  output << endl;
} 
/**********************************************/
void CornerDescriptor::writeCommon(ofstream &output){

    Matrix U(2,2,0.0),D,Vi,V;
    U(1,1)=mi11;
    U(1,2)=mi12;
    U(2,1)=mi21;
    U(2,2)=mi22;
    U.svd(Vi,D,V);
    //D=D*(c_scale/3.0); Write it out at what was computed.
    D = D*c_scale;
    D(1,1)=1.0/(D(1,1)*D(1,1));
    D(2,2)=1.0/(D(2,2)*D(2,2));
    U=V*D*V.transpose();
    
    output << x << " " << y << " " << U(1,1)<< " "<< U(1,2)<< " "<< U(2,2);
    if(size>0){
      for(int j=0; j<size;j++){
	output  << " " << vec[j];
      }
    }
    output << endl;
} 

bool
is_ellipse_soln(float a, float b, float c, float y)
{
  float val = y*y*(b*b - a*c) + a;
  if (val < 0) 
    return false;
  else
    return true;
}

bool
CornerDescriptor::is_fully_inside(float x1, float x2, float y1, float y2)
{
  if (x1 <= x && x < x2 && y1 <= y && y < y2) {
    Matrix U(2,2,0.0),D,Vi,V;
    U(1,1)=mi11;
    U(1,2)=mi12;
    U(2,1)=mi21;
    U(2,2)=mi22;
    U.svd(Vi,D,V);
    //D=D*(c_scale/3.0); Write it out at what was computed.
    D = D*c_scale;
    D(1,1)=1.0/(D(1,1)*D(1,1));
    D(2,2)=1.0/(D(2,2)*D(2,2));
    U=V*D*V.transpose();

//    cout << x << " " << y << " " << U(1,1) << " " << U(1,2) << " " << U(2,2) << endl;
//    getchar();

    if (is_ellipse_soln(U(1,1),U(1,2),U(2,2),y - y1) || is_ellipse_soln(U(1,1),U(1,2),U(2,2),y - y2) ||
        is_ellipse_soln(U(2,2),U(1,2),U(1,1),x - x1) || is_ellipse_soln(U(2,2),U(1,2),U(1,1),x - x2))
      return false;
    else
      return true;
  }
  else
    return false;
}

void
CornerDescriptor::writeCommonWA(ofstream &output)
{
    Matrix U(2,2,0.0),D,Vi,V;
    U(1,1)=mi11;
    U(1,2)=mi12;
    U(2,1)=mi21;
    U(2,2)=mi22;
    U.svd(Vi,D,V);
    //D=D*(c_scale/3.0); Write it out at what was computed.
    D = D*c_scale;
    D(1,1)=1.0/(D(1,1)*D(1,1));
    D(2,2)=1.0/(D(2,2)*D(2,2));
    U=V*D*V.transpose();
    
    output << x << " " << y << " " << angle << " " << U(1,1)<< " "<< U(1,2)<< " "<< U(2,2);
    if(size>0){
      for(int j=0; j<size;j++){
	output  << " " << vec[j];
      }
    }
    output << endl;

}
  
/**********************************************/
void CornerDescriptor::changeBase(Matrix *base){
  float val;
  float *vect = new float[base->nbCols()];
  for(int row=1;row<= base->nbRows();row++){
    val=0;
    for(int col=1;col<= base->nbCols();col++){
      val+=base->tabMat[row][col]*vec[col-1];
    }
    vect[row-1]=val;
    
  }
  for(int col=0;col< base->nbCols();col++)
      vec[col]=vect[col];
  delete []vect;
}

/**********************************************/
void CornerDescriptor::changeBase(float *mat){
  for(int v=0;v<size;v++){
    vec[v] = vec[v]*mat[v];   
  }
}
/**********************************************/
void CornerDescriptor::pca(int dim, float *avg, float *base){

  float *outvec = new float[dim];
  for(int v=0;v<dim;v++){
    outvec[v]=0;
  }
  for(int v=0;v<size;v++){
    vec[v]-=avg[v];
  }
  uint cnt=0;
  for(int i=0;i<dim;i++){
    for(int v=0;v<size;v++,cnt++){
      outvec[i] += vec[v]*base[cnt];   
    }
  }
  allocVec(dim);
  for(int v=0;v<size;v++){
    vec[v]=outvec[v];
  }
  delete outvec;
}

/**********************************************/
void CornerDescriptor::changeBase(int dim,float *mat){
  float val;
  float *vect = new float[dim];
  for(int row=0;row<dim;row++){
    val=0;
    for(int col=0;col<dim; col++){
      val+=mat[row*dim+col]*vec[col];
    }
    vect[row]=val;
    
  }
  for(int col=0;col< dim;col++)
      vec[col]=vect[col];
  delete []vect;
}

/**********************************************/
void changeBase(vector<CornerDescriptor *> &desc, Matrix *base){  
  for(unsigned int c=0;c<desc.size();c++){
    desc[c]->changeBase(base);
  } 
}

/**********************************************/
void loadFredCorners( const char* points1, vector<CornerDescriptor*> &cor1){
    CornerDescriptor* cor;
    ifstream input1(points1);
    if(!input1)return;
    int cor_nb1=0,size=0; 
    input1 >> cor_nb1;
    if(cor_nb1==0)return;
    input1 >> size;   
    do{
      cor = new CornerDescriptor();
      cor->readFred(input1,size);
      cor1.push_back(cor);      
    }while(!input1.eof());
    cor1.erase((std::vector<CornerDescriptor*>::iterator) &cor1[(int)cor1.size()-1]);
    if(cor_nb1!=(int)cor1.size()){
      cout << "warning:"<< endl<<"in header: "<<cor_nb1 << ", in file: "<< cor1.size()<< endl; 
    }

}
/**********************************************/
void loadCorners( const char* points1, vector<CornerDescriptor*> &cor1, int format){
    CornerDescriptor* cor;
    ifstream input1(points1);
    if(!input1)return;
    int cor_nb1,size; 
    float tmp;
    
    string line;
    getline(input1, line);
    tmp = atof(line.c_str());
    getline(input1, line);
    cor_nb1 = atoi(line.c_str());
    
    //input1 >> tmp;
    //input1 >> cor_nb1;   //cout << cor_nb1 << endl;
    if(tmp<=1.0)size=0;
    else size=(int)tmp;
    if(cor_nb1==0)return;
    while(input1.good()){
      cor = new CornerDescriptor();
      if(format==0)
	cor->read(input1,size);
      else
	cor->readCommon(input1,size);
      cor1.push_back(cor); //cout << "read " <<cor1.size()<<endl;     
    }
    delete cor1[(int)cor1.size()-1];
    cor1.erase((std::vector<CornerDescriptor*>::iterator) &cor1[(int)cor1.size()-1]);
    if(cor_nb1!=(int)cor1.size()){
      cout << "warning:"<< endl<<"in header: "<<cor_nb1 << ", in file: "<< cor1.size()<< endl; 
    }

}

/**********************************************/
void writeCorners(vector<CornerDescriptor*> cor, const char* points_out, int format){
  if (cor.size()==0){
      // cout << " descriptors nb " << cor.size() << endl;
      return; 
  }
    ofstream output(points_out);
    if(!output)cout << "error opening " << points_out<< endl;
    output << cor[0]->getSize()<< endl;
    output << cor.size()<< endl;
    for(unsigned int i=0; i<cor.size();i++){
      if(format==0)
	cor[i]->write(output);
      else if (format == 1) cor[i]->writeCommon(output);
      else if (format == 2) cor[i]->writeCommonWA(output);
    }  
    output.close();  
} 

