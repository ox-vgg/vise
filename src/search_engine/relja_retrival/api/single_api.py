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

import multiprocessing;

import engine_3relja_ext.processing.feat_standard as feat_standard;
import engine_3relja_ext.processing.feature_wrappers as feature_wrappers;
import engine_3relja_ext.util.load_clusters;
import engine_3relja_ext.vlad.vlad;
import engine_3relja_ext.vlad.multivoc_vlad;



class singleAPI:
    
    
    
    # don't forget to set .pathManager_obj
    def __init__(self, APIport, APIhost= "localhost", scoreThr= None, verbose= False, clstFn= None, featureWrapper= feature_wrappers.featureNoWrapper, SIFTscale3= True ):
        
        self.APIport= APIport;
        self.APIhost= APIhost;
        self.scoreThr= scoreThr;
        self.verbose= verbose;
        self.docMap= documentMapUsingAPI(self);
        
        self.enableUpload= (clstFn!=None);
        
        if self.enableUpload:
            
            # TODO make this configurable
            
            assert( SIFTscale3 );
            self.detdesc_obj= feat_standard.filterROI(feat_standard.getDetDesc('hesaff', 'sift_scale3'));
            self.featureWrapper= featureWrapper if featureWrapper!=None else feature_wrappers.featureNoWrapper;
            
            # the same rotation as on the dataset size
            self.vladRotation= feature_wrappers.rotation( os.path.expanduser('~/Relja/Data/BBC/BBCc/rotation_BBCc_test_hesaff_rootsift_k256_a0_norm3_dim256.bin') ) if True else feature_wrappers.featureNoWrapper;
            
            numVoc= 4;
            
            mvC_objs= [];
            for iVoc in range(0, numVoc):
                
                clusters= engine_3relja_ext.util.load_clusters.loadFlatClusters( '~/Relja/Data/BBC/BBCc/clst_BBCc_train_hesaff_rootsift_scale3_256_%d.bin' % (43+iVoc) );
                vC_obj= engine_3relja_ext.vlad.vlad.vladComputer(clusters, adaptCentroids= None, normMethod= 'innorm');
                mvC_objs.append( vC_obj );
            
            projAndMuFn= os.path.expanduser('~/Relja/Data/BBC/BBCc/proj_BBCc_train_BBCc_train_k256_rs1_a0_norm3.bin');
            self.mvocC_obj= engine_3relja_ext.vlad.multivoc_vlad.multivocComputer(mvC_objs, projAndMuFn, shortDim= 256);
    
    
    
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
        return False;
    
    
    
    def internalQuery( self, docID, xl= None, xu= None, yl= None, yu= None, startFrom= 0, numberToReturn= 20 ):
        
        request= "<internalQuery>";
        
        request+= "<docID>%d</docID>" % docID;
        request+= singleAPI.getQueryRegion( xl, xu, yl, yu );
        request+= "<startFrom>%d</startFrom><numberToReturn>%d</numberToReturn>\n" % (startFrom, numberToReturn);
        request+= "</internalQuery>";
        
        reply= self.customRequest( request );
        
        results= self.getResults(reply);
        return results;
    
    
    
    def externalQuery( self, featFn, xl= None, xu= None, yl= None, yu= None, startFrom= 0, numberToReturn= 20 ):
        
        request= "<externalQuery>";
        request+= "<featFn>%s</featFn>" % featFn;
        request+= "<startFrom>%d</startFrom><numberToReturn>%d</numberToReturn>\n" % (startFrom, numberToReturn);
        request+= singleAPI.getQueryRegion( xl, xu, yl, yu );
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
                request+= "<featFn%d>%s</featFn%d>" % (i,querySpecs[i],i);
        if rois!=None:
            for i in range(0, len(rois)):
                roi= rois[i];
                request+= singleAPI.getQueryRegion( roi[0], roi[1], roi[2], roi[3], '%d'%i );
        request+="</multiQuery>";
        
        reply= self.customRequest( request );
        
        results= self.getResults(reply);
        return results;
    
    
    
    def processImage( self, imageFn, featFn, ROI= (None,None,None,None) ):
        
        regs, descs= self.detdesc_obj.execute( imageFn, ROI= ROI );
        descs= self.featureWrapper( descs );
        
        vlads= self.mvocC_obj.compute( descs, points= regs );
        vlads= self.vladRotation( vlads.reshape(vlads.shape[0],1).T );
        
        vlads.tofile( os.path.expanduser(featFn) );
    
    
    
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
        request+= singleAPI.getDocID(docID);
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
        request+= singleAPI.getDocID(docID);
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
