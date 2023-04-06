// ...
// Placeholder for necessary preambles and includes:

#include <pthread.h>
#include <sys/prctl.h>
#include <sys/syscall.h>

#include <ios>
#include <string>
#include <thread>

// ...

#define gettid() syscall(SYS_gettid)

// Other placeholders.
// ... 


/***********************************************************************
* Set the thread name within the kernel so GDB, "ps", "top" and other 
* system commands and tools can see and display those user-specified 
* names.
*
* @param[in] threadId - Opaque thread ID object specifying the thread 
*                       whose name is to be changed.
* @param[in] name     - the new name to be given to the thread. Will be 
*                       truncated if greater than 15 characters.
*
* @return   None. However logs with errno are created on error(s).
*
* @note     By default, all threads created using pthread_create() 
*           inherit the program name. The SetThreadName() function can 
*           be used to set a unique name for a thread, which can be 
*           useful for debugging multi-threaded applications. The thread
*           name is a meaningful C-language string, whose length is 
*           restricted to 16 characters, including the terminating null
*           byte ('\0').
*
* @warning
***********************************************************************/
inline void SetThreadName(pthread_t threadId, const char * name)
{
    if ('\0' != name[0])
    {
        std::string name_str(name, (size_t)((strlen(name) < (THREAD_NAME_LENGTH - 1)) ? strlen(name) : (THREAD_NAME_LENGTH - 1)));

        int retval = -1;

        // Can return (34) Invalid Directory or Filename when /proc/[pid]/task/[tid]/comm doesn't exist.
        // Note that this error is harmless as this is only a debugging api.
        retval = pthread_setname_np(threadId, name);
        if (0 != retval)
        {
            NonInterspersedLog<WarnLog_t>("Thread name set failed! name, errno =", 
                name_str.c_str(), strerror(retval));
        }
    }
    else
    {
        NonInterspersedLog<ErrorLog_t>("Requested thread name is invalid!");
    }
}

/***********************************************************************
* Retrieve the thread name within the kernel.
*
* @param[in] threadId - Opaque thread ID object specifying the thread 
*                       whose name is to be retrieved.
* @param[in] name     - Buffer used to return the thread name.
* @param[in] length   - Specifies the number of bytes available in the 
*                       buffer.
*
* @return   None. However logs with errno are created on error(s).
*
* @note     The buffer specified by name should be at least 16 characters
*           in length. The returned thread name in the output buffer will
*           be null terminated.
*
* @warning
***********************************************************************/
inline void GetThreadName(pthread_t threadId, char * name, size_t length)
{
    if (THREAD_NAME_LENGTH <= length)
    {
        int retval = -1;

        retval = pthread_getname_np(threadId, name, length);
        if (0 != retval)
        {
            NonInterspersedLog<WarnLog_t>("Thread name get failed! errno =", 
                strerror(errno));
        }
    }
    else
    {
        NonInterspersedLog<ErrorLog_t>("Buffer length insufficient!");
    }
}

inline void SetThreadName(std::thread * thread, const char * name)
{
    if (thread)
    {
        auto handle = thread->native_handle();
        SetThreadName(handle, name);
    }
}

inline void GetThreadName(std::thread * thread, char * name, size_t length)
{
    if (thread)
    {
        auto handle = thread->native_handle();
        GetThreadName(handle, name, length);
    }
}

inline void SetThreadName(const char * name)
{
    if (gettid() == getpid())
    {
        NonInterspersedLog<WarnLog_t>("Main thread should NOT be renamed! Tools like killall will cease working otherwise.");
        return;
    }

    if ('\0' != name[0])
    {
        std::string name_str(name, (size_t)((strlen(name) < (THREAD_NAME_LENGTH - 1)) ? strlen(name) : (THREAD_NAME_LENGTH - 1)));

        int retval = 0;

        retval = prctl(PR_SET_NAME, name_str.c_str(), NULL, NULL, NULL);
        if (-1 == retval)
        {
            NonInterspersedLog<WarnLog_t>("Thread name set failed! name, errno =", 
                name_str.c_str(), strerror(errno));
        }
    }
    else
    {
        NonInterspersedLog<ErrorLog_t>("Requested thread name invalid!");
    }
}

inline void GetThreadName(char * name, size_t length)
{
    if (THREAD_NAME_LENGTH <= length)
    {
        int retval = 0;

        retval = prctl(PR_GET_NAME, name);
        if (-1 == retval)
        {
            std::ostringstream oss;
            oss << __PRETTY_FUNCTION__;
            oss << ": Warning! Thread name get failed. Errno = ";
            oss << strerror(errno);
            
            // Ensure that even the newlines are properly output without
            // being incorrectly interspersed.
            std::string logString = oss.str() + "\n";
            std::cout << logString;
        }
    }
    else
    {
        std::ostringstream oss; 
        oss << __PRETTY_FUNCTION__;
        oss << ": Error! Insufficient buffer length.";

        // Ensure that even the newlines are properly output without
        // being incorrectly interspersed.
        std::string logString = oss.str() + "\n";
        std::cout << logString;
    } 
}
