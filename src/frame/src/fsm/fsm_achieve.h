/*=============================================================================
#  Author:          dantezhu - http://www.vimer.cn
#  Email:           zny2008@gmail.com
#  FileName:        fsm.h
#  Description:     
#  Version:         1.0
#  LastChange:      2011-01-19 12:13:02
#  History:         
=============================================================================*/
#ifndef _FSM_H_
#define _FSM_H_
#include <iostream>
#include <memory>
#include <string>
#include <vector>
#include <set>
#include <map>
#include <list>

#include "fsm_interface.h"
using namespace std;
class CFrameBase : public IFrame
{
public:
    CFrameBase () {
        m_needGCCount = 0;
        m_allActorCount = 0;
    }
    virtual ~CFrameBase () {}

    int AddActor(IActor* pActor)
    {
        m_listActors.push_front(pActor);
        m_allActorCount++;
        return 0;
    }
    int DelActor(IActor* pActor)
    {
        m_listActors.remove(pActor);
        m_allActorCount--;
        return 0;
    }
    int GetActorCount()
    {
        return m_allActorCount;
    }
    int AddNeedGCCount()
    {
        m_needGCCount++;
        return 0;
    }
    int SubNeedGCCount()
    {
        m_needGCCount--;
        if (m_needGCCount < 0)
        {
            m_needGCCount = 0;
        }
        return 0;
    }
    int GetNeedGCCount()
    {
        return m_needGCCount;
    }
    void GCActors()
    {
        for(list<IActor*>::iterator it = m_listActors.begin(); it != m_listActors.end(); ++it)
        {
            list<IActor*>::iterator tempIt = it;
            it++;

            if ((*tempIt)->GetGCMark())
            {
                delete (*tempIt);
                m_listActors.erase(tempIt);
                m_allActorCount--;
            }
        }
    }
    map<int, IFsm*>* GetFsmMgr()
    {
        return &m_mapFsmMgr;
    }
    int RegFsm(int state, IFsm* fsm)
    {
        if (state <=0 || fsm == NULL)
        {
            return -1;
        }
        fsm->AttachFrame(this);
        m_mapFsmMgr[state] = fsm;
        return 0;
    }

protected:
    list<IActor*> m_listActors;

    int m_needGCCount;
    int m_allActorCount;
    map<int, IFsm*> m_mapFsmMgr;
};

class CActorBase : public IActor
{
public:
    CActorBase () {
        m_bGC = false;
        m_Fsm = NULL;
        m_ptrMapFsmMgr = NULL;
        m_pFrame = NULL;
    }

    virtual ~CActorBase () {
        if (m_pFrame)
        {
            m_pFrame->SubNeedGCCount();
        }
    }

    virtual void SetGCMark()
    {
        m_bGC = true;
        if (m_pFrame)
        {
            m_pFrame->AddNeedGCCount();
        }
    }
    virtual bool GetGCMark()
    {
        return m_bGC;
    }
    int AttachFrame(IFrame* pFrame)
    {
        if (pFrame == NULL)
        {
            return -1;
        }
        m_pFrame = pFrame;
        m_pFrame->AddActor(this);
        AttachFsmMgr(m_pFrame->GetFsmMgr());
        return 0;
    }
    IFrame* GetFrame()
    {
        return m_pFrame;
    }

    int AttachFsmMgr(map<int, IFsm*> * ptrMapFsmMgr)
    {
        m_ptrMapFsmMgr = ptrMapFsmMgr;
        return 0;
    }
    map<int, IFsm*>* GetFsmMgr()
    {
        return m_ptrMapFsmMgr;
    }

    int ProcessState()
    {
        if (m_ptrMapFsmMgr == NULL)
        {
            return -1;
        }
        int state = doChangeFsm(m_Fsm);
        return ChangeState(state);
    }

    int ChangeState(int destState)
    {
        m_vecStates.push_back(destState);
        if (m_ptrMapFsmMgr == NULL)
        {
            return -1;
        }

        if (0 == destState)
        {
            //此次处理结束,但是并没有退出这个状态
            return 0;
        }
        else if (destState < 0)
        {
            //需要关闭整个请求
            if (m_Fsm)
            {
                m_Fsm->Exit(this);
                m_Fsm = NULL;
            }
            return destState;
        }
        IFsm * destFsm = NULL;
        destFsm = (*m_ptrMapFsmMgr)[destState];
        int state = doChangeFsm(destFsm);
        return ChangeState(state);
    }
    int HandleEvent(IEvent* pEvent)
    {
        //默认是什么也不做的，继承的类如果需要用到，就要重写
        return 0;
    }
private:
    int doChangeFsm(IFsm* destFsm)
    {
        if (destFsm == NULL)
        {
            return 0;
        }

        if (m_Fsm != destFsm)
        {
            if (m_Fsm != NULL)
            {
                m_Fsm->Exit(this);
            }
            m_Fsm = destFsm;
            m_Fsm->Entry(this);
        }
        return m_Fsm->Process(this);
    }


protected:
    bool m_bGC;
    IFsm* m_Fsm;
    map<int, IFsm*> *m_ptrMapFsmMgr;
    IFrame* m_pFrame;
    vector<int> m_vecStates;//历史的states
};
class CFsmBase : public IFsm
{
public:
    CFsmBase () {
        m_pFrame = NULL;
    }
    virtual ~CFsmBase () {}
    virtual int AttachFrame(IFrame* pFrame)
    {
        m_pFrame = pFrame;
        return 0;
    }

private:
    IFrame* m_pFrame;
};
#endif
