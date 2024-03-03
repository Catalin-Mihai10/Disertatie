#include "dataReader.hpp"
#include "../../interface/basicLibraries.hpp"
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>

uint32 getFileSize(const pSChar8 source)
{
    struct stat status;
    stat(source, &status);
    
    return status.st_size;
}

uint32 readData(const pSChar8 source, pSChar8 data)
{
    uint32 sourceSize = getFileSize(source);
    uint32 descriptor = open(source, O_RDONLY);

    if(descriptor < 0)
    {
        printf("ERROR: Cannot open source file!\n");
        exit(1);
    }
    
    data = (pSChar8) malloc((sizeof *data) * sourceSize);

    if(!data)
    {
        printf("ERROR: Cannot read source file!\n");
        exit(1);
    }

    uint32 readResult = read(descriptor, data, sourceSize);
    close(descriptor);

    if(readResult < 0)
    {
        printf("ERROR: Could not read source data!\n");
        exit(1);
    }
    
    return readResult;
}

void writeData(pSChar8 destination, pSChar8 data, uint32 size)
{
    uint32 desDescriptor = open(destination, O_CREAT | O_WRONLY);

    if(desDescriptor < 0)
    {
        printf("ERROR: Cannot open output file!\n");
        exit(1);
    }

    uint32 writeResult = write(desDescriptor, data, size);
    
    close(desDescriptor);

    if(writeResult < 0)
    {
        printf("ERROR: Could not write data into file!\n");
    }
}

void parseSource(const pSChar8 source, const pSChar8 destination)
{
    pSChar8 data;
    uint32 readBytes = readData(source, data);

    // TODO: Parse the data and tokenize.

    writeData(destination, data, readBytes);
}

int main()
{    
    const pSChar8 source = "/home/catalin/DARPA/monday/bsm.list";
    const pSChar8 des = "/home/catalin/Disertatie/output.txt";
    
    parseSource(source, des);
    
    return 0;
}
