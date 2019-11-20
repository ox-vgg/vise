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

import sys, os;
from socket import *;
import time;
import xml.parsers.expat;
import uuid;

import numpy as np;

import api_request;

import get_scriptroot;
scriptroot= get_scriptroot.getScriptroot();

from upload import savedTemp;



class API:
    
    
    
    # don't forget to set .pathManager_obj
    def __init__(self, APIport, APIhost= "localhost", scoreThr= None, verbose= False ):
        
        self.APIport= APIport;
        self.APIhost= APIhost;
        self.scoreThr= scoreThr;
        self.verbose= verbose;
        self.docMap= documentMapUsingAPI(self);
    
    
    
    def __del__(self):
        
        if self.enableUpload:
            del self.dassignPre_obj;
    
    
    
    def running( self ):
        
        sock= socket(AF_INET, SOCK_STREAM);
        running= True;
        
        try:
            sock.connect((self.APIhost, self.APIport));
        except error, msg:
            running= False;
        
        sock.close();
        return running;
        
        
        
    def customRequest( self, request, appendEnd= True ):
        return api_request.customRequest(self.APIhost, self.APIport, request, appendEnd= appendEnd);
    
    
    
    def getResults(self, reply):
        
        parser= xml.parsers.expat.ParserCreate();
        resultParser_obj= resultParser();
        parser.StartElementHandler= resultParser_obj.startHandler;
        parser.Parse(reply,1);
        
        results= resultParser_obj.results;
        
        if self.scoreThr!=None:
            resultsAll= results;
            results= [];
            for (rank, docID, score, H) in resultsAll:
                if score<self.scoreThr:
                    break;
                results.append( (rank, docID, score, H) );
        
        if self.verbose:
            for (rank, docID, score, H) in results:
                print rank, docID, score, H;
        
        return results;
    
    
    
    def supportsInternalQueryROI(self):
        return True;
    
    
    
    def internalQuery( self, docID= None, xl= None, xu= None, yl= None, yu= None, startFrom= 0, numberToReturn= 20 ):
        
        request= "<internalQuery>";
        request+= API.getDocID( docID );
        request+= API.getQueryRegion( xl, xu, yl, yu );
        request+= "<startFrom>%d</startFrom><numberToReturn>%d</numberToReturn>\n" % (startFrom, numberToReturn);
        request+= "</internalQuery>";
        
        reply= self.customRequest( request );
        
        results= self.getResults(reply);
        return results;
    
    
    
    def getGeneralMatches(self, request ):
        
        reply= self.customRequest( request );
        
        parser= xml.parsers.expat.ParserCreate();
        matchesParser_obj= matchesParser();
        parser.StartElementHandler= matchesParser_obj.startHandler;
        parser.Parse(reply,1);
        
        if self.verbose:
            for ( el1, el2 ) in matchesParser_obj.matches:
                print el1, el2;
        
        return matchesParser_obj.matches;
    
    
    
    def getInternalMatches( self, docID1= None, docID2= None, xl= None, xu= None, yl= None, yu= None ):
        request= "<getInternalMatches>";
        request+= API.getDocID( docID1, tgD= "docID1" );
        request+= API.getDocID( docID2, tgD= "docID2" );
        request+= API.getQueryRegion( xl, xu, yl, yu );
        request+="</getInternalMatches>";
        return self.getGeneralMatches(request);
    
    
    
    def getPutativeInternalMatches( self, docID1= None, docID2= None, xl= None, xu= None, yl= None, yu= None ):
        request= "<getPutativeInternalMatches>";
        request+= API.getDocID( docID1, tgD= "docID1" );
        request+= API.getDocID( docID2, tgD= "docID2" );
        request+= API.getQueryRegion( xl, xu, yl, yu );
        request+="</getPutativeInternalMatches>";
        return self.getGeneralMatches(request);
    
    
    
    def getExternalMatches( self, wordFn1, docID2= None, xl= None, xu= None, yl= None, yu= None ):
        request= "<getExternalMatches>";
        request+= "<wordFn1>%s</wordFn1>" % wordFn1;
        request+= API.getDocID( docID2, tgD= "docID2" );
        request+= API.getQueryRegion( xl, xu, yl, yu );
        request+="</getExternalMatches>";
        return self.getGeneralMatches(request);
    
    
    
    def getPutativeExternalMatches( self, wordFn1, docID2= None, xl= None, xu= None, yl= None, yu= None ):
        request= "<getPutativeExternalMatches>";
        request+= "<wordFn1>%s</wordFn1>" % wordFn1;
        request+= API.getDocID( docID2, tgD= "docID2" );
        request+= API.getQueryRegion( xl, xu, yl, yu );
        request+="</getPutativeExternalMatches>";
        return self.getGeneralMatches(request);
    
    
    
    def externalQuery( self, wordFn, xl= None, xu= None, yl= None, yu= None, startFrom= 0, numberToReturn= 20 ):
        
        request= "<externalQuery>";
        request+= "<wordFn>%s</wordFn>" % wordFn;
        request+= "<startFrom>%d</startFrom><numberToReturn>%d</numberToReturn>\n" % (startFrom, numberToReturn);
        request+= API.getQueryRegion( xl, xu, yl, yu );
        request+="</externalQuery>";
        
        reply= self.customRequest( request );
        
        results= self.getResults(reply);
        return results;
    
    
    
    def multiQuery( self, querySpecs, rois= None, startFrom= 0, numberToReturn= 20 ):
        
        request= "<multiQuery>";
        request+= "<startFrom>%d</startFrom><numberToReturn>%d</numberToReturn>\n" % (startFrom, numberToReturn);
        request+= "<numQ>%d</numQ>" % len(querySpecs);
        for i in range(0, len(querySpecs)):
            if type(querySpecs[i])==int:
                request+= "<docID%d>%d</docID%d>" % (i,querySpecs[i],i);
            else:
                request+= "<wordFn%d>%s</wordFn%d>" % (i,querySpecs[i],i);
        if rois!=None:
            for i in range(0, len(rois)):
                roi= rois[i];
                request+= API.getQueryRegion( roi[0], roi[1], roi[2], roi[3], '%d'%i );
        request+="</multiQuery>";
        
        reply= self.customRequest( request );
        
        results= self.getResults(reply);
        return results;
    
    
    
    def processImage( self, imageFn, compDataFn, ROI= None ):
        # don't care about ROI as this will be handled in externalQuery
        
        request= "<processImage>";
        request+= "<imageFn>%s</imageFn>" % imageFn;
        request+= "<compDataFn>%s</compDataFn>" % compDataFn;
        request+= "</processImage>";
        reply= self.customRequest( request );
    
    
    
    def register( self, docID1= None, docID2= None, xl= None, xu= None, yl= None, yu= None ):
        
        request= "<register>";
        request+= API.getDocID( docID1, tgD= "docID1" );
        request+= API.getDocID( docID2, tgD= "docID2" );
        request+= API.getQueryRegion( xl, xu, yl, yu );
        
        registerID= str(uuid.uuid4());
        outFnPrefix= os.path.join( scriptroot, 'tmp' );
        outFn1= os.path.join(outFnPrefix, '%s_im1.jpg' % registerID);
        outFn2= os.path.join(outFnPrefix, '%s_im2.jpg' % registerID);
        outFn2t= os.path.join(outFnPrefix, '%s_im2t.jpg' % registerID);
        
        request+= "<outFn1>%s</outFn1>" % outFn1;
        request+= "<outFn2>%s</outFn2>" % outFn2;
        request+= "<outFn2t>%s</outFn2t>" % outFn2t;
        
        # original file names (e.g. in case where we have large images, ones like for ballads, index on resizes but want to be able to show originals)
        fullSizeFn1= self.pathManager_obj.getFullSize( self.pathManager_obj.docMap.getFn(int(docID1)) );
        fullSizeFn2= self.pathManager_obj.getFullSize( self.pathManager_obj.docMap.getFn(int(docID2)) );
        
        request+= "<fullSizeFn1>%s</fullSizeFn1>" % fullSizeFn1;
        request+= "<fullSizeFn2>%s</fullSizeFn2>" % fullSizeFn2;
        
        request+="</register>";
        
        reply= self.customRequest( request );
        return registerID;
        
        
        
    def registerExternal( self, wordFn1, uploadID1, docID2= None, xl= None, xu= None, yl= None, yu= None ):
        
        request= "<register>";
        request+= "<wordFn1>%s</wordFn1>" % wordFn1;
        request+= API.getDocID( docID2, tgD= "docID2" );
        request+= API.getQueryRegion( xl, xu, yl, yu );
        
        registerID= str(uuid.uuid4());
        outFnPrefix= os.path.join( scriptroot, 'tmp' );
        outFn1= os.path.join(outFnPrefix, '%s_im1.jpg' % registerID);
        outFn2= os.path.join(outFnPrefix, '%s_im2.jpg' % registerID);
        outFn2t= os.path.join(outFnPrefix, '%s_im2t.jpg' % registerID);
        outFnBlend= os.path.join(outFnPrefix, '%s_blend.jpg' % registerID);
        outFnDiff= os.path.join(outFnPrefix, '%s_diff.jpg' % registerID);
        
        request+= "<outFn1>%s</outFn1>" % outFn1;
        request+= "<outFn2>%s</outFn2>" % outFn2;
        request+= "<outFn2t>%s</outFn2t>" % outFn2t;
        request+= "<outFnBlend>%s</outFnBlend>" % outFnBlend;
        request+= "<outFnDiff>%s</outFnDiff>" % outFnDiff;
        
        st= savedTemp.load(uploadID1);
        inFn1= st['localFilename_jpg'];
        fullSizeFn1= st['localFilename_full_jpg'];
        request+= "<inFn1>%s</inFn1>" % inFn1;
        request+= "<fullSizeFn1>%s</fullSizeFn1>" % fullSizeFn1;
        
        # original file names (e.g. in case where we have large images, ones like for ballads, index on resizes but want to be able to show originals)
        fullSizeFn2= self.pathManager_obj.getFullSize( self.pathManager_obj.docMap.getFn(int(docID2)) );
        
        request+= "<fullSizeFn2>%s</fullSizeFn2>" % fullSizeFn2;
        
        request+="</register>";
        
        reply= self.customRequest( request );
        return registerID;
    
    
    
    def dsetGetNumDocs(self):
        
        # at startup wait for API to launch
        firstWait= True;
        while not(self.running()):
            if firstWait:
                print "Waiting for the backend to launch (you should be doing it)";
                firstWait= False;
            time.sleep(1);
        
        request= "<dsetGetNumDocs>";
        request+="</dsetGetNumDocs>";
        reply= self.customRequest( request );
        return int(reply);
    
    
    
    def dsetGetFn(self, docID):
        request= "<dsetGetFn>";
        request+= API.getDocID(docID);
        request+="</dsetGetFn>";
        reply= self.customRequest( request );
        return reply.strip();
    
    
    
    def dsetGetDocID(self, fn):
        request= "<dsetGetDocID>";
        request+="<fn>%s</fn>" % fn;
        request+="</dsetGetDocID>";
        reply= self.customRequest( request );
        return int(reply);
    
    
    
    def containsFn(self, fn):
        request= "<containsFn><fn>%s</fn></containsFn>" % fn;
        reply= self.customRequest( request );
        return reply.strip()=='1';
    
    
    
    def dsetGetWidthHeight(self, docID):
        request= "<dsetGetWidthHeight>";
        request+= API.getDocID(docID);
        request+="</dsetGetWidthHeight>";
        reply= self.customRequest( request );
        imw, imh= reply.strip().split()
        return int(imw), int(imh);
    
    
    
    @staticmethod
    def getDocID( docID, tgD= "docID" ):
        return "<%s>%s</%s>" % (tgD,docID,tgD);
    
    @staticmethod
    def getQueryRegion( xl, xu, yl, yu, i='' ):
        request= "";
        if xl!=None: request+="<xl%s>%.2f</xl%s>" % (i,xl,i);
        if xu!=None: request+="<xu%s>%.2f</xu%s>" % (i,xu,i);
        if yl!=None: request+="<yl%s>%.2f</yl%s>" % (i,yl,i);
        if yu!=None: request+="<yu%s>%.2f</yu%s>" % (i,yu,i);
        return request;
        


