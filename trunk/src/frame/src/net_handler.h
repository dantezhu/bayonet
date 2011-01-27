/*=============================================================================
#  Author:          dantezhu - http://www.vimer.cn
#  Email:           zny2008@gmail.com
#  FileName:        net_handler.h
#  Description:     socket的handler
#  Version:         1.0
#  LastChange:      2011-01-27 11:49:52
#  History:         
=============================================================================*/
#ifndef _NET_HANDLER_H_
#define _NET_HANDLER_H_
#include <iostream>
#include <memory>
#include <string>
#include <vector>
#include <set>
#include <map>

#include "comm_def.h"
#include "fl_log.h"
using namespace std;
class CNetHandlerBase
{
public:
    CNetHandlerBase () {
        m_Port = 0;
        m_SocketFd = -1;
        m_ClientPort = 0;
    }
    virtual ~CNetHandlerBase ();

    int Init(string ip,int port);
    int Init(int socketFd);

    int GetSocketFd();

    string GetClientIp();

    int GetClientPort();

    int SetNoBlock(int socketFd);
    
    int Close();

    virtual int Create()=0;
    virtual int Send(char* pBuf,int bufLen)=0;
    virtual int Recv(char* pBuf,int bufSize)=0;

protected:
    string m_IP;
    int m_Port;

    int m_SocketFd;

    string m_ClientIp;
    int m_ClientPort;
};
//=============================================================================
class CNetHandlerTcp : public CNetHandlerBase
{
public:
    CNetHandlerTcp () {}
    virtual ~CNetHandlerTcp () {}
    virtual int Create();
    virtual int Send(char* pBuf,int bufLen);
    virtual int Recv(char* pBuf,int bufSize);

protected:
    int netConnect();
};
class CNetHandlerUdp : public CNetHandlerBase
{
public:
    CNetHandlerUdp () {}
    virtual ~CNetHandlerUdp () {}
    virtual int Create();
    virtual int Send(char* pBuf,int bufLen);
    virtual int Recv(char* pBuf,int bufSize);
};
//=============================================================================
#endif
