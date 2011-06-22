#include <iostream>
#include <memory>
#include <string>
#include <vector>
#include <set>
#include <map>
using namespace std;
#include "bayonet_frame.h"

#define APP_FSM_LOGIC1 2000

class CAppFsmLogic1;
class CActionFirst : public IAction
{
public:
    int HandleEncodeSendBuf(
            CSocketActorData* pSocketActor,
            CAppActorBase* pAppActor,
            string & strSendBuf,
            int &len)
    {
        trace_log("send");
        strSendBuf="woainizhende111111";
        len = strSendBuf.size();
        return 0;
    }

    int HandleInput(
            CSocketActorData* pSocketActor,
            CAppActorBase* pAppActor,
            const char *buf,
            int len)
    {
        return len;
    }

    int HandleDecodeRecvBuf(
            CSocketActorData* pSocketActor,
            CAppActorBase* pAppActor,
            const char *buf, 
            int len)
    {
        CAppActorBase * app_actor = new CAppActorBase();
        app_actor->AttachFrame(pSocketActor->GetFrame());
        app_actor->AttachCommu(pSocketActor);
        app_actor->ChangeState(APP_FSM_LOGIC1);
        trace_log("listen tcp HandleDecodeRecvBuf");
        return 0;
    }
};

class CAppFsmLogic1 : public CAppFsmBase
{
public:
    virtual ~CAppFsmLogic1 () {}
    virtual int HandleEntry(CActionInfoSet *pActionInfoSet, CAppActorBase* pAppActor)
    {
        static CActionFirst actionFirst;
        StActionInfoParam param;
        param.id = 1;
        param.ip = "127.0.0.1";
        param.port = 100;
        param.protoType = PROTO_TYPE_UDP;
        param.pAction = &actionFirst;
        param.actionType = ACTIONTYPE_SENDONLY;
        param.timeout_ms = 1000;

        /*CActionInfo * pActionInfo = new CActionInfo();
        pActionInfo->Init(param);
        pActionInfo->SetAppActor(pAppActor);
        pActionInfoSet->Add(pActionInfo);*/
        return 0;
    }
    virtual int HandleProcess(CActionInfoSet *pActionInfoSet, CAppActorBase* pAppActor)
    {
        trace_log("HandleProcess");
        /*set<CActionInfo*> &setAction = pActionInfoSet->GetActionSet();
        for(set<CActionInfo*>::iterator it = setAction.begin(); it != setAction.end(); ++it)
        {
            trace_log("error no:%d",(*it)->GetErrno());
        }*/
        return APP_FSM_RSP;//代表要回复客户端啦
    }
    virtual int HandleExit(CActionInfoSet *pActionInfoSet, CAppActorBase* pAppActor)
    {
        return 0;
    }
};
int main(int argc, const char *argv[])
{
    CBayonetFrame srv;
    StFrameParam param;
    param.infoDir="bayonet";
    param.ip="0.0.0.0";
    param.port = 20000;
    //param.bKeepcnt= true;
    param.protoType = PROTO_TYPE_UDP;
    //param.protoType = PROTO_TYPE_TCP;
    param.pAction = new CActionFirst();
    param.gcMaxCount=10000;
    param.workerNum=2;

    int ret = srv.Init(param);
    if (ret != 0)
    {
        return -1;
    }
    srv.RegFsm(APP_FSM_LOGIC1,new CAppFsmLogic1());
    srv.Process();
    return 0;
}
