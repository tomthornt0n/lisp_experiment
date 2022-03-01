
Function double
T_SecondsGet(void)
{
    size_t microseconds = T_MicrosecondsGet();
    double seconds = (double)microseconds * 0.000001;
    return seconds;
}
