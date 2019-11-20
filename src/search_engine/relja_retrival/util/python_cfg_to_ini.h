/*
==== Author:

Relja Arandjelovic (relja@robots.ox.ac.uk)
Visual Geometry Group,
Department of Engineering Science
University of Oxford

==== Copyright:

The library belongs to Relja Arandjelovic and the University of Oxford.
No usage or redistribution is allowed without explicit permission.
*/

#include <fstream>
#include <string>

#include "macros.h"


// convert Python config file into a valid ini file (remove multiline options)

void pythonCfgToIni( std::string const &configFn, std::string const &iniFn ){
    
    std::ifstream f(configFn.c_str());
    ASSERT(f.is_open());
    std::ofstream fout(iniFn.c_str());
    ASSERT(fout.is_open());
    
    bool inMultiLine= false;
    
    std::string s;
    
    while (std::getline(f, s)){
        
        if (s.length()>0 && (s[0]=='#' || s[0]==';'))
            continue;
        
        if (inMultiLine){
            
            // is it the end of multi line (does it start with a non-white character?)
            if ( s.length()==0 || !(s.length()>1 && isspace(s[0])) ) {
                fout<<"\n";
                inMultiLine= false;
            } else {
                fout<<s;
                continue;
            }
            
        }
        
        ASSERT( !inMultiLine );
        
        // check start inMultiLine (i.e. last non-white character in a line is '=' )
        int i;
        for (i= s.length()-1; i>0 && isspace(s[i]); --i);
        if (i>0 && s[i]=='='){
            fout<<s;
            inMultiLine= true;
        } else {
            fout<<s<<"\n";
        }
        
    }
    
    fout.close();
    f.close();
    
}