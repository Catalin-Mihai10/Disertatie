#ifndef NNLOGGER_HEADER
#define NNLOGGER_HEADER

#include "../../interface/types.h"

#define ENABLE_DEBUG_LOG 0

typedef enum logLevels
{
    NNLOG_FATAL = 0,
    NNLOG_ERROR,
    NNLOG_DEBUG,
    NNLOG_WARN,
    NNLOG_INFO,
    
} logLevels;

static const schar8 *severity[] = 
{
    "FATAL",
    "ERROR",
    "DEBUG",
    "WARN",
    "INFO"
};

static const schar8 *colors[] = 
{
    "\033[95m", // MAGENTA - 0
    "\033[91m", // RED     - 1
    "\033[94m", // BLUE    - 2
    "\033[93m", // YELLOW  - 3
    "\033[92m", // GREEN   - 4
    "\033[96m", // CYAN    - 5
    "\033[97m", // WHITE   - 6
    "\033[90m", // GREY    - 7
    "\033[0m"   // CLEAR   - 8
};

void NNlog(logLevels level, const pSChar8 file, sint32 line, const pSChar8 message, ...); 

#define NNFATAL(message, ...) NNlog(NNLOG_FATAL, __FILE__, __LINE__, message, ##__VA_ARGS__)
#define NNERROR(message, ...) NNlog(NNLOG_ERROR, __FILE__, __LINE__, message, ##__VA_ARGS__)

#if ENABLE_DEBUG_LOG == 1
#define NNDEBUG(message, ...) NNlog(NNLOG_DEBUG, __FILE__, __LINE__, message, ##__VA_ARGS__)
#else
#define NNDEBUG(message, ...) 
#endif

#define NNWARN(message, ...)  NNlog(NNLOG_WARN, __FILE__, __LINE__, message, ##__VA_ARGS__)
#define NNINFO(message, ...)  NNlog(NNLOG_INFO, __FILE__, __LINE__, message, ##__VA_ARGS__)

#endif /* NNLOGER_HEADER */
