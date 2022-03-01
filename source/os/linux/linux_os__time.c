
//~NOTE(tbt): internal helpers

typedef struct tm TM;

Function T_DateTime
LINUX_DateTimeFromTm(TM tm)
{
    T_DateTime result;
    result.year = tm.tm_year + 1900;
    result.mon = tm.tm_mon;
    result.day = tm.tm_mday - 1;
    result.hour = tm.tm_hour;
    result.min = tm.tm_min;
    result.sec = tm.tm_sec;
    result.msec = 0; // TODO(tbt): get this somehow
    return result;
}

Function TM
LINUX_TmFromDateTime(T_DateTime t)
{
    TM result;
    result.tm_year = t.year - 1900;
    result.tm_mon = t.mon;
    result.tm_mday = t.day + 1;
    result.tm_hour = t.hour;
    result.tm_min = t.min;
    result.tm_sec = t.sec;
    result.tm_isdst = -1;
    mktime(&result);
    return result;
}

Function T_DenseTime
LINUX_DenseTimeFromSeconds(time_t t)
{
    TM tm;
    localtime_r((time_t[1]){0}, &tm);
    tm.tm_sec += t;
    mktime(&tm);
    T_DateTime date_time = LINUX_DateTimeFromTm(tm);
    T_DenseTime result = T_DenseTimeFromDateTime(date_time);
    return result;
}

//~NOTE(tbt): real world time

Function T_DateTime
T_UTCGet(void)
{
    time_t t = time(0);
    TM tm;
    gmtime_r(&t, &tm);
    T_DateTime result = LINUX_DateTimeFromTm(tm);
    return result;
}

Function T_DateTime
T_LTCGet(void)
{
    time_t t = time(0);
    TM tm;
    localtime_r(&t, &tm);
    T_DateTime result = LINUX_DateTimeFromTm(tm);
    return result;
}

Function T_DateTime
T_LTCFromUTC(T_DateTime universal_time)
{
    TM tm = LINUX_TmFromDateTime(universal_time);
    time_t t = mktime(&tm);
    localtime_r(&t, &tm);
    T_DateTime result = LINUX_DateTimeFromTm(tm);
    return result;
}

Function T_DateTime
T_UTCFromLTC(T_DateTime local_time)
{
    TM tm = LINUX_TmFromDateTime(local_time);
    time_t t = mktime(&tm);
    gmtime_r(&t, &tm);
    T_DateTime result = LINUX_DateTimeFromTm(tm);
    return result;
}

//~NOTE(tbt): precision time


Function void
T_Sleep(size_t milliseconds)
{
    struct timespec ts =
    {
        .tv_sec = milliseconds / 1000,
        .tv_nsec = (milliseconds % 1000)*1000000,
    };
    int r;
    do
    {
        r = nanosleep(&ts, &ts);
    } while(r && errno == EINTR);
}

Function size_t
T_MicrosecondsGet(void)
{
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC_RAW, &ts);
    size_t result = (ts.tv_sec*1000000000 + ts.tv_nsec)/1000;
    return result;
}
