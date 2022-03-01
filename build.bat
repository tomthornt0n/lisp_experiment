@echo off

set app_name=lisp_experiment

if not exist build mkdir build
pushd build

if not exist zlib.lib goto build_zlib
:ret_build_zlib

echo ~~~~~~~ shader compilation ~~~~~~~~
fxc.exe /nologo /T vs_5_0 /E vs /O3 /WX /Zpc /Ges /Fh w32__vshader.h /Vn d3d11_vshader /Qstrip_reflect /Qstrip_debug /Qstrip_priv ..\source\graphics\win32\w32__shader.hlsl
fxc.exe /nologo /T ps_5_0 /E ps /O3 /WX /Zpc /Ges /Fh w32__pshader.h /Vn d3d11_pshader /Qstrip_reflect /Qstrip_debug /Qstrip_priv ..\source\graphics\win32\w32__shader.hlsl

echo ~~~~~~~~~~~ debug build ~~~~~~~~~~~
set compile_options= /DUNICODE /D_UNICODE /DBuild_ModeDebug=1 /DBuild_UseSSE3=1 /DBuild_EnableAsserts=1
set compile_flags= -nologo /Zi /FC /Od /I../source/
set link_flags= /incremental:no /DEBUG:FULL /subsystem:console
cl.exe %compile_flags% %compile_options% /DApplicationName="%app_name%" ..\source\main.c zlib.lib /link %link_flags%  /out:"%app_name%_debug.exe"

echo ~~~~~~~~~~ release build ~~~~~~~~~~
set compile_options= /DUNICODE /D_UNICODE /DBuild_ModeRelease=1 /DBuild_NoCRT=1 /DBuild_UseSSE3=1 /DBuild_EnableAsserts=0
set compile_flags= -nologo /O2 /FC /I../source/ /GS- /Gs2097152 /utf-8 /Gm- /GR- /EHa- /Zi
set link_flags= /opt:ref /incremental:no /debug /subsystem:windows /STACK:0x100000,0x100000 /NODEFAULTLIB
set link_libraries= advapi32.lib d3d11.lib dxgi.lib dxguid.lib gdi32.lib kernel32.lib shell32.lib Ole32.lib shlwapi.lib user32.lib userenv.lib uuid.lib winmm.lib
cl.exe %compile_flags% %compile_options% /DApplicationName="%app_name%" ..\source\main.c zlib.lib /link %link_flags% %link_libraries% /out:"%app_name%.exe"

del *.obj

popd

exit /b

:build_zlib
set zlib_sources=..\source\external\zlib\adler32.c ..\source\external\zlib\compress.c ..\source\external\zlib\crc32.c ..\source\external\zlib\deflate.c ..\source\external\zlib\infback.c ..\source\external\zlib\inffast.c ..\source\external\zlib\inflate.c ..\source\external\zlib\inftrees.c  ..\source\external\zlib\trees.c ..\source\external\zlib\zutil.c
cl /nologo /c %zlib_sources% /O2 /FC /GS- /Gs2097152 /Gm- /GR- /EHa-
lib /nologo /out:zlib.lib adler32.obj compress.obj crc32.obj deflate.obj infback.obj inffast.obj inflate.obj  inftrees.obj trees.obj zutil.obj /NODEFAULTLIB
goto ret_build_zlib