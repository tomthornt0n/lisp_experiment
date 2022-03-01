
pushd build > /dev/null

app_name="near_manager"

echo compiling $app_name

echo ~~~~~~~~~~~ debug build ~~~~~~~~~~~
compile_optns="-D Build_ModeDebug=1 -D Build_EnableAsserts=1 -D Build_UseSSE3=1 -D ApplicationName=$app_name"
compile_flags='-g -O0 -I../source/ -msse4.1 -ldl -lpthread -lGL -lxcb -lX11 -lX11-xcb -lm -lz'
gcc $compile_optns $compile_flags ../source/main.c -o "$app_name"_debug

echo ~~~~~~~~~~ release build ~~~~~~~~~~
compile_optns="-D Build_ModeRelease=1 -D Build_UseSSE3=1 -D ApplicationName=$app_name"
compile_flags='-O2 -I../source/ -msse4.1 -ldl -lpthread -lGL -lxcb -lX11 -lX11-xcb -lm -lz'
gcc $compile_optns $compile_flags ../source/main.c -o $app_name

popd > /dev/null
