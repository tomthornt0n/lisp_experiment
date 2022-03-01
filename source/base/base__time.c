Function T_DenseTime
T_DenseTimeFromDateTime(T_DateTime date_time)
{
    T_DenseTime result = 0;
    unsigned int encoded_year = (signed int)date_time.year + 0x8000;
    result = (result + encoded_year)   * 12;
    result = (result + date_time.mon)  * 31;
    result = (result + date_time.day)  * 24;
    result = (result + date_time.hour) * 60;
    result = (result + date_time.min)  * 61;
    result = (result + date_time.sec)  * 1000;
    result = (result + date_time.msec) * 1;
    return result;
}

Function T_DateTime
T_DateTimeFromDenseTime(T_DenseTime dense_time)
{
    T_DateTime result = {0};
    signed int encoded_year;
    result.msec = dense_time % 1000;
    dense_time /= 1000;
    result.sec = dense_time % 61;
    dense_time /= 61;
    result.min = dense_time % 60;
    dense_time /= 60;
    result.hour = dense_time % 24;
    dense_time /= 24;
    result.day = dense_time % 31;
    dense_time /= 31;
    result.mon = dense_time % 12;
    dense_time /= 12;
    encoded_year = (signed int)dense_time;
    result.year = encoded_year - 0x8000;
    return result;
}

Function S8
T_S8FromDateTime(M_Arena *arena, T_DateTime date_time)
{
    S8 result = S8FromFmt(arena, "%02d:%02d %02d/%02d/%d",
                          (int)date_time.hour,
                          (int)date_time.min,
                          (int)date_time.day + 1,
                          (int)date_time.mon + 1,
                          (int)date_time.year);
    return result;
}

Function S8
T_S8FromDenseTime(M_Arena *arena, T_DenseTime dense_time)
{
    T_DateTime date_time = T_DateTimeFromDenseTime(dense_time);
    S8 result = T_S8FromDateTime(arena, date_time);
    return result;
}
