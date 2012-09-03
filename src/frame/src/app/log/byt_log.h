/*=============================================================================
#
#     FileName: byt_log.h
#         Desc: ��ӡlog
#
#       Author: dantezhu
#        Email: zny2008@gmail.com
#     HomePage: http://www.vimer.cn
#
#      Created: 2011-06-30 00:19:34
#      Version: 0.0.1
#      History:
#               0.0.1 | dantezhu | 2011-06-30 00:19:34 | initialization
#
=============================================================================*/

#ifndef _BYT_LOG_H_20110805110834_
#define _BYT_LOG_H_20110805110834_

#include <string.h>
#include <stdio.h>
#include <stdarg.h>
#include <time.h>
#include <unistd.h>

#include <sys/mman.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <assert.h>

//=============================================================================
// ֱ�ӵ��õĺ�
#define byt_log_init(lvl, path, args...)    bayonet::APILogInit(lvl, path, ##args) //log���ʼ��

#define byt_log_detail(lvl, fmt, args...)   bayonet::APILogWrite(lvl, "[%s:%u][%s] " fmt "\n", __FILE__, __LINE__, __FUNCTION__, ##args)

#define byt_trace_log(fmt, args...)     byt_log_detail(bayonet::LM_TRACE, fmt, ##args)
#define byt_debug_log(fmt, args...)     byt_log_detail(bayonet::LM_DEBUG, fmt, ##args)
#define byt_warn_log(fmt, args...)      byt_log_detail(bayonet::LM_WARNING, fmt, ##args)
#define byt_info_log(fmt, args...)      byt_log_detail(bayonet::LM_INFO, fmt, ##args)
#define byt_error_log(fmt, args...)     byt_log_detail(bayonet::LM_ERROR, fmt, ##args)
#define byt_fatal_log(fmt, args...)     byt_log_detail(bayonet::LM_FATAL, fmt, ##args)

#define byt_screen_info(fmt, args...)   fprintf(stdout, fmt, ##args); fprintf(stdout, "\n")
//=============================================================================

namespace bayonet {

inline void screen_output(const char* fmt, ...)
{
    time_t now = time(NULL);
    struct tm tm;
    localtime_r(&now, &tm);
    char temp_time[64];
    strftime(temp_time,sizeof(temp_time)-1,"%Y-%m-%d %H:%M:%S",&tm);

    fprintf(stdout, "[%s][%05d]", temp_time, getpid());

    va_list ap;
    va_start(ap, fmt);
    vfprintf(stdout, fmt, ap);
    va_end(ap);
    fprintf(stdout, "\n");
}


////////////////////////////////////////////////////////////////////////////////
// CBytLog
////////////////////////////////////////////////////////////////////////////////

// log�ļ�Ĭ�ϴ�С
const int LOG_DEFAULT_SIZE        = 200000000;
// ͬ��log�ļ�Ĭ���������
const int LOG_DEFAULT_MAXNUM      = 1000;
// log level������
const int LOG_LEVEL_MAXNUM        = 10;

// ����log�����С
const int LOG_MSG_SIZE            = 4096;

typedef enum _Log_Level_
{
    LM_ALL          = 0,
    LM_TRACE        = 1,
    LM_DEBUG        = 2,
    LM_WARNING      = 3,
    LM_INFO         = 4,
    LM_ERROR        = 5,
    LM_FATAL        = 6,
    LM_NONE         = 10
}LogLevel;

typedef struct
{
    int fd;
    int seq;
}LogFileInfo;

class CBytLog
{
    public:
        CBytLog();
        ~CBytLog();

    public:
        int Init(LogLevel logLevel, const char* logDir, const char* logName, unsigned long logSize=LOG_DEFAULT_SIZE);
        int WriteLog(LogLevel logLevel, const char* logFormat, ...);
        int VWriteLog(LogLevel logLevel, const char* logFormat,va_list ap);

    private:
        int OpenLogFile();
        int GetLogFileName(char* logFile, int iLen);
        int ShiftLogFiles(LogLevel logLevel, unsigned long maxsize, unsigned short maxnum);

    private:
        char* m_LogBuf;
        LogLevel m_LogLevel;
        char m_LogDir[256];
        char m_LogName[64];
        unsigned long m_LogSize;

        LogFileInfo m_LogFileInfo;

    public:
        static CBytLog* m_instance;
        static CBytLog* instance();
};

extern int APILogInit(LogLevel logLevel, const char* logDir, const char* logName, unsigned long logSize);
extern int APILogWrite(LogLevel logLevel, const char* logFormat, ...);
}

#endif
