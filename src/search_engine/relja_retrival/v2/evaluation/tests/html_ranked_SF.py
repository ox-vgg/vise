#
# ==== Author:
# 
# Relja Arandjelovic (relja@robots.ox.ac.uk)
# Visual Geometry Group,
# Department of Engineering Science
# University of Oxford
#
# ==== Copyright:
#
# The library belongs to Relja Arandjelovic and the University of Oxford.
# No usage or redistribution is allowed without explicit permission.
#

import sys;
sys.path.insert(0, '/home/relja/rr/src/ui/web');

import Image;
#import StringIO;
import os;

import api;
import details;
from upload import savedTemp;



if __name__=='__main__':
    
    assert( len(sys.argv)==2 );
    
    dsetname= sys.argv[1];
    
    dirname= "/home/relja/Relja/Data/tmp/SF/results/" + dsetname;
    
    evalOut= open("/home/relja/Relja/Data/SF_landmarks/hamm/results/minires.txt",'r').readlines();
    startInd= 0;
    while not(evalOut[startInd].startswith('000 0000')):
        startInd+= 1;
    evalOut= evalOut[startInd:];
    
    firstN= 5;
    
    resultsPerPage= 50;
    
    for queryStartID in range(0, 803, resultsPerPage):
        
        outfn= dirname+'/res/%04d.html' % queryStartID;
        
        navigation= 'Current: %04d <a href="%04d.html">Prev</a> <a href="%04d.html">Next</a>' %  (queryStartID, queryStartID-resultsPerPage, queryStartID+resultsPerPage);
        
        fout= open(outfn, 'w');
        print >>fout, """
        <html><body>
        %s <br><br>
        <table>
        """ % navigation;
        
        for queryID in range(queryStartID, min(queryStartID+resultsPerPage,803)):
            
            print queryID;
            print >>fout, """
            <tr><td>%04d</td>
            <td> <img src="/home/relja/Relja/Databases/SF_landmarks/BuildingQueryImagesCartoIDCorrected-Upright/%04d.jpg" height="200"> </td>
            """ % (queryID, queryID);
            
            imlist= sorted( os.listdir(dirname+'/%04d/' % queryID) );
            
            wasCorrect= False;
            
            for (imfn,i) in zip(imlist[:firstN], range(0,firstN)):
                
                if evalOut[queryID].split()[2][i]=='1':
                    col= "white" if wasCorrect else "green";
                    wasCorrect= True;
                else:
                    col= "red";
                
                print >>fout, """
                <td> <img src="%s/%04d/%s" height="200" style="border:5px solid %s"> </td>
                """ % (dirname, queryID, imfn, col);
            
            
            print >>fout, """
            </tr>
            """;
        
        print >>fout, """
        </table><br><br>
        %s
        </body></html>
        """ % navigation;
        fout.close();
    