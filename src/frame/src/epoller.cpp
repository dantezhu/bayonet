#include "epoller.h"

CEPoller::CEPoller()
{
    m_epollFd = -1;
}

CEPoller::~CEPoller()
{
}

int CEPoller::Create(int maxfd)
{
    m_epollFd = epoll_create(maxfd);
    if ( m_epollFd <= 0 )
    {
        snprintf(m_szErrMsg,NET_ERRMSG_SIZE,"epoller init error,size:%d,error:%s\n",maxfd,strerror(errno));
        return -1;
    }

    return 0;
}

void CEPoller::AttachSocket(CSocketActorBase* pSocketActor)
{
    int fd = pSocketActor->GetSocketFd();
    if ( fd > 0 )
        m_mapSocketActor[fd]=pSocketActor;

    return ;
}

void CEPoller::DetachSocket(CSocketActorBase* pSocketActor)
{
    int fd = pSocketActor->GetSocketFd();
    if ( fd > 0 )
    {
        DelEpollIO(fd);
        m_mapSocketActor.erase(fd);
    }

    return ;
}

int CEPoller::LoopForEvent(int timeout)
{
    int fd;
    int nfds;
    CSocketActorBase*  pSocketActor = NULL;
    unsigned ev;
    struct timeval prev_tm;
    struct timeval next_tm;
    long    use_time_usec;
    gettimeofday(&prev_tm,NULL);

    for(;;)
    {
        nfds = epoll_wait(m_epollFd, m_events, EPOLL_FD_MAX, timeout);

        if (nfds < 0)
        {
            if ( errno == EINTR )
                continue;

            snprintf(m_szErrMsg,NET_ERRMSG_SIZE,"epoll-wait rtn:%d error:%s\n",nfds,strerror(errno));
            return -1;
        }

        for( int i=0;i<nfds;i++ )
        {
            fd = m_events[i].data.fd;
            pSocketActor = m_mapSocketActor[fd];
            if ( pSocketActor == NULL )
            {
                DelEpollIO(fd); close(fd);
                continue;
            }

            ev = m_events[i].events;
            int ret = 0;
            if ( ev&EPOLLIN )
                ret = pSocketActor->ChangeState(SOCKET_FSM_RECVING);                         
            else if ( ev&EPOLLOUT )
                ret = pSocketActor->ChangeState(SOCKET_FSM_SENDING);
            else if ( ev&EPOLLHUP )
                ret = pSocketActor->ChangeState(SOCKET_FSM_CLOSING);
            else if ( ev&EPOLLERR )
                ret = pSocketActor->ChangeState(SOCKET_FSM_ERROR);
            else
                ret = pSocketActor->ChangeState(SOCKET_FSM_ERROR);
        }

        gettimeofday(&next_tm,NULL);
        use_time_usec = (next_tm.tv_sec - prev_tm.tv_sec)*1000000 +
            (next_tm.tv_usec - prev_tm.tv_usec);
        if ( use_time_usec > (1000))
        {
            for(map<int, CSocketActorBase*>::iterator it = m_mapSocketActor.begin(); it != m_mapSocketActor.end();)
            {
                //这样写，就可以自由的删掉自己了
                map<int, CSocketActorBase*>::iterator tempIt = it;
                it++;

                tempIt->second->CheckTimeOut(next_tm);
            }
            prev_tm = next_tm;
        }
    }
}

char * CEPoller::GetErrMsg()
{
    return m_szErrMsg;
}

int CEPoller::SetEpollIO(int fd,unsigned flag)
{
    epoll_event ev;
    ev.data.fd = fd;
    ev.events = flag|EPOLLHUP|EPOLLERR;

    if ( epoll_ctl(m_epollFd, EPOLL_CTL_MOD , fd, &ev) < 0 )    
    {
        if ( epoll_ctl(m_epollFd, EPOLL_CTL_ADD , fd, &ev) < 0 )
        {
            snprintf(m_szErrMsg,NET_ERRMSG_SIZE,"epoll_ctl fd:%d err:%s\n",fd,strerror(errno));
            return -1;
        }
    }

    return 0;
}

int CEPoller::AddEpollIO(int fd,unsigned flag)
{
    epoll_event ev;
    ev.data.fd = fd;
    ev.events = flag;

    if ( epoll_ctl(m_epollFd, EPOLL_CTL_ADD , fd, &ev) < 0 )
        return -1;

    return 0;
}

int CEPoller::ModEpollIO(int fd,unsigned flag)
{
    epoll_event ev;
    ev.data.fd = fd;
    ev.events = flag;

    if ( epoll_ctl(m_epollFd, EPOLL_CTL_MOD , fd, &ev) < 0 )    
    {
        return -1;
    }

    return 0;
}

int CEPoller::DelEpollIO(int fd)
{
    epoll_event ev;
    ev.data.fd = fd;
    ev.events = 0;
    if ( epoll_ctl(m_epollFd, EPOLL_CTL_DEL, fd, &ev) < 0 )
        return -1;

    return 0;
}
