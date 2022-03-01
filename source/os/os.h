
//~NOTE(tbt): OS layer headers
#include "os__init.h"
#include "os__memory.h"
#include "os__thread_context.h"
#include "os__console.h"
#include "os__time.h"
#include "os__file_io.h"
#include "os__entropy.h"
#include "os__shared_libraries.h"
#include "os__thread.h"
#include "os__clipboard.h"
#include "os__work_queue.h"

#if Build_NoCRT
Function int APP_EntryPoint(void);
#endif