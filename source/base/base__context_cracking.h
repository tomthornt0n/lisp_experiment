
//~NOTE(tbt): msvc
#if defined(_MSC_VER)
# define Build_CompilerMSVC 1

//-NOTE(tbt): os detection
# if defined(_WIN32)
#  define Build_OSWindows 1
# else
#  error Could not detect OS
# endif

//-NOTE(tbt): architecture detection
# if defined(_M_AMD64)
#  define Build_ArchX64 1
# elif defined(_M_I86)
#  define Build_ArchX86 1
# elif defined(_M_ARM)
#  define Build_ArchARM 1
// TODO(tbt): ARM64 ???
# else
#  error Could not detect architecture
# endif

//~NOTE(tbt): clang
#elif defined(__clang__)
# define Build_CompilerClang 1

//-NOTE(tbt): os detection
# if defined(_WIN32)
#  define Build_OSWindows 1
# elif defined(__gnu_linux__)
#  define Build_OSLinux 1
# elif defined(__APPLE__) && defined(__MACH__)
#  define Build_OSMac 1
# else
#  error Could not detect OS
# endif

//-NOTE(tbt): architecture detection
# if defined(__amd64__)
#  define Build_ArchX64 1
# elif defined(__i386__)
#  define Build_ArchX86 1
# elif defined(__arm__)
#  define Build_ArchARM 1
# elif defined(__aarch64__)
#  define Build_ArchARM64 1
# else
#  error Could not detect architecture
# endif

//~NOTE(tbt): GCC
#elif defined(__GNUC__)
# define Build_CompilerGCC 1

//-NOTE(tbt): os detection
# if defined(_WIN32)
#  define Build_OSWindows 1
# elif defined(__gnu_linux__)
#  define Build_OSLinux 1
# elif defined(__APPLE__) && defined(__MACH__)
#  define Build_OSMac 1
# else
#  error Could not detect OS
# endif

//-NOTE(tbt): architecture detection
# if defined(__amd64__)
#  define Build_ArchX64 1
# elif defined(__i386__)
#  define Build_ArchX86 1
# elif defined(__arm__)
#  define Build_ArchARM 1
# elif defined(__aarch64__)
#  define Build_ArchARM64 1
# else
#  error Could not detect architecture
# endif

#else
# error Could not detect compiler

#endif

//~NOTE(tbt): zero fill non set macros

//-NOTE(tbt): compiler
#if !defined(Build_CompilerMSVC)
# define Build_CompilerMSVC 0
#endif
#if !defined(Build_CompilerClang)
# define Build_CompilerClang 0
#endif
#if !defined(Build_CompilerGCC)
# define Build_CompilerGCC 0
#endif

//-NOTE(tbt): OS
#if !defined(Build_OSWindows)
# define Build_OSWindows 0
#endif
#if !defined(Build_OSLinux)
# define Build_OSLinux 0
#endif
#if !defined(Build_OSMac)
# define Build_OSMac 0
#endif

//-NOTE(tbt): architecture
#if defined(Build_ArchX64)
# define Build_UseSSE2 1
# include <emmintrin.h>
# include <immintrin.h>
#else
# define Build_ArchX64 0
#endif
#if !defined(Build_ArchX86)
# define Build_ArchX86 0
#endif
#if !defined(Build_ArchARM)
# define Build_ArchARM 0
#endif
#if !defined(Build_ArchARM64)
# define Build_ArchARM64 0
#endif
#if !defined(Build_UseSSE3)
# define Build_UseSSE3 0
#endif
#if !defined(Build_UseSSE2)
# if Build_UseSSE3
#  define Build_UseSSE2 1
# else
#  define Build_UseSSE2 0
# endif
#endif

//-NOTE(tbt): build options
#if !defined(Build_ModeDebug)
# define Build_ModeDebug 0
#endif
#if !defined(Build_ModeRelease)
# define Build_ModeRelease 0
#endif
#if !defined(Build_NoCRT)
# define Build_NoCRT 0
#endif
#if !defined(Build_EnableAsserts)
# define Build_EnableAsserts 0
#endif
