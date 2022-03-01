
//~NOTE(tbt): real world time

Function T_DateTime T_UTCGet     (void);                      // NOTE(tbt): current date-time in universal time coordinates
Function T_DateTime T_LTCGet     (void);                      // NOTE(tbt): current date-time in local time coordinates
Function T_DateTime T_LTCFromUTC (T_DateTime universal_time); // NOTE(tbt): convert from universal time coordinates to local time coordinates
Function T_DateTime T_UTCFromLTC (T_DateTime local_time);     // NOTE(tbt): convert from local time coordinates to universal time coordinates

//~NOTE(tbt): precision time

Function void T_Sleep(size_t milliseconds);

Function size_t T_MicrosecondsGet (void); // NOTE(tbt): query the performance counter to get the current time in microseconds
Function double T_SecondsGet      (void); // NOTE(tbt): query the performance counter and convert to seconds

#define T_LogBlock(N) size_t N ## _begin; DeferLoop(N ## _begin = T_MicrosecondsGet(), ConsoleOutputFmt(#N " took %zu microseconds\n", T_MicrosecondsGet() - N ## _begin))