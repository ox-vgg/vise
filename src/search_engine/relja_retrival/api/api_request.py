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

from socket import *;



def customRequest( APIhost, APIport, request, appendEnd= True ):
    
    sock= socket(AF_INET, SOCK_STREAM);
    try:
        sock.connect((APIhost, APIport));
    except error, msg:
        print 'Connect failed', msg;
        return '';
    
    if appendEnd:
        request+= " $END$";
    
    sock.send( request );
    
    reply= "";
    while 1:
        data = sock.recv(1024);
        if not data:
            break;
        reply += data;
    
    # Close socket
    sock.close();
    
    return reply;