class resultParser:
    
    def __init__(self):
        self.results=[];
        self.currRank= -1;
        self.inError= False;
    
    def startHandler(self, name, attrs):
        
        if self.inError:
            return;
        
        if name=="result":
            
            # get rank
            rank= int( attrs.get("rank") );
            if self.currRank<0:
                # first result
                self.currRank= rank;
            else:
                if rank!=self.currRank:
                    print "problem: results not sorted";
                    self.inError= True;
                    return;
            
            docID= int(attrs.get("docID"));
            score= float(attrs.get("score"));
            if "H" in attrs:
                H=attrs.get("H");
            else:
                H= None;
            self.results.append( (rank, docID,score,H) );
            
            self.currRank+=1;



class matchesParser:
    
    def __init__(self):
        self.matches=[];
    
    def startHandler(self, name, attrs):
        if name=="match":
            el1= attrs.get("el1");
            el2= attrs.get("el2");
            self.matches.append( ( \
                [ float(x) for x in el1.split(',') ], \
                [ float(x) for x in el2.split(',') ], \
            ) );


class documentMapUsingAPI:
    def __init__(self, API_obj):
        self.API_obj= API_obj;
        self.numDocs= self.API_obj.dsetGetNumDocs();
    def getFn(self, docID):
        return self.API_obj.dsetGetFn(docID);
    def containsFn(self, fn):
        return self.API_obj.containsFn(fn);
    def getDocID(self, fn):
        return self.API_obj.dsetGetDocID(fn);
    def getWidthHeight(self, docID):
        return self.API_obj.dsetGetWidthHeight(docID);
    def __len__(self):
        return self.numDocs;
