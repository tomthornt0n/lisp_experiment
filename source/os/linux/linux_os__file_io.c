
#include <stdio.h>

Function size_t
LINUX_FileSizeGet(int fd)
{
    off_t current = lseek(fd, 0, SEEK_CUR);
    size_t result = lseek(fd, 0, SEEK_END);
    lseek(fd, current, SEEK_SET);
    return result;
}

Function S8
F_ReadEntire(M_Arena *arena, S8 filename)
{
    S8 result = {0};
    
    int fd = open(filename.buffer, O_RDONLY);
    if(fd > 0)
    {
        size_t n_total_bytes_to_read = LINUX_FileSizeGet(fd);
        
        M_Temp checkpoint = M_TempBegin(arena);
        
        result.len = n_total_bytes_to_read;
        result.buffer = M_ArenaPush(arena, result.len + 1);
        
        char *cursor = result.buffer;
        char *end = result.buffer + n_total_bytes_to_read;
        
        Bool success = True;
        while(cursor < end && success)
        {
            size_t n_bytes_to_read = end - cursor;
            ssize_t n_bytes_read = read(fd, cursor, n_bytes_to_read);
            if(n_bytes_read > 0)
            {
                cursor += n_bytes_read;
            }
            else
            {
                success = False;
            }
        }
        
        if(!success)
        {
            result.buffer = 0;
            result.len = 0;
            M_TempEnd(&checkpoint);
        }
        
        close(fd);
    }
    else
    {
        ConsoleOutputFmt("fd < 0");
    }
    return result;
}

Function S8
F_ReadTextEntire(M_Arena *arena, S8 filename)
{
    M_Temp scratch = TC_ScratchGet(&arena, 1);
    S8 raw = F_ReadEntire(scratch.arena, filename);
    S8 result = S8LFFromCRLF(arena, raw);
    M_TempEnd(&scratch);
    return result;
}

Function Bool
F_WriteEntire(S8 filename, S8 data)
{
    Bool success = True;
    
    int fd = creat(filename.buffer, S_IRUSR | S_IWUSR | S_IRGRP);
    if(fd > 0)
    {
        char *cursor = data.buffer;
        char *end = data.buffer + data.len;
        
        while(cursor < end && success)
        {
            size_t n_bytes_to_write = end - cursor;
            ssize_t n_bytes_written = write(fd, cursor, n_bytes_to_write);
            if(n_bytes_written > 0)
            {
                cursor += n_bytes_written;
            }
            else
            {
                success = False;
            }
        }
        
        close(fd);
    }
    else
    {
        success = False;
    }
    
    return success;
}

Function F_Properties
F_PropertiesGet(S8 filename)
{
    F_Properties result = {0};
    struct stat sb;
    if(-1 != stat(filename.buffer, &sb))
    {
        result.flags = F_PropertiesFlags_Exists;
        if(S_IFDIR == (sb.st_mode & S_IFMT))
        {
            result.flags |= F_PropertiesFlags_IsDirectory;
        }
        if('.' == FilenameLast(filename).buffer[0])
        {
            result.flags |= F_PropertiesFlags_Hidden;
        }
        
        if(0 == access(filename.buffer, R_OK))
        {
            result.access_flags |= DataAccessFlags_Read;
        }
        if(0 == access(filename.buffer, W_OK))
        {
            result.access_flags |= DataAccessFlags_Write;
        }
        if(0 == access(filename.buffer, X_OK))
        {
            result.access_flags |= DataAccessFlags_Execute;
        }
        
        result.size = sb.st_size;
        result.creation_time = 0; // TODO(tbt): statx or something
        result.access_time = LINUX_DenseTimeFromSeconds(sb.st_atim.tv_sec);
        result.write_time = LINUX_DenseTimeFromSeconds(sb.st_mtim.tv_sec);
    }
    return result;
}

Function Bool
F_Destroy(S8 filename)
{
    Bool success = (0 == unlink(filename.buffer));
    return success;
}

