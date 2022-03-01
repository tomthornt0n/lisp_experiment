typedef void *SharedLib;

Function SharedLib SharedLibMake      (S8 filename);
Function void     *SharedLibSymbolGet (SharedLib lib, char *symbol);
Function void      SharedLibDestroy   (SharedLib lib);
