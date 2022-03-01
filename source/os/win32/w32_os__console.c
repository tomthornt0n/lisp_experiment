Global HANDLE w32_console_handle = INVALID_HANDLE_VALUE;

Function void
W32_ConsoleInit(void)
{
    FreeConsole();
    AllocConsole();
    SetConsoleCP(CP_UTF8);
    w32_console_handle = GetStdHandle(STD_OUTPUT_HANDLE);
    
    // TODO(tbt): setup stdout handle to use ANSI escape sequence things
}

Function void
ConsoleOutputS8(S8 string)
{
    M_Temp scratch = TC_ScratchGet(0, 0);
    S16 string_s16 = S16FromS8(scratch.arena, string);
    WriteConsoleW(w32_console_handle, string_s16.buffer, string_s16.len, 0, 0);
    OutputDebugStringW(string_s16.buffer);
    M_TempEnd(&scratch);
}

Function void
ConsoleOutputS16(S16 string)
{
    WriteConsoleW(w32_console_handle, string.buffer, string.len, 0, 0);
    OutputDebugStringW(string.buffer);
}

Function S8List
CmdLineGet(M_Arena *arena)
{
    S8List result = {0};
    
    int arguments_count;
    LPWSTR *arguments = CommandLineToArgvW(GetCommandLineW(), &arguments_count);
    
    for(size_t argument_index = 0;
        argument_index < arguments_count;
        argument_index += 1)
    {
        S8Node *node = M_ArenaPush(arena, sizeof(*node));
        node->string = S8FromS16(arena, CStringAsS16(arguments[argument_index]));
        S8ListAppendExplicit(&result, node);
    }
    
    LocalFree(arguments);
    
    return result;
}