Function Bool
F_Move(S8 filename, S8 new_filename)
{
    Bool success = (0 == rename(filename.buffer, new_filename.buffer));
    return success;
}

Function Bool
F_DirectoryMake(S8 filename)
{
    Bool success = (0 == mkdir(filename.buffer, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH));
    return success;
}

Function Bool
F_DirectoryDestroy(S8 filename)
{
    Bool success = (0 == rmdir(filename.buffer));
    return success;
}

typedef struct
{
    F_Iterator _;
    DIR *d;
    struct dirent *dir;
    Bool is_done;
} LINUX_FileIterator;

Function F_Iterator *
F_IteratorMake(M_Arena *arena, S8 directory)
{
    LINUX_FileIterator *result = M_ArenaPush(arena, sizeof(*result));
    result->_.directory = S8Clone(arena, directory);
    result->d = opendir(directory.buffer);
    return (F_Iterator *)result;
}

Function Bool
F_IteratorNext(M_Arena *arena, F_Iterator *iter)
{
    Bool result = False;
    M_Temp scratch = TC_ScratchGet(&arena, 1);
    
    LINUX_FileIterator *_iter = (LINUX_FileIterator *)iter;
    if(_iter->d != 0)
    {
        while(!_iter->is_done)
        {
            _iter->dir = readdir(_iter->d);
            if(0 == _iter->dir)
            {
                _iter->is_done = True;
            }
            else
            {
                S8 filename = CStringAsS8(_iter->dir->d_name);
                
                Bool should_ommit = ((filename.len >= 2) &&
                                     (S8Match(filename, S8("."), MatchFlags_RightSideSloppy)) ||
                                     (S8Match(filename, S8(".."), MatchFlags_RightSideSloppy)));
                
                if(!should_ommit)
                {
                    S8 full_path = FilenamePush(scratch.arena, iter->directory, filename);
                    _iter->_.current_name = S8Clone(arena, filename);
                    _iter->_.current_full_path = AbsolutePathFromRelativePath(arena, full_path);
                    _iter->_.current_properties = F_PropertiesGet(_iter->_.current_full_path);
                    result = True;
                    break;
                }
            }
        }
    }
    
    M_TempEnd(&scratch);
    return result;
}

Function void
F_IteratorDestroy(F_Iterator *iter)
{
    LINUX_FileIterator *_iter = (LINUX_FileIterator *)iter;
    if(0 != _iter->d)
    {
        closedir(_iter->d);
    }
}

static S8
F_StdPathGet(M_Arena *arena, F_StdPath path)
{
    S8 result = {0};
    
    M_Temp scratch = TC_ScratchGet(&arena, 1);
    switch(path)
    {
        case(F_StdPath_CWD):
        {
            // NOTE(tbt): will fail if the length of the cwd > cap * grow_factor ^ grow_max
            
            size_t cap = 2048;
            int grow_max = 4;
            int grow_factor = 4;
            
            M_Temp before = M_TempBegin(scratch.arena);
            for(size_t r = 0; r < grow_max; r += 1, cap *= grow_factor)
            {
                char *buffer = M_ArenaPush(scratch.arena, cap);
                if(0 == getcwd(buffer, cap) && ERANGE == errno)
                {
                    M_TempEnd(&before);
                }
                else
                {
                    result = S8Clone(arena, CStringAsS8(buffer));
                    break;
                }
            }
        } break;
        
        case(F_StdPath_ExecutableFile):
        case(F_StdPath_ExecutableDir):
        {
            if(0 == TC_Get()->exe_path.buffer)
            {
                // NOTE(tbt): will fail if the length of the path to the exe > cap * grow_factor ^ grow_max
                
                size_t cap = 2048;
                int grow_max = 4;
                int grow_factor = 4;
                
                M_Temp before = M_TempBegin(scratch.arena);
                for(size_t r = 0; r < grow_max; r += 1, cap *= grow_factor)
                {
                    char *buffer = M_ArenaPush(scratch.arena, cap);
                    if(cap == readlink("/proc/self/exe", buffer, cap))
                    {
                        M_TempEnd(&before);
                    }
                    else
                    {
                        TC_Get()->exe_path = S8Clone(&TC_Get()->permanent_arena, CStringAsS8(buffer));
                        break;
                    }
                }
            }
            
            if(0 != TC_Get()->exe_path.buffer)
            {
                result = TC_Get()->exe_path;
                if(F_StdPath_ExecutableDir == path)
                {
                    result = FilenamePop(result);
                }
                result = S8Clone(arena, result);
            }
        } break;
        
        case(F_StdPath_Home):
        {
            char *home = getenv("HOME");
            if(0 != home)
            {
                result = S8Clone(arena, CStringAsS8(home));
            }
        } break;
        
        case(F_StdPath_Config):
        {
            char *config = getenv("XDG_CONFIG_HOME");
            if(0 == config)
            {
                result = FilenamePush(arena, F_StdPathGet(scratch.arena, F_StdPath_Home), S8(".config"));
            }
            else
            {
                result = S8Clone(arena, CStringAsS8(config));
            }
        } break;
        
        case(F_StdPath_Temp):
        {
            // NOTE(tbt): should this be copied onto the passed in arena?
            result = S8("/tmp");
        } break;
    }
    M_TempEnd(&scratch);
    
    return result;
}

