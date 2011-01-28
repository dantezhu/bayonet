/*=============================================================================
#  Author:          dantezhu - http://www.vimer.cn
#  Email:           zny2008@gmail.com
#  FileName:        socketactor_set.h
#  Description:    
#  Version:         1.0
#  LastChange:      2011-01-25 20:16:46
#  History:         
=============================================================================*/
#ifndef _SOCKETACTOR_SET_H_
#define _SOCKETACTOR_SET_H_

#include <iostream>
#include <string>
#include <vector>
#include <set>
#include <map>

#include "fsm_achieve.h"
#include "action_info.h"

class CSocketActorSet
{
public:
    CSocketActorSet() {
        m_pActor = NULL;
    }
    virtual ~CSocketActorSet() {}
    virtual void AttachActor(IActor* pActor)
    {
        m_pActor = pActor;
    }
    virtual IActor* GetActor()
    {
        return m_pActor;
    }
    /**
     * @brief   添加一个pActionInfo
     *
     * @param   pActionInfo
     *
     * @return  0
     */
    virtual int Add(CActionInfo* pActionInfo)
    {
        m_setActionInfos.insert(pActionInfo);
        return 0;
    }

    /**
     * @brief   删除一个pActionInfo
     *
     * @param   pActionInfo
     *
     * @return  0
     */
    virtual int Del(CActionInfo* pActionInfo)
    {
        m_setActionInfos.erase(pActionInfo);
        return 0;
    }
private:
    set<CActionInfo*> m_setActionInfos;
    IActor * m_pActor;
};
#endif
