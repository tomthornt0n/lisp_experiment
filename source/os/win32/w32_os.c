
//~NOTE(tbt): platform specific system headers

#define COBJMACROS
#define WIN32_LEAN_AND_MEAN

#undef Function

#include <windows.h>

#include <commctrl.h>
#include <d3d11.h>
#include <d3dcompiler.h>
#include <intrin.h>
#include <shellapi.h>
#include <shlobj_core.h>
#include <shlwapi.h>
#include <thumbcache.h>
#include <timeapi.h>
#include <wincrypt.h>

#define Function static

#if Build_NoCRT
# include "w32_os__crt_replacement.h"
Function int APP_EntryPoint(void);
int __stdcall WinMainCRTStartup()
{
    int rc = APP_EntryPoint();
    ExitProcess(rc);
}
# endif

//~NOTE(tbt): libraries

// NOTE(tbt): for some reason does not work and will have to be declared at command line when using /nodefaultlib
#pragma comment (lib, "advapi32.lib")
#pragma comment (lib, "gdi32.lib")
#pragma comment (lib, "kernel32.lib")
#pragma comment (lib, "ole32.lib")
#pragma comment (lib, "shell32.lib")
#pragma comment (lib, "shlwapi.lib")
#pragma comment (lib, "user32.lib")
#pragma comment (lib, "userenv.lib")
#pragma comment (lib, "uuid.lib")
#pragma comment (lib, "winmm.lib")

//~NOTE(tbt): OS layer backend implementation files

#include "w32_os__internal.c"
#include "w32_os__thread_context.c"
#include "w32_os__memory.c"
#include "w32_os__console.c"
#include "w32_os__time.c"
#include "w32_os__file_io.c"
#include "w32_os__entropy.c"
#include "w32_os__shared_libraries.c"
#include "w32_os__thread.c"
#include "w32_os__init.c"
#include "w32_os__clipboard.c"
