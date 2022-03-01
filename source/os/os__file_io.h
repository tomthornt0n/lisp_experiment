
//~NOTE(tbt): file properties

typedef unsigned int F_PropertiesFlags;
typedef enum
{
    F_PropertiesFlags_Exists      = Bit(0),
    F_PropertiesFlags_IsDirectory = Bit(1),
    F_PropertiesFlags_Hidden      = Bit(2),
} F_PropertiesFlags_ENUM;

typedef struct
{
    F_PropertiesFlags flags;
    size_t size;
    DataAccessFlags access_flags;
    T_DenseTime creation_time;
    T_DenseTime access_time;
    T_DenseTime write_time;
} F_Properties;

Function F_Properties F_PropertiesGet(S8 filename);

//~NOTE(tbt): basic file IO operations

Function S8   F_ReadEntire     (M_Arena *arena, S8 filename);
Function S8   F_ReadTextEntire (M_Arena *arena, S8 filename);
Function Bool F_WriteEntire    (S8 filename, S8 data);

//~NOTE(tbt): file management

Function Bool F_Destroy          (S8 filename);                  // NOTE(tbt): deletes a single file
Function Bool F_Move             (S8 filename, S8 new_filename); // NOTE(tbt): renames or moves a file or directory
Function Bool F_DirectoryMake    (S8 filename);                  // NOTE(tbt): creates an empty directory
Function Bool F_DirectoryDestroy (S8 filename);                  // NOTE(tbt): deletes an empty directory

//~NOTE(tbt): directory iteration

typedef struct
{
    S8 directory;
    S8 current_name;
    S8 current_full_path;
    F_Properties current_properties;
} F_Iterator;

Function F_Iterator *F_IteratorMake    (M_Arena *arena, S8 filename);
Function Bool        F_IteratorNext    (M_Arena *arena, F_Iterator *iter);
Function void        F_IteratorDestroy (F_Iterator *iter);

//~NOTE(tbt): standard paths

typedef enum
{
    F_StdPath_CWD,
    F_StdPath_ExecutableFile,
    F_StdPath_ExecutableDir,
    F_StdPath_Home,
    F_StdPath_Config,
    F_StdPath_Temp,
    
    F_StdPath_MAX,
} F_StdPath;

static S8 F_StdPathGet(M_Arena *arena, F_StdPath path);

//~NOTE(tbt): opening/executing files

typedef enum
{
    F_ExecVerb_Default,
    F_ExecVerb_Open,
    F_ExecVerb_Edit,
    F_ExecVerb_Print,
    F_ExecVerb_MAX,
} F_ExecVerb;
Function void F_Exec(S8 filename, F_ExecVerb verb);

//~NOTE(tbt): path utils

Function void CwdSet                       (S8 cwd);
Function S8   AbsolutePathFromRelativePath (M_Arena *arena, S8 path);

//~NOTE(tbt): directory change notifications

typedef void *F_ChangeHandle;
typedef enum
{
    F_ChangeHandleTimeout_Infinite = ~((size_t)0),
} F_ChangeHandleTimeout;

Function F_ChangeHandle F_ChangeHandleMake    (S8 filename, Bool recursive);
Function Bool           F_ChangeHandleWait    (F_ChangeHandle handle, F_ChangeHandleTimeout milliseconds);
Function void           F_ChangeHandleDestroy (F_ChangeHandle handle);


//~NOTE(tbt): error string

Function S8 F_LastErrorStringGet (M_Arena *arena);