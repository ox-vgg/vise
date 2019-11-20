#include "jetLocalDirectional.h"


void JetLocalDirectional::computeComponents(DARY *img_in){
	  ((JetLocal*)this)->computeComponents(img_in); 
	  if (!isOK())return;
      double a_tmp;
      double sina=0,cosa=0;
      double *val = new double[size];
      int inv=0;
      val[inv++]=vec[0];
      /*1st derivative*/
      if(order>=1){
        sina=sin(angle);cosa=cos(angle);
        val[inv++]=vec[1]*cosa+vec[2]*sina;
        a_tmp=(M_PI/2)+angle;
        sina=sin(a_tmp);cosa=cos(a_tmp);
        val[inv++]=vec[1]*cosa+vec[2]*sina; //equals ZERO if angle==grad direction
      }
      /*2nd derivative*/
      if(order>=2){
        val[inv++]=vec[3]*cosa*cosa+2*vec[4]*sina*cosa+vec[5]*sina*sina;
        a_tmp=(M_PI/3)+angle;
        sina=sin(a_tmp);cosa=cos(a_tmp);
        val[inv++]=vec[3]*cosa*cosa+2*vec[4]*sina*cosa+vec[5]*sina*sina;
        a_tmp=2*(M_PI/3)+angle;
        sina=sin(a_tmp);cosa=cos(a_tmp);
        val[inv++]=vec[3]*cosa*cosa+2*vec[4]*sina*cosa+vec[5]*sina*sina;
      }
      /*3d derivative */
      if(order>=3){
        sina=sin(angle);cosa=cos(angle);
        val[inv++]=vec[6]*cosa*cosa*cosa+3*vec[7]*cosa*cosa*sina+
          3*vec[8]*sina*sina*cosa+vec[9]*sina*sina*sina;
        a_tmp=(M_PI/4)+angle;
        sina=sin(a_tmp);cosa=cos(a_tmp);
        val[inv++]=vec[6]*cosa*cosa*cosa+3*vec[7]*cosa*cosa*sina+
          3*vec[8]*sina*sina*cosa+vec[9]*sina*sina*sina;
        a_tmp=2*(M_PI/4)+angle;
        sina=sin(a_tmp);cosa=cos(a_tmp);
        val[inv++]=vec[6]*cosa*cosa*cosa+3*vec[7]*cosa*cosa*sina+
          3*vec[8]*sina*sina*cosa+vec[9]*sina*sina*sina;
        a_tmp=3*(M_PI/4)+angle;
        sina=sin(a_tmp);cosa=cos(a_tmp);
        val[inv++]=vec[6]*cosa*cosa*cosa+3*vec[7]*cosa*cosa*sina+
          3*vec[8]*sina*sina*cosa+vec[9]*sina*sina*sina;
      }
      /*4th derivative */
      if(order>=4){
        sina=sin(angle);cosa=cos(angle);
        val[inv++]=vec[10]*cosa*cosa*cosa*cosa+4*vec[11]*cosa*cosa*cosa*sina+
          6*vec[12]*cosa*cosa*sina*sina+4*vec[13]*cosa*sina*sina*sina+vec[14]*sina*sina*sina*sina;
        a_tmp=(M_PI/5)+angle;
        sina=sin(a_tmp);cosa=cos(a_tmp);
        val[inv++]=vec[10]*cosa*cosa*cosa*cosa+4*vec[11]*cosa*cosa*cosa*sina+
          6*vec[12]*cosa*cosa*sina*sina+4*vec[13]*cosa*sina*sina*sina+vec[14]*sina*sina*sina*sina;
        a_tmp=2*(M_PI/5)+angle;
        sina=sin(a_tmp);cosa=cos(a_tmp);
        val[inv++]=vec[10]*cosa*cosa*cosa*cosa+4*vec[11]*cosa*cosa*cosa*sina+
          6*vec[12]*cosa*cosa*sina*sina+4*vec[13]*cosa*sina*sina*sina+vec[14]*sina*sina*sina*sina;
        a_tmp=3*(M_PI/5)+angle;
        sina=sin(a_tmp);cosa=cos(a_tmp);
        val[inv++]=vec[10]*cosa*cosa*cosa*cosa+4*vec[11]*cosa*cosa*cosa*sina+
          6*vec[12]*cosa*cosa*sina*sina+4*vec[13]*cosa*sina*sina*sina+vec[14]*sina*sina*sina*sina;
        a_tmp=4*(M_PI/5)+angle;
        sina=sin(a_tmp);cosa=cos(a_tmp);
        val[inv++]=vec[10]*cosa*cosa*cosa*cosa+4*vec[11]*cosa*cosa*cosa*sina+
          6*vec[12]*cosa*cosa*sina*sina+4*vec[13]*cosa*sina*sina*sina+vec[14]*sina*sina*sina*sina;
      //cout << a_tmp << " sin " << sina << " cos " << cosa << endl; 
        //getchar();
      }
	  allocVec(inv);
	  for(int i=0;i<size;i++)vec[i]=val[i];
	  delete[] val; 

	state=1;
}

void computeJLDDescriptors(DARY *image, vector<CornerDescriptor *> &desc){
    JetLocalDirectional * ds=new JetLocalDirectional();
    for(unsigned int c=0;c<desc.size();c++){
	cout << "\rcompute "<< c<< "    "<< flush;
	ds->copy(desc[c]);	
	ds->computeComponents(image);
	//ds->changeBase(ds->getSize(),baseJLD);
 	desc[c]->copy((CornerDescriptor*)ds); 
    }
    for(unsigned int c=0;c<desc.size();c++){
      if(!desc[c]->isOK()){
	desc.erase((std::vector<CornerDescriptor*>::iterator)&desc[c]);
	c--;
      }
    }    
    cout<<endl;
}

