/*=============================================================================
#  Author:          dantezhu - http://www.vimer.cn
#  Email:           zny2008@gmail.com
#  FileName:        socketactor_passive.cpp
#  Description:     
#  Version:         1.0
#  LastChange:      2011-01-28 17:08:34
#  History:         
=============================================================================*/
#include "socketactor_passive.h"
#include "appactor_base.h"

int CSocketActorPassive::OnFiniOver()
{
    if (m_pAppActorProxy.true_ptr())
    {
        CAppActorBase* pAppActor = (CAppActorBase*)m_pAppActorProxy.true_ptr();
        pAppActor->ChangeState(APP_FSM_FINI);
        pAppActor->DetachCommu();
    }

    return SOCKET_FSM_ALLOVER;
}
int CSocketActorPassive::OnSendOver()
{
    NotifyAppActor();//通知上层状态机可以FINI了
    Clear();
    if (m_bKeepcnt && m_ProtoType == PROTO_TYPE_TCP)
    {
        return SOCKET_FSM_WAITRECV;
    }
    else
    {
        return SOCKET_FSM_CLOSING;
    }
}
void CSocketActorPassive::NotifyAppActor()
{
    if (m_pAppActorProxy.true_ptr())
    {
        m_pAppActorProxy.true_ptr()->ChangeState(APP_FSM_FINI);
    }
    else
    {
        error_log("[class:%s]m_pAppActor is NULL",Name().c_str());
    }
}
int CSocketActorPassive::OnCloseOver()
{
    NotifyAppActor();//通知上层状态机可以FINI了
    return CSocketActorData::OnCloseOver();
}
