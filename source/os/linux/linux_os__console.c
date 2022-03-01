
enum { LINUX_ConsoleStderr = 2, };

Function void
ConsoleOutputS8(S8 string)
{
    write(LINUX_ConsoleStderr, string.buffer, string.len);
}

Function void
ConsoleOutputS16(S16 string)
{
    M_Temp scratch = TC_ScratchGet(0, 0);
    S8 string_s8 = S8FromS16(scratch.arena, string);
    write(LINUX_ConsoleStderr, string_s8.buffer, string_s8.len);
    M_TempEnd(&scratch);
}

Function S8List
CmdLineGet(M_Arena *arena)
{
    S8List result = {0};
    M_Temp scratch = TC_ScratchGet(&arena, 1);
    
    
    int cmdline_fd = open("/proc/self/cmdline", O_RDONLY);
    char buf[4096] = {0};
    if(read(cmdline_fd, buf, sizeof(buf)) > 0)
    {
        close(cmdline_fd);
        
        char *cur = buf;
        while('\0' != *cur && cur < buf + sizeof(buf))
        {
            S8 str = CStringAsS8(cur);
            cur += str.len + 1;
            S8ListAppend(arena, &result, str);
        }
    }
    
    M_TempEnd(&scratch);
    return result;
}
