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
    
    assert( len(sys.argv)==3 );
    
    dsetname= sys.argv[1];
    datasetOpt= {'dsetname': dsetname, 'APIport': int(sys.argv[2]), 'enableUpload': False};
    
    APIhost = "localhost";
    API_obj= api.API( datasetOpt['APIport'], APIhost= APIhost, scoreThr= None);
    
    assert( API_obj.running() );
    
    details_obj= details.details(None, {"dsetname": API_obj}, {"dsetname": API_obj.docMap}, None);
    
    dirname= "/home/relja/Relja/Data/tmp/SF/results/" + dsetname;
    try:
        os.mkdir( dirname );
    except:
        pass
    
    for queryID in range(0, 803):
        
        print queryID;
        
        compDataFilename= "/home/relja/Relja/Data/SF_landmarks/query/%04d.v2bin" % queryID;
        results= API_obj.externalQuery(compDataFilename, startFrom= 0, numberToReturn= 10);
        
        prevInd= -1;
        
        try:
            os.mkdir( dirname + "/%04d/" % queryID );
        except:
            pass;
        
        for (ind, resDocID, score, H) in results:
            
            if ind%10==0: print " %d" % ind;
            assert(ind==prevInd+1);
            prevInd= ind;
            
            st= savedTemp();
            st['compDataFilename']= compDataFilename;
            st['localFilename_jpg']= '/home/relja/Relja/Databases/SF_landmarks/BuildingQueryImagesCartoIDCorrected-Upright/%04d.jpg' % queryID;
            matchImage= details_obj.drawMatches(uploadID1= st, docID2= resDocID, xl= None, xu= None, yl= None, yu= None, drawBoxes= "false", drawPutative= "true", drawLines= "true", drawRegions= "true");
            
            #im= Image.open(StringIO.StringIO(matchImage));
            #im.show();
            #raw_input("Press Enter to continue...");
            
            outfn= dirname + "/%04d/%04d_%06d.jpg" % (queryID, ind, resDocID);
            
            f= open(outfn, 'wb'); print >>f, matchImage; f.close();
            
            #im= Image.open(outfn);
            #im.show();
            #raw_input("Press Enter to continue...");
    