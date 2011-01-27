/*=============================================================================
#  Author:          dantezhu - http://www.vimer.cn
#  Email:           zny2008@gmail.com
#  FileName:        socketactor_base.cpp
#  Description:     
#  Version:         1.0
#  LastChange:      2011-01-24 20:34:16
#  History:         
=============================================================================*/
#include "socketactor_base.h"
#include "bayonet_frame.h"

int CSocketActorData::Init(string ip,int port,int timeout_ms,int protoType)
{
    int ret = CSocketActorBase::Init(ip,port,timeout_ms,protoType);
    if (ret)
    {
        return -1;
    }
    switch(m_ProtoType)
    {
        case PROTO_TYPE_TCP:
            m_pNetHandler = new CNetHandlerTcp();
            break;
        case PROTO_TYPE_UDP:
            m_pNetHandler = new CNetHandlerUdp();
            break;
        default:
            return -2;
    }
    ret = m_pNetHandler->Init(m_IP,m_Port);
    if (ret)
    {
        return -3;
    }
    return 0;
}

int CSocketActorData::Init(int socketFd,int timeout_ms,int protoType)
{
    trace_log("");
    int ret = CSocketActorBase::Init(socketFd,timeout_ms,protoType);
    if (ret)
    {
        return -1;
    }
    switch(m_ProtoType)
    {
        case PROTO_TYPE_TCP:
            m_pNetHandler = new CNetHandlerTcp();
            break;
        case PROTO_TYPE_UDP:
            m_pNetHandler = new CNetHandlerUdp();
            break;
        default:
            return -2;
    }
    ret = m_pNetHandler->Init(m_SocketFd);
    if (ret)
    {
        return -3;
    }
    trace_log("");
    return 0;
}
int CSocketActorData::OnInit()
{
    trace_log("");
    int ret = m_pNetHandler->Create();
    if (ret)
    {
        trace_log("");
        return SOCKET_FSM_CLOSING;
    }
    else
    {
        trace_log("");
        m_SocketFd = m_pNetHandler->GetSocketFd();
    }
    CEPoller* pEpoller = GetEpoller();
    if (!pEpoller)
    {
        error_log("pEpoller is NULL");
        return SOCKET_FSM_CLOSING;
    }
    pEpoller->AttachSocket(this);//加入到epoll中
    trace_log("");
    return OnInitOver();
}
int CSocketActorData::OnFini()
{
    return OnFiniOver();
}
int CSocketActorData::OnRecv()
{
    int ret = 0;
    if (m_recvFlag == 0)
    {
        m_recvFlag = 1;
    }
    for(;;)
    {
        ret = m_pNetHandler->Recv((char*)m_strSingleRecvBuf.c_str(),m_strSingleRecvBuf.size());
        if (ret == 0)
        {
            return SOCKET_FSM_CLOSING;
        }
        else if (ret < 0)
        {
            if ( errno == EINTR || errno == EAGAIN )
            {
                return SOCKET_FSM_WAITRECV;
            }
            return SOCKET_FSM_CLOSING;
        }
        else
        {
            while ((ret + m_recvedLen) > m_strRecvBuf.size())
            {
                m_strRecvBuf.resize(m_recvedLen*2 + ret);
            }
            memcpy((char*)(m_strRecvBuf.c_str()+m_recvedLen),m_strSingleRecvBuf.c_str(),ret);
            m_recvedLen += ret;
        }
        ret = HandleInput(m_strRecvBuf.c_str(), m_recvedLen);
        if (ret == 0)
        {
            continue;
        }
        else if (ret < 0)
        {
            return SOCKET_FSM_CLOSING;
        }
        else
        {
            //数据完整了
            m_strRecvBuf.resize(ret);
            m_recvedLen = ret;
            break;
        }
    }
    ret = HandleDecodeRecvBuf(m_strRecvBuf.c_str(), m_recvedLen);
    if (ret)
    {
        return SOCKET_FSM_CLOSING;
    }
    return OnRecvOver();
}
int CSocketActorData::OnSend()
{
    trace_log("");
    int ret = 0;
    if (m_sendFlag == 0)
    {
        m_sendFlag = 1;
        ret = HandleEncodeSendBuf(m_strSendBuf,m_sendBufLen);
        if (ret)
        {
            error_log("HandleEncodeSendBuf error :%d",ret);
            return SOCKET_FSM_CLOSING;
        }
        if (m_strSendBuf.size() < m_sendBufLen)
        {
            error_log("HandleEncodeSendBuf len error:%d/%d",m_strSendBuf.size(),m_sendBufLen);
            return SOCKET_FSM_CLOSING;
        }
    }
    trace_log("");
    while (m_sendedLen != m_sendBufLen)
    {
        trace_log("");
        ret = m_pNetHandler->Send((char*)(m_strSendBuf.c_str()+m_sendedLen),m_sendBufLen-m_sendedLen);
        trace_log("%s",m_strSendBuf.c_str());
        if ( ret == 0 )
        {
            trace_log("");
            return SOCKET_FSM_WAITSEND;
        }
        else if ( ret < 0 )
        {
            trace_log("");
            if ( errno == EINTR || errno == EAGAIN )
            {
                return SOCKET_FSM_WAITSEND;
            }
            return SOCKET_FSM_CLOSING;
        }
        else
        {
            trace_log("");
            m_sendedLen += ret;
        }
    }
    trace_log("");
    return OnSendOver();
}
int CSocketActorData::OnClose()
{
    if (m_pNetHandler)
    {
        m_pNetHandler->Close();
        m_SocketFd = m_pNetHandler->GetSocketFd();
    }
    return OnCloseOver();
}
int CSocketActorData::OnFiniOver()
{
    return SOCKET_FSM_ALLOVER;
}
int CSocketActorData::Clear()
{
    m_sendedLen = 0;
    m_sendBufLen = 0;

    m_recvedLen = 0;

    m_sendFlag = 0;
    m_recvFlag = 0;
}
int CSocketActorData::HandleEncodeSendBuf(
        string & strSendBuf,
        int &len)
{
    if (m_pAction == NULL)
    {
        return -1;
    }
    return m_pAction->HandleEncodeSendBuf(strSendBuf,len);
}
int CSocketActorData::HandleInput(
        const char *buf,
        int len)
{
    if (m_pAction == NULL)
    {
        return -1;
    }
    return m_pAction->HandleInput(buf,len);
}
int CSocketActorData::HandleDecodeRecvBuf(const char *buf, int len)
{
    if (m_pAction == NULL)
    {
        return -1;
    }
    return m_pAction->HandleDecodeRecvBuf(buf,len);
}
//=============================================================================