Function void
F_Exec(S8 filename, F_ExecVerb verb)
{
    M_Temp scratch = TC_ScratchGet(0, 0);
    
    switch(verb)
    {
        case(F_ExecVerb_Default):
        case(F_ExecVerb_Open):
        case(F_ExecVerb_Edit):
        {
            S8 cmd = S8FromFmt(scratch.arena, "xdg-open %.*s & disown", FmtS8(filename));
            system(cmd.buffer);
        } break;
        
        case(F_ExecVerb_Print):
        {
            // TODO(tbt): i'm not really sure what the best option is here?
            S8 cmd = S8FromFmt(scratch.arena, "lp %.*s & disown", FmtS8(filename));
            system(cmd.buffer);
        } break;
    }
    
    M_TempEnd(&scratch);
}

Function void
CwdSet(S8 cwd)
{
    chdir(cwd.buffer);
}

Function S8
AbsolutePathFromRelativePath(M_Arena *arena, S8 path)
{
    char *absolute_path = realpath(path.buffer, M_ArenaPush(arena, PATH_MAX));
    S8 result = CStringAsS8(absolute_path);
    return result;
}

Function F_ChangeHandle
F_ChangeHandleMake(S8 filename, Bool recursive)
{
    uint32_t mask = IN_CREATE | IN_DELETE | IN_DELETE_SELF | IN_MOVE_SELF | IN_MOVED_TO;
    
    // TODO(tbt): observe recursive argument
    
    union
    {
        F_ChangeHandle handle;
        int fd;
    } result =
    {
        .fd = inotify_init1(IN_NONBLOCK),
    };
    inotify_add_watch(result.fd, filename.buffer, mask);
    
    return result.handle;
}

Function Bool
F_ChangeHandleWait(F_ChangeHandle handle, F_ChangeHandleTimeout milliseconds)
{
    Bool result = True;
    
    union
    {
        F_ChangeHandle handle;
        int fd;
    } watch =
    {
        .handle = handle,
    };
    
    size_t begin = T_MicrosecondsGet();
    char buffer[4096];
    while(read(watch.fd, buffer, sizeof(buffer)) < 0 && EWOULDBLOCK == errno)
    {
        // TODO(tbt): avoid busy wait
        size_t elapsed = (((T_MicrosecondsGet() - begin) + 500) / 1000);
        if(F_ChangeHandleTimeout_Infinite != milliseconds && elapsed >= milliseconds)
        {
            result = False;
            break;
        }
    }
    
    return result;
}

Function void
F_ChangeHandleDestroy(F_ChangeHandle handle)
{
    union
    {
        F_ChangeHandle handle;
        int fd;
    } watch =
    {
        .handle = handle,
    };
    close(watch.fd);
}
