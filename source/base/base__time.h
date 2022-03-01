
typedef struct
{
    unsigned short msec; // NOTE(tbt): milliseconds after the seconds, [0, 999]
    unsigned char sec;   // NOTE(tbt): seconds after the minute, [0, 60] (60 allows for leap seconds)
    unsigned char min;   // NOTE(tbt): minutes after the hour, [0, 59]
    unsigned char hour;  // NOTE(tbt): hours into the day, [0, 23]
    unsigned char day;   // NOTE(tbt): days into the month, [0, 30]
    unsigned char mon;   // NOTE(tbt): months into the year, [0, 11]
    signed short year;   // NOTE(tbt): ...  -1 == -2 BCE; 0 == -1 BCE; 1 == 1 CE, 2 == 2 CE; ...
} T_DateTime;

typedef uint64_t T_DenseTime;

Function T_DenseTime T_DenseTimeFromDateTime (T_DateTime date_time);
Function T_DateTime  T_DateTimeFromDenseTime (T_DenseTime dense_time);

// TODO(tbt): strftime style format strings? probably actually wouldn't be that useful:
//            you can just call S8FromFmt() with the desired fields of T_DateTime
Function S8 T_S8FromDateTime  (M_Arena *arena, T_DateTime date_time);
Function S8 T_S8FromDenseTime (M_Arena *arena, T_DenseTime dense_time);
