#ifndef LOG_H_
#define LOG_H_

#include <time.h>
#include <iostream>
#include <string.h>


#define LOG_BUF_SIZE 1024
#define TIME_BUF_SIZE 128


#define LOG(level, fmt, varlist...) \
    do { \
        char log_buf[LOG_BUF_SIZE]; \
        snprintf(log_buf, sizeof(log_buf), fmt, ##varlist); \
        char time_buf[TIME_BUF_SIZE]; \
        time_t now = time(NULL); \
        tm tmp; \
        localtime_r(&now, &tmp); \
        strftime(time_buf, sizeof(time_buf), "%F %T", &tmp); \
        cout<<time_buf<<"\t"<<level<<"\t"<<log_buf<<endl; \
    } while(0)

#ifdef DEBUG
#define DEBUG_LOG(fmt, varlist...) \
    do { \
        LOG("DEBUG", fmt, ##varlist); \
    } while(0)
#else
#define DEBUG_LOG(fmt, varlist...)
#endif

#define INFO_LOG(fmt, varlist...) \
    do { \
        LOG("INFO", fmt, ##varlist); \
    } while(0)

#define WARN_LOG(fmt, varlist...) \
    do { \
        LOG("WARN", fmt, ##varlist); \
    } while(0)

#define ERR_LOG(fmt, varlist...) \
    do { \
        LOG("ERROR", fmt, ##varlist); \
    } while(0)

#endif /* LOG_H_ */
