Function void
ConsoleOutputFmtV(char *fmt, va_list args)
{
    M_Temp scratch = TC_ScratchGet(0, 0);
    S8 string = S8FromFmtV(scratch.arena, fmt, args);
    ConsoleOutputS8(string);
    M_TempEnd(&scratch);
}

Function void
ConsoleOutputFmt(char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    ConsoleOutputFmtV(fmt, args);
    va_end(args);
}