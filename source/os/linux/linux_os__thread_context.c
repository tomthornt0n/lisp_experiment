
Global pthread_key_t linux_tc_key;
Global pthread_once_t linux_once_init = PTHREAD_ONCE_INIT;

Function void
LINUX_TCKeyMake_(void)
{
    pthread_key_create(&linux_tc_key, 0);
}
#define LINUX_TCEnsureIsInitialised pthread_once(&linux_once_init, LINUX_TCKeyMake_);

Function TC_Data *
TC_Get(void)
{
    LINUX_TCEnsureIsInitialised;
    TC_Data *result = pthread_getspecific(linux_tc_key);
    return result;
}

Function void
TC_Set(TC_Data *ptr)
{
    LINUX_TCEnsureIsInitialised;
    pthread_setspecific(linux_tc_key, ptr);
}
