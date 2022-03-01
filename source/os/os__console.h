
// TODO(tbt): console input

// TODO(tbt): helpers for escape sequences

Function void ConsoleOutputS8   (S8 string);
Function void ConsoleOutputS16  (S16 string);
Function void ConsoleOutputFmtV (char *fmt, va_list args);
Function void ConsoleOutputFmt  (char *fmt, ...);

Function S8List CmdLineGet(M_Arena *arena);
