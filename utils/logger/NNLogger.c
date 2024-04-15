#include "../../interface/basic/libraries.h"

#include "NNLogger.h"
#include <fcntl.h>
#include <stdarg.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>

#define MAX_LOG_FILE_NAME 255
#define MAX_LOG_LENGTH_SIZE 2048 

uint8 fileExists(const pSChar8 file)
{
    struct stat status;
    return stat(file, &status);
}

void writeToFile(logLevels level, const pSChar8 fileSource, sint32 line, const pSChar8 message, struct tm* logTimeInfo)
{

    schar8 fileOutput[MAX_LOG_LENGTH_SIZE];
    sint16 formatFileOutputStatus = snprintf(fileOutput, MAX_LOG_LENGTH_SIZE, "%d-%d-%d_%d:%d:%d [%s] %s[%d]: %s\n", logTimeInfo->tm_year + 1900, logTimeInfo->tm_mon + 1, logTimeInfo->tm_mday, 
                                                                                                                     logTimeInfo->tm_hour, logTimeInfo->tm_min, logTimeInfo->tm_sec, severity[level], fileSource, line, message);
                                                                                                                   
    if(formatFileOutputStatus < 0)
    {
        perror("ERROR: Could not format output message for file!\n");
        exit(1);
    }

    schar8 file[MAX_LOG_FILE_NAME];
    sint8 formatFileNameStatus = snprintf(file, MAX_LOG_FILE_NAME, "log_%d_%d_%d", logTimeInfo->tm_year + 1900, logTimeInfo->tm_mon + 1, logTimeInfo->tm_mday);

    if(formatFileNameStatus < 0)
    {
        perror("Could not create log file!\n");
        exit(1);
    }
    
    FILE* fileStream = fopen(file, "a");
    if(fileStream == NULL)
    {
        perror("ERROR: Could not create log file!\n");
        exit(1);
    }

    sint32 writeStatus = fwrite(fileOutput, (sizeof *fileOutput), formatFileOutputStatus, fileStream);
    if(writeStatus == 0)
    {
        perror("ERROR: Could not write into the log file!\n");
        exit(1);
    }
    fflush(fileStream);

    if(fclose(fileStream) != 0)
    {
        perror("ERROR: Could not close the log file!\n");
        exit(1);
    }
}

void NNlog(logLevels level, const pSChar8 file, sint32 line, const pSChar8 message, ...)
{
    schar8 formatedMessage[MAX_LOG_LENGTH_SIZE];
    
    va_list arguments;
    va_start(arguments, message);
    sint32 formatMessageStatus = vsnprintf(formatedMessage, MAX_LOG_LENGTH_SIZE, message, arguments);
    
    if(formatMessageStatus < 0)
    {
        perror("ERROR: Could not build log message!\n");
        exit(1);
    }
    va_end(arguments);
    
    time_t logTime;
    if(time(&logTime) == -1)
    {
        perror("ERROR: Could not get calendar time info!\n");
        exit(1);
    }

    struct tm *logTimeInfo = localtime(&logTime);
    if(logTimeInfo == NULL)
    {
        perror("ERROR: Could not get time of log message!\n");
        exit(1);
    }

    schar8 logOutput[MAX_LOG_LENGTH_SIZE];
    sint16 formatOutputStatus = snprintf(logOutput, MAX_LOG_LENGTH_SIZE, "%s%d:%d:%d%s %s[%s]%s %s%s%s[%s%d%s]: %s%s%s\n", colors[5], logTimeInfo->tm_hour, logTimeInfo->tm_min, logTimeInfo->tm_sec, colors[8],
                                                                                                                        colors[level], severity[level], colors[8], colors[7], file, colors[8], colors[5], line, colors[8],
                                                                                                                        colors[level], formatedMessage, colors[8]);
    if(formatOutputStatus < 0)
    {
        perror("ERROR: Could not format output log message!\n");
        exit(1);
    }

    printf("%s", logOutput);
    fflush(stdout);
    writeToFile(level, file, line, formatedMessage, logTimeInfo);
}

/*
int main()
{
    NNINFO("This is a INFO log type message! - %d", 2);
    NNWARN("This is a WARN log type message! - %d", 2);
    NNDEBUG("This is a DEBUG log type message! - %d", 2);
    NNERROR("This is a ERROR log type message! - %d", 2);
    NNFATAL("This is a FATAL log type message!");
    
    return 0;
}
*/
