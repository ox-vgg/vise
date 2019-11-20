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

import os, sys;
import ConfigParser;

import pypar as mpi;
from dkmeans_relja.dkmeans import compute_clusters;


def getOptional( f, defaultValue ):

    try:
        value= f();
    except ConfigParser.NoOptionError:
        value= defaultValue;
    return value;



if __name__=='__main__':

    dsetname= "oxMini20_v2";
    if len(sys.argv)>1: dsetname= sys.argv[1];
    configFn= "../src/ui/web/config/config.cfg";
    if len(sys.argv)>2: configFn= sys.argv[2];

    config= ConfigParser.ConfigParser();
    config.read( configFn );

    RootSIFT= getOptional( lambda: config.getboolean(dsetname, 'RootSIFT'), True );

    data_dir = config.get(dsetname, 'data_dir');
    pntsFn   = os.path.join(data_dir, config.get(dsetname, 'descsFn') );
    clstFn   = os.path.join(data_dir, config.get(dsetname, 'clstFn') );
    vocSize  = getOptional( lambda: config.getint(dsetname, 'vocSize'), 100 );
    num_iter = getOptional( lambda: config.getint(dsetname, 'cluster_num_iteration'), 10 );
    seed     = 43;

    compute_clusters(clstFn, pntsFn, vocSize,
                     num_iter, approx=True, seed= seed,
                     featureWrapper= ("hell" if RootSIFT else None) );

    mpi.finalize();
