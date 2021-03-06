/*=============================================================================
#  Author:          dantezhu - http://www.vimer.cn
#  Email:           zny2008@gmail.com
#  FileName:        socketactor_listen_udp.cpp
#  Description:     
#  Version:         1.0
#  LastChange:      2011-01-28 00:48:05
#  History:         
=============================================================================*/
#include "socketactor_listen_udp.h"
#include "appactor_base.h"
namespace bayonet {
CSocketActorListenUdp::CSocketActorListenUdp()
{
    m_attachedSocketMaxSize = ATTACHED_SOCKET_MAXSIZE;
}
CSocketActorListenUdp::~CSocketActorListenUdp() 
{}

int CSocketActorListenUdp::Init(string ip,int port,int timeout_ms,int protoType)
{
    int ret = CSocketActorData::Init(ip, port, timeout_ms, protoType);
    if (ret != 0)
    {
        return ret;
    }

    int optval;
    m_SocketFd = socket(AF_INET,SOCK_DGRAM,0);
    if(m_SocketFd < 0)
    {   
        byt_error_log("[class:%s]Create socket error:%s\n",Name().c_str(),strerror(errno));
        return -1; 
    }   
    struct sockaddr_in myaddr;
    myaddr.sin_family = AF_INET;
    myaddr.sin_port = htons(m_Port);
    myaddr.sin_addr.s_addr = inet_addr(m_IP.c_str());

    optval = 1;
    setsockopt(m_SocketFd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));
    if(bind(m_SocketFd,(struct sockaddr*)&myaddr,sizeof(struct sockaddr)) < 0)
    {   
        //close(listen_fd);
        //到CLOSING状态会帮你关闭掉
        byt_error_log("[class:%s]CreateListen bind ip:%s port:%d sock:%d err:%s\n",
                  Name().c_str(),m_IP.c_str(),m_Port,m_SocketFd,strerror(errno));
        return -2; 
    }
    ret = m_pNetHandler->Init(m_SocketFd);
    if (ret)
    {
        byt_error_log("[class:%s]nethandler init error:%d",Name().c_str(),ret);
        return -3; 
    }
    return 0;
}

void CSocketActorListenUdp::SetAttachedSocketMaxSize(int attachedSocketMaxSize)
{
    m_attachedSocketMaxSize = attachedSocketMaxSize;
}

int CSocketActorListenUdp::GetAttachedSocketMaxSize()
{
    return m_attachedSocketMaxSize;
}

int CSocketActorListenUdp::OnInitOver()
{
    return SOCKET_FSM_WAITRECV;
}

int CSocketActorListenUdp::OnWaitSend()
{
    CSocketActorPassiveUdp* pSocketActorAccept = CreatePassiveActor();
    if (pSocketActorAccept)
    {
        pSocketActorAccept->ChangeState(SOCKET_FSM_WAITSEND);
    }
    //清空标志，要不然会有内存泄漏
    ResetStatusData();
    return SOCKET_FSM_WAITRECV;
}

int CSocketActorListenUdp::OnSendOver()
{
    return SOCKET_FSM_WAITRECV;
}

int CSocketActorListenUdp::OnWaitRecv()
{
    ClearFsmNodes();
    return CSocketActorData::OnWaitRecv();
}

int CSocketActorListenUdp::OnRecv()
{
    CEPoller* pEpoller = GetEpoller();
    if (pEpoller)
    {
        if (pEpoller->GetAttachedSocketCount() > m_attachedSocketMaxSize)
        {
            byt_error_log("attachedSocketCount has reach the max:%d/%d",
                      pEpoller->GetAttachedSocketCount(),m_attachedSocketMaxSize);
            return SOCKET_FSM_WAITRECV;
        }
    }

    return CSocketActorData::OnRecv();
}

int CSocketActorListenUdp::OnRecvOver()
{
    CreatePassiveActor();
    //清空标志，要不然会有内存泄漏
    ResetStatusData();
    return SOCKET_FSM_WAITRECV;
}

bool CSocketActorListenUdp::IsTimeOut()
{
    //永不超时
    return false;
}

CSocketActorPassiveUdp* CSocketActorListenUdp::CreatePassiveActor()
{
    if (m_pNetHandler->GetClientIp().size()>0 && m_pNetHandler->GetClientPort()>0)
    {
        CSocketActorPassiveUdp * pSocketActorAccept = new CSocketActorPassiveUdp();
        pSocketActorAccept->AttachFrame(m_pFrame);
        pSocketActorAccept->SetIActionPtr(m_pAction);
        if (m_pAppActorProxy.true_ptr())
        {
            CAppActorBase* pAppActor = (CAppActorBase*)m_pAppActorProxy.true_ptr();
            pAppActor->AttachCommu(pSocketActorAccept);
        }
        //重新指向一个新的
        m_pAppActorProxy = NULL;

        byt_trace_log("%s,%d",m_pNetHandler->GetClientIp().c_str(),m_pNetHandler->GetClientPort());
        pSocketActorAccept->Init(m_pNetHandler->GetClientIp(),m_pNetHandler->GetClientPort(),m_TimeoutMs,m_ProtoType);
        pSocketActorAccept->ChangeState(SOCKET_FSM_INIT);
        return pSocketActorAccept;
    }
    return NULL;
}
}
