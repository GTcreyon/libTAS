#include "threads.h"
#include "logging.h"
#include <errno.h>
#include <unistd.h>

pthread_t mainThread = 0;

/* Get the current thread id */
pthread_t getThreadId(void)
{
    if (pthread_self_real)
        return pthread_self_real();
    return 0;
}

/* Indicate that we are running on the main thread */
void setMainThread(void)
{
    if (mainThread != 0)
        /* Main thread was already set */
        return;
    if (pthread_self_real)
        mainThread = pthread_self_real();
}

/* 
 * We will often want to know if we are running on the main thread
 * Because only it can advance the deterministic timer, and other stuff 
 */
int isMainThread(void)
{
    if (pthread_self_real)
        return (pthread_self_real() == mainThread);

    /* If pthread library is not loaded, it is likely that the game is single threaded */
    return 1;

    //return (getppid() == getpgrp());
}


/* Override */ SDL_Thread* SDL_CreateThread(SDL_ThreadFunction fn, const char *name, void *data)
{
    debuglog(LCF_THREAD, "SDL Thread ", name, " was created.");
    return SDL_CreateThread_real(fn, name, data);
}

/* Override */ void SDL_WaitThread(SDL_Thread * thread, int *status)
{
    debuglog(LCF_THREAD, "Waiting for another SDL thread.");
    SDL_WaitThread_real(thread, status);

}
/*
void SDL_DetachThread(SDL_Thread * thread)
{
    debuglog(LCF_THREAD, "Some SDL thread is detaching.\n");
    SDL_DetachThread(thread);
}
*/

/* Override */ int pthread_create (pthread_t * thread, const pthread_attr_t * attr, void * (* start_routine) (void *), void * arg) throw()
{
    char name[16];
    name[0] = '\0';
    int ret = pthread_create_real(thread, attr, start_routine, arg);
    pthread_getname_np_real(*thread, name, 16);
    std::string thstr = stringify(*thread);
    if (name[0])
        debuglog(LCF_THREAD, "Thread ", thstr, " was created (", name, ").");
    else
        debuglog(LCF_THREAD, "Thread ", thstr, " was created.");
    return ret;
}

/* Override */ void pthread_exit (void *retval)
{
    debuglog(LCF_THREAD, "Thread has exited.");
    pthread_exit_real(retval);
}

/* Override */ int pthread_join (pthread_t thread, void **thread_return)
{
    //if (1) {
    //    return 0;
    //}
    std::string thstr = stringify(thread);
    debuglog(LCF_THREAD, "Joining thread ", thstr);
    return pthread_join_real(thread, thread_return);
}

/* Override */ int pthread_detach (pthread_t thread) throw()
{
    //if (1) {
    //    return 0;
    //}
    std::string thstr = stringify(thread);
    debuglog(LCF_THREAD, "Detaching thread ", thstr);
    return pthread_detach_real(thread);
}

/* Override */ int pthread_tryjoin_np(pthread_t thread, void **retval) throw()
{
    std::string thstr = stringify(thread);
    debuglog(LCF_THREAD, "Try to join thread ", thstr);
    int ret = pthread_tryjoin_np_real(thread, retval);
    if (ret == 0) {
        debuglog(LCF_THREAD, "Joining thread ", thstr, " successfully.");
    }
    if (ret == EBUSY) {
        debuglog(LCF_THREAD, "Thread ", thstr, " has not yet terminated.");
    }
    return ret;
}

/* Override */ int pthread_timedjoin_np(pthread_t thread, void **retval, const struct timespec *abstime)
{
    std::string thstr = stringify(thread);
    debuglog(LCF_THREAD, "Try to join thread ", thstr, " in ", 1000*abstime->tv_sec + abstime->tv_nsec/1000000," ms.");
    int ret = pthread_timedjoin_np_real(thread, retval, abstime);
    if (ret == 0) {
        debuglog(LCF_THREAD, "Joining thread ", thstr, " successfully.");
    }
    if (ret == ETIMEDOUT) {
        debuglog(LCF_THREAD, "Call timed out before thread ", thstr, " terminated.");
    }
    return ret;
}
