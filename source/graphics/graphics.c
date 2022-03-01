
//~NOTE(tbt): platform agnostic implementations

#include "graphics__input.c"
#include "graphics__events.c"
#include "graphics__renderer.c"

//~NOTE(tbt): platform specific back-ends

#if Build_OSWindows
# include "win32/w32_graphics.c"
#elif Build_OSLinux
# include "linux/linux_graphics.c"
#else
# error no graphics backend for current platform
#endif
