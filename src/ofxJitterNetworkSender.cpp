#include "ofxJitterNetworkSender.h"


//------------------------------------------------------------------------------
ofxJitterNetworkSender::ofxJitterNetworkSender()
{
}

//------------------------------------------------------------------------------
ofxJitterNetworkSender::~ofxJitterNetworkSender()
{
}

//------------------------------------------------------------------------------
void ofxJitterNetworkSender::sendFrame(const ofPixelsRef pixels)
{
    int planecount = pixels.getNumChannels();
    int dimcount = 2; // only sending 2d matrices from of
    int dim[dimcount];
    dim[0]       = pixels.getWidth();
    dim[1]       = pixels.getHeight();
    int typeSize = pixels.getBytesPerChannel();
    int type     = JIT_MATRIX_TYPE_CHAR;

    makeMatrixHeader(planecount, typeSize, type, dim, dimcount);

    char *matrix = (char*)pixels.getPixels();
    
    //////SEND ONE MATRIX
    sendRawBytes((char *)(&m_chunkHeader), sizeof(t_jit_net_packet_header));
    sendRawBytes((char *)(&m_matrixHeader), sizeof(t_jit_net_packet_matrix));
    
    int packSize = SWAP32(m_matrixHeader.dimstride[SWAP32(m_matrixHeader.dimcount)-1])
                * SWAP32(m_matrixHeader.dim[SWAP32(m_matrixHeader.dimcount)-1]);

    sendRawBytes(matrix, packSize);

}

//------------------------------------------------------------------------------
void ofxJitterNetworkSender::sendText(const string& txt) {
    m_messageHeader.id = SWAP32(JIT_MESSAGE_PACKET_ID);
    m_messageHeader.size = SWAP32(sizeof(long) + // size
                                  sizeof(long) + // ac
                                  sizeof(char) + // type
                                  sizeof(char)*txt.length() + // number
                                  sizeof(char)); // null terminator
    
    sendRawBytes((char *)&m_messageHeader.id, sizeof(long));
    sendRawBytes((char *)&m_messageHeader.size, sizeof(long));
    
    // the packet
    long messageSizeBytes = m_messageHeader.size; //	32-bit integer that contains the size of the serialized message in bytes. 
    long ac = SWAP32(0);      //    Following that another 32-bit integer gives the argument count for the atoms. 
    /// Following that comes the message atoms themselves, starting with the leading symbol if it exists. 
    //  Each atom is represented in memory first with a char that indicates what type of atom it is:
    //		's' for symbol, 'l' for long, and 'f' for float. 
    //		For long and float atoms, the next 4 bytes contain the value of the atom; 
    //		for symbol atoms a null terminated character string follows. 
    
    
    char atomType = 's'; //'s' for symbol, 'l' for long, and 'f' for float. 
    const char *cp = txt.c_str(); // seriously
    char nullTerm = '\0';
    sendRawBytes((char *)&messageSizeBytes, sizeof(long));
    sendRawBytes((char *)&ac, sizeof(long));
    sendRawBytes((char *)&atomType, sizeof(char));
    sendRawBytes((char *)cp, txt.length()*sizeof(char));
    sendRawBytes((char *)&nullTerm, sizeof(char));
    
    //readResponse();
}

//------------------------------------------------------------------------------
double ofxJitterNetworkSender::getLastSent() const
{
    return lastSent;
}

//------------------------------------------------------------------------------
void ofxJitterNetworkSender::readResponse()
{
    // TODO read latency data here.
    char buf[MAXDATASIZE]; 
    int numBytes = receiveRawBytes(buf, MAXDATASIZE-1);
    if (numBytes == -1)
    {
        // printf("recv error\n");
        // skip it, there's nothing there
    }
    else
    {
        buf[numBytes] = '\0'; // end it
        
        m_latencyPacket.id                      = ((t_jit_net_packet_latency *)buf)->id; // cast it to get the id
        m_latencyPacket.client_time_original    = ((t_jit_net_packet_latency *)buf)->client_time_original;
        m_latencyPacket.server_time_before_data = ((t_jit_net_packet_latency *)buf)->server_time_before_data;
        m_latencyPacket.server_time_after_data  = ((t_jit_net_packet_latency *)buf)->server_time_after_data;
        
//printf("id: %d\n", (m_latencyPacket.id));
//printf("client time original %f\n",m_latencyPacket.client_time_original);
//printf("before Data %fl\n",m_latencyPacket.server_time_before_data);
//printf("after Data %f\n",m_latencyPacket.server_time_after_data);
//printf("diff=%f\n\n",m_latencyPacket.server_time_after_data - m_latencyPacket.server_time_before_data);
        
        // cout << buf << endl;
        
        // if(lastSent >= m_latencyPacket.client_time_original) {
        //  printf("GTOE => last sent=%f and client_time_original=%f\n",lastSent,m_latencyPacket.client_time_original);	
        // } else {
        //  printf("NNNWWW => last sent=%f and client_time_original=%f\n",lastSent,m_latencyPacket.client_time_original);	
        // }
    }
}

//------------------------------------------------------------------------------
void ofxJitterNetworkSender::makeMatrixHeader(int planecount,
                                              int typeSize,
                                              int type,
                                              int *dim,
                                              int dimcount)
{
    long i, j, k;
    
    m_chunkHeader.id = SWAP32(JIT_MATRIX_PACKET_ID);
    m_chunkHeader.size = sizeof(t_jit_net_packet_matrix);
    
    m_matrixHeader.id = JIT_MATRIX_PACKET_ID;
    m_matrixHeader.size = SWAP32(sizeof(t_jit_net_packet_matrix));
    m_matrixHeader.planecount = SWAP32(planecount);
    m_matrixHeader.type = SWAP32(type);
    m_matrixHeader.dimcount = SWAP32(dimcount);
    
    for(i=0; i < dimcount; i++)
    {
        m_matrixHeader.dim[i] = SWAP32(dim[i]);
    }
    
    while(i < JIT_MATRIX_MAX_DIMCOUNT)
    {
        m_matrixHeader.dim[i] = SWAP32(0); // <-- in the jitter one, they seem to just copy the dim through ...
        i++;
    }
    
    //special case for first value
    m_matrixHeader.dimstride[0] = SWAP32(typeSize * planecount);
    
    for(i=1; i <= dimcount; i++)
    {
        m_matrixHeader.dimstride[i] = SWAP32(dim[i-1]*SWAP32(m_matrixHeader.dimstride[i-1])); // watch out for these .. they need to come back ...
    }
    
    while(i < JIT_MATRIX_MAX_DIMCOUNT)
    {
        m_matrixHeader.dimstride[i] = SWAP32(0);
        i++;
    }
    
    m_matrixHeader.datasize = SWAP32(SWAP32(m_matrixHeader.dimstride[dimcount-1])*SWAP32(m_matrixHeader.dim[dimcount-1]));
    m_matrixHeader.time = ofGetElapsedTimef();

    // just to keep track
    lastSent = m_matrixHeader.time;

}
