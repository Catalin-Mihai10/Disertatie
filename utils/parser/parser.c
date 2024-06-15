#include "parser.h"
#include "../../interface/basic/libraries.h"
#include "../../interface/constants.h"
#include <fcntl.h>
#include <net/ethernet.h>
#include <netinet/in.h>
#include <unistd.h>
#include <sys/stat.h>
#include <math.h>

#include <pcap.h>

#define MB 1048576

pFeatures dataFeatures = NULL;
uint32    dataCounter;
static uint32 const initialArraySize = 10 * MB;
static uint32 actualSize      = ZERO,
              maxArraySize = initialArraySize;

uint8 ipHeaderLength(uchar8 versionHeaderLength)
{
    return versionHeaderLength & 0x0F;
}

uint8 ipVersion(uchar8 versionHeaderLenght)
{
    return versionHeaderLenght >> 4;
}

uint8 tcpOffset(uchar8 dataOffset)
{
    return (dataOffset & 0x0F) >> 4;
}

void allocateFeatures(pFeatures features, uint32 arraySize)
{
    features->sourceIp      = malloc((sizeof *features->sourceIp) * arraySize);
    features->destinationIp = malloc((sizeof *features->destinationIp) * arraySize);
    features->length        = malloc((sizeof *features->length) * arraySize);
    features->payloadSize   = malloc((sizeof *features->payloadSize) * arraySize);
    features->protocol      = malloc((sizeof *features->protocol) * arraySize);

    if((features->sourceIp == NULL) || (features->destinationIp == NULL) ||
       (features->length == NULL) || (features->payloadSize == NULL) || (features->protocol == NULL))
    {
        printf("ERROR: Could not allocate memory!\n");
        return;
    }
}

void freeFeatures(pFeatures features)
{
    free(features->sourceIp);
    free(features->destinationIp);
    free(features->length);
    free(features->payloadSize);
    free(features->protocol);

    features->sourceIp      = NULL;
    features->destinationIp = NULL;
    features->length        = NULL;
    features->payloadSize   = NULL;
    features->protocol      = NULL;
}

void copyFeaturesToTemp(pFeatures temp, pFeatures array)
{
    for(uint32 iterator = ZERO; iterator < actualSize; ++iterator)
    {
        temp->sourceIp[iterator] = array->sourceIp[iterator];
        temp->destinationIp[iterator] = array->destinationIp[iterator];
        temp->length[iterator] = array->length[iterator];
        temp->payloadSize[iterator] = array->payloadSize[iterator];
        temp->protocol[iterator] = array->protocol[iterator];
    }
}

void reallocateFeatures(void)
{
    maxArraySize = actualSize * 2;
    pFeatures temp = malloc((sizeof *temp));
    if(temp == NULL)
    {
        printf("ERROR: Could not resize array!\n");
        return;
    }
    
    allocateFeatures(temp, maxArraySize);
    copyFeaturesToTemp(temp, dataFeatures);
    freeFeatures(dataFeatures);
    dataFeatures = temp;
}

pFeatures getFeatures(void)
{
    return dataFeatures;
}

uint32 getFileSize(const pSChar8 source)
{
    struct stat status;
    stat(source, &status);
    
    return status.st_size;
}

uint32 getNumberOfLines(const pTcpString data)
{
    pSChar8 temp;
    temp = malloc((sizeof *data->data) * data->size);
    
    strcpy(temp, data->data);

    pSChar8 line = strtok(temp, "\n");
    uint32 linesCount = (uint32)ZERO;
    while(line)
    {
        ++linesCount;
        line = strtok(NULL, "\n");
    }
    
    free(temp);
    temp = NULL;
    return linesCount;
}

uint32 fileReading(const pSChar8 source, pTcpString data)
{
    uint32 readResult = READ_ERROR;
    uint32 sourceSize = getFileSize(source);
    uint32 descriptor = open(source, O_RDONLY);

    if(descriptor < 0)
    {
        printf("ERROR: Cannot open source file!\n");
        return readResult;
    }
    data->data = (pSChar8) malloc(((sizeof *data->data) * sourceSize) + 1);
    if(data->data == NULL) return readResult;
 
    
    readResult = read(descriptor, data->data, sourceSize);
    data->size = readResult; 
    data->lines = getNumberOfLines(data);
    
    close(descriptor);

    if(readResult < 0)
    {
        readResult = READ_ERROR;
    }
    
    return readResult;
}

uint32 fileWriting(pSChar8 destination, pTcpString data)
{
    uint32 writeResult = WRITE_ERROR;
    uint32 desDescriptor = open(destination, O_CREAT | O_RDWR);
    
    if(desDescriptor < 0) return writeResult;

    writeResult = write(desDescriptor, data->data, data->size);
    
    close(desDescriptor);

    if(writeResult < 0)
    {
        writeResult = WRITE_ERROR;
    }
    
    return writeResult;
}

void analyzePacket(pPcapPacket pcapPacket)
{
    uint16 type = ntohs(pcapPacket->ethHead->ether_type);
    if(type != ETHERTYPE_IP) return;

    pIpHeader ip = (pIpHeader) (pcapPacket->packet + ETHER_HEADER_SIZE);
    uint32 ipSize = ipHeaderLength(ip->versionHeaderLength) * 4;

    if(ipSize < 20)
    {
        printf("ERROR: Invalid IP header length: %u bytes", ipSize);
        return;
    }

    uint32 tcpSize = 0,
           udpSize = 0;
    
    if(actualSize >= initialArraySize) reallocateFeatures();

    if(ip->protocol == IPPROTO_TCP)
    {
        pTcpHeader tcp = (pTcpHeader) (pcapPacket->packet + ETHER_HEADER_SIZE + ipSize);
        tcpSize = tcpOffset(tcp->dataOffset) * 4;
        
        dataFeatures->protocol[actualSize]      = TCP;
        dataFeatures->sourceIp[actualSize]      = tcp->sourcePort;
        dataFeatures->destinationIp[actualSize] = tcp->destinationPort;
        dataFeatures->length[actualSize]        = pcapPacket->header->len;
        dataFeatures->payloadSize[actualSize]   = pcapPacket->header->caplen - (ETHER_HEADER_SIZE + ipSize + tcpSize);
        ++actualSize;
    } else if(ip->protocol != IPPROTO_UDP)
    {
        pUdpHeader udp = (pUdpHeader) (pcapPacket->packet + ETHER_HEADER_SIZE + ipSize);
        udpSize = 8;
       
        dataFeatures->protocol[actualSize]      = UDP;
        dataFeatures->sourceIp[actualSize]      = udp->sourcePort;
        dataFeatures->destinationIp[actualSize] = udp->destinationPort;
        dataFeatures->length[actualSize]        = pcapPacket->header->len;
        dataFeatures->payloadSize[actualSize]   = pcapPacket->header->caplen - (ETHER_HEADER_SIZE + ipSize + udpSize);
        ++actualSize;
    } else return;
}

void handlePackets(pUChar8 arguments, pPcap_pkthdr header, cpUChar8 packet)
{
    pcapPacket pcapPacket;
    pcapPacket.header = header;
    pcapPacket.ethHead = (pEtherHead) packet;
    pcapPacket.arguments = arguments;
    pcapPacket.packet = packet;
    
    analyzePacket(&pcapPacket);
    //printPacket(&pcapPacket);
    return;
}

void printPacket(pPcapPacket pcapPacket)
{
    printf("Packet capture length: %d\nPacket Total Length: %d\n", pcapPacket->header->caplen, pcapPacket->header->len);
}

void parsePcapFile(const pSChar8 source)
{
    schar8 errorBuffer[PCAP_ERRBUF_SIZE];
    pPcap handle = pcap_open_offline(source, errorBuffer); 

    if(handle == NULL)
    {
        printf("ERROR: Cannot open capture file!\n");
        return;
    }   

    pcap_loop(handle, 0, handlePackets, NULL);
    pcap_close(handle);
}

void testArrayResizing(void)
{
    for(uint32 iterator = ZERO; iterator < 20000; ++iterator)
    {
        if(actualSize >= maxArraySize)
        {
            reallocateFeatures();
            printf("reallocated array!\n");
            
            for(uint32 iterator = ZERO; iterator < maxArraySize; ++iterator)
            {
              printf("%d %d %d %d %d\n", dataFeatures->sourceIp[iterator], dataFeatures->destinationIp[iterator],
                                       dataFeatures->length[iterator], dataFeatures->payloadSize[iterator],
                                       dataFeatures->protocol[iterator]);
            }
        }
        printf("%d\n", actualSize);
        dataFeatures->sourceIp[actualSize] = iterator;
        dataFeatures->destinationIp[actualSize] = iterator;
        dataFeatures->length[actualSize] = iterator;
        dataFeatures->payloadSize[actualSize] = iterator;
        dataFeatures->protocol[actualSize] = iterator % 1;
        ++actualSize;
    }
}

void initializeFeaturesArray(void)
{
    dataFeatures = malloc((sizeof *dataFeatures));
    allocateFeatures(dataFeatures, maxArraySize);
}

void parseCsvData(pTcpString data, pCicDataset dataset)
{
    //TODO: make assigning of columns and rows dynamic.
    dataset->features = malloc((sizeof *dataset->features) * data->lines * 60);
    dataset->featuresColumns = 60;
    dataset->labels = malloc((sizeof *dataset->labels) * data->lines * 2);
    dataset->labelsColumns = 2;
    dataset->rows = data->lines;
    memset(dataset->labels, 0x00, (sizeof *dataset->labels) * data->lines * 2);

    pSChar8 tempLine, tempElement;
    pSChar8 line = strtok_r(data->data, "\n", &tempLine);
    uint32 count = 0,
          elemCount = 0,
          elemCountGlobal = 0,
          firstLine = 0,
          ipCount = 0;

    pSChar8 *ip = malloc((sizeof *ip) * 10000);
    for(uint32 ipt = 0; ipt < 10000; ++ipt)
    {
        ip[ipt] = malloc((sizeof **ip) * 200);
    }
    while(line != NULL)
    {
        if(firstLine != 0)
        {
            pSChar8 element = strtok_r(line, ",\n", &tempElement);
            while(element != NULL)
            {
                //printf("element before parsing: %s -- %d\n", element, elemCountGlobal);
                switch(elemCountGlobal)
                {
                    case 2:
                    case 4:
                    {
                        //printf("element: %s\n", element);
                        sint32 index = -1;
                        for(uint32 it = 0; it < ipCount; ++it)
                        {
                            //printf("compare: %s with %s\n", ip[it], element);
                            if(strcmp(ip[it], element) == 0)
                            {
                                index = it;
                                //printf("entered after compare, index now: %d\n", index);
                                break;
                            }
                        }
                        
                        if(index == -1)
                        {
                            //printf("add new ip:%s  -- ipCount: %u\n", element, ipCount);
                            strcpy(ip[ipCount], element);
                            ++ipCount;
                            index = ipCount;
                        }
                        dataset->features[(count * 60) + elemCount] = index;
                        //printf("value of: %lf - at index: %d\n", dataset->features[(count * 60) + elemCount], (count * 60) + elemCount);
                        ++elemCount;
                        ++elemCountGlobal;
                    }break;
                    case 0:
                    case 1:
                    case 37:
                    case 38:
                    case 39:
                    case 40:
                    case 50:
                    case 51:
                    case 52:
                    case 53:
                    case 54:
                    case 55:
                    case 56:
                    case 57:
                    case 58:
                    case 63:
                    case 64:
                    case 65:
                    case 66:
                    case 67:
                    case 68:
                    case 73:
                    case 74:
                    case 75:
                    case 76:
                    case 85:
                    case 86:
                    {
                        //printf("skip: %d\n", elemCountGlobal);
                        ++elemCountGlobal;
                    }break;
                    case 87:
                    {
                        uint8 index;
                        if((strcmp(element, "BENIGN") == 0))
                        {
                            index = 0;
                        }
                        else if((strcmp(element, "UDP") == 0) || (strcmp(element, "Syn") == 0) || (strcmp(element, "Portmap") == 0) ||
                           (strcmp(element, "NetBIOS") == 0) || (strcmp(element, "MSSQL") == 0) || (strcmp(element, "LDAP") == 0) || (strcmp(element, "TFTP") == 0))
                        {
                            index = 1;
                        }
                        else if((strcmp(element, "DrDoS_DNS") == 0) || (strcmp(element, "DrDoS_LDAP") == 0) || (strcmp(element, "DrDoS_MSSQL") == 0) ||
                           (strcmp(element, "DrDoS_NetBIOS") == 0) || (strcmp(element, "DrDoS_NTP") == 0) || (strcmp(element, "DrDoS_SNMP") == 0) ||
                           (strcmp(element, "DrDoS_SSDP") == 0) || (strcmp(element, "DrDoS_UDP") == 0) || (strcmp(element, "UDPLag") == 0))
                        {
                            index = 1;
                        }
                        
                        for(uint8 iterator = 0; iterator < 2; ++iterator)
                        {
                            if(index == iterator)
                            {
                                dataset->labels[(count * 2) + iterator] = 1.00;
                            }
                        }
                        elemCount = 0;
                        elemCountGlobal = 0;
                    }break;
                    default:
                    {
                        //printf("count: %d -- elemCount: %d\n", count, elemCount);
                        dataset->features[(count * 60) + elemCount] = atof(element);
                        ++elemCount;
                        ++elemCountGlobal;
                    }
                }
                
               // printf("%s - %u\n", element, count);
                //++count;
                //++elemCountGlobal;
                element = strtok_r(NULL, ",\n", &tempElement);
                
            }
            elemCount = 0;
            elemCountGlobal = 0;
            ++count;
         }
        //printf("Number of features: %d", count);
        firstLine = 1;
        line = strtok_r(NULL, "\n", &tempLine);
    }

    for(uint32 ips = 0; ips < 10000; ++ips)
    {
        free(ip[ips]);
        ip[ips] = NULL;
    }
    free(ip);
    ip = NULL;
}

void normalizeCsvData(pCicDataset dataset, uint32 size)
{
    double32 maxValue[60], minValue[60], epsilon = 1e-6;
    for(uint32 column = 0; column < dataset->featuresColumns; ++column)
    {
        maxValue[column] = dataset->features[column];
        minValue[column] = 0xFFFFFF;
        for(uint32 line = (uint32)ONE; line < size; ++line)
        {
            if(maxValue[column] < dataset->features[(line * dataset->featuresColumns) + column])
            {
                maxValue[column] = dataset->features[(line * dataset->featuresColumns) + column];
            }
            if(minValue[column] > dataset->features[(line * dataset->featuresColumns) + column])
            {
                minValue[column] = dataset->features[(line * dataset->featuresColumns) + column];
            }
        }
    }

    for(uint32 line = (uint32)ZERO; line < size; ++line)
    {
        for(uint32 column = (uint32)ZERO; column < dataset->featuresColumns; ++column)
        {
            double32 numerator = fabs(dataset->features[(line * dataset->featuresColumns) + column] - minValue[column]),
                     denominator = fabs(maxValue[column] - minValue[column]);
            if((numerator != 0) && (denominator != 0))
            {
                dataset->features[(line * dataset->featuresColumns) + column] = numerator / denominator; 
            }else if((numerator != 0) && (denominator == 0)){
                dataset->features[(line * dataset->featuresColumns) + column] = numerator;
            }else {
                dataset->features[(line * dataset->featuresColumns) + column] = epsilon;
            }
        }
    }
    
    for(uint32 line = (uint32)ZERO; line < size; ++line)
    {
        for(uint32 column = (uint32)ZERO; column < dataset->featuresColumns; ++column)
        {
            if(isnan(dataset->features[(line * dataset->featuresColumns) + column]))
            {
                dataset->features[(line * dataset->featuresColumns) + column] = epsilon;
            }else if(isinf(dataset->features[(line * dataset->featuresColumns) + column]))
            {
                dataset->features[(line * dataset->featuresColumns) + column] = ONE;
            }
        }
    }
 
}

void parseKDD(pTcpString data, pCicDataset dataset)
{
    //TODO: make assigning of columns and rows dynamic.
    dataset->features = malloc((sizeof *dataset->features) * data->lines * 41);
    dataset->featuresColumns = 41;
    dataset->labels = malloc((sizeof *dataset->labels) * data->lines * 2);
    dataset->labelsColumns = 2;
    dataset->rows = data->lines;
    memset(dataset->labels, 0x00, (sizeof *dataset->labels) * data->lines * 2);

    pSChar8 tempLine, tempElement;
    pSChar8 line = strtok_r(data->data, "\n", &tempLine);
    uint32 count = 0,
          elemCount = 0,
          elemCountGlobal = 0,
          firstLine = 0,
          ipCount = 0;

    pSChar8 *ip = malloc((sizeof *ip) * 10000);
    for(uint32 ipt = 0; ipt < 10000; ++ipt)
    {
        ip[ipt] = malloc((sizeof **ip) * 200);
    }
    while(line != NULL)
    {
        if(firstLine != 0)
        {
            pSChar8 element = strtok_r(line, ",\n", &tempElement);
            while(element != NULL)
            {
                //printf("element before parsing: %s -- %d\n", element, elemCountGlobal);
                switch(elemCountGlobal)
                {
                    case 0:
                    case 4:
                    case 5:
                    case 22:
                    case 23:
                    case 31:
                    case 32:
                    {
                        dataset->features[(count * dataset->featuresColumns) + elemCount] = atof(element);
                        ++elemCount;
                        ++elemCountGlobal; 
                    } break;
                    case 1:
                    case 2:
                    case 3:
                    {
                        //printf("element: %s\n", element);
                        sint32 index = -1;
                        for(uint32 it = 0; it < ipCount; ++it)
                        {
                            //printf("compare: %s with %s\n", ip[it], element);
                            if(strcmp(ip[it], element) == 0)
                            {
                                index = it;
                                //printf("entered after compare, index now: %d\n", index);
                                break;
                            }
                        }
                        
                        if(index == -1)
                        {
                            //printf("add new ip:%s  -- ipCount: %u\n", element, ipCount);
                            strcpy(ip[ipCount], element);
                            ++ipCount;
                            index = ipCount;
                        }
                        dataset->features[(count * dataset->featuresColumns) + elemCount] = index;
                        //printf("value of: %lf - at index: %d\n", dataset->features[(count * 10) + elemCount], (count * 10) + elemCount);
                        ++elemCount;
                        ++elemCountGlobal;
                    } break;
                    case 41:
                    {
                        uint8 index;
                        if((strcmp(element, "normal") == 0))
                        {
                            index = 0;
                        }else {
                            index = 1;
                        }
                        
                        for(uint8 iterator = 0; iterator < 2; ++iterator)
                        {
                            if(index == iterator)
                            {
                                dataset->labels[(count * 2) + iterator] = 1.00;
                            }
                        }
                        elemCount = 0;
                        elemCountGlobal = 0;
                    }break;
                    default:
                    {
                        //printf("count: %d -- elemCount: %d\n", count, elemCount);
                        ++elemCountGlobal;
                    }
                }
            
                //printf("%s - %u\n", element, count);
                //++count;
                //++elemCountGlobal;
                element = strtok_r(NULL, ",\n", &tempElement);
                
            }
            elemCount = 0;
            elemCountGlobal = 0;
            ++count;
         }
        //printf("Number of features: %d", count);
        //if(firstLine == 1) break;

        firstLine = 1;
    
        line = strtok_r(NULL, "\n", &tempLine);
    }

    for(uint32 ips = 0; ips < 10000; ++ips)
    {
        free(ip[ips]);
        ip[ips] = NULL;
    }
    free(ip);
    ip = NULL;

}

void parseCsvData_less_features(pTcpString data, pCicDataset dataset)
{
    //TODO: make assigning of columns and rows dynamic.
    dataset->features = malloc((sizeof *dataset->features) * data->lines * 10);
    dataset->featuresColumns = 10;
    dataset->labels = malloc((sizeof *dataset->labels) * data->lines * 2);
    dataset->labelsColumns = 2;
    dataset->rows = data->lines;
    memset(dataset->labels, 0x00, (sizeof *dataset->labels) * data->lines * 2);

    pSChar8 tempLine, tempElement;
    pSChar8 line = strtok_r(data->data, "\n", &tempLine);
    uint32 count = 0,
          elemCount = 0,
          elemCountGlobal = 0,
          firstLine = 0,
          ipCount = 0;

    pSChar8 *ip = malloc((sizeof *ip) * 10000);
    for(uint32 ipt = 0; ipt < 10000; ++ipt)
    {
        ip[ipt] = malloc((sizeof **ip) * 200);
    }
    while(line != NULL)
    {
        if(firstLine != 0)
        {
            pSChar8 element = strtok_r(line, ",\n", &tempElement);
            while(element != NULL)
            {
                //printf("element before parsing: %s -- %d\n", element, elemCountGlobal);
                switch(elemCountGlobal)
                {
                    case 3:
                    case 5:
                    case 6:
                    case 7:
                    case 8:
                    case 15:
                    case 19:
                    case 9:
                    {
                        //printf("count: %d -- elemCount: %d\n", count, elemCount);
                        dataset->features[(count * 10) + elemCount] = atof(element);
                        ++elemCount;
                        ++elemCountGlobal;
                    } break;
                    case 2:
                    case 4:
                    {
                        //printf("element: %s\n", element);
                        sint32 index = -1;
                        for(uint32 it = 0; it < ipCount; ++it)
                        {
                            //printf("compare: %s with %s\n", ip[it], element);
                            if(strcmp(ip[it], element) == 0)
                            {
                                index = it;
                                //printf("entered after compare, index now: %d\n", index);
                                break;
                            }
                        }
                        
                        if(index == -1)
                        {
                            //printf("add new ip:%s  -- ipCount: %u\n", element, ipCount);
                            strcpy(ip[ipCount], element);
                            ++ipCount;
                            index = ipCount;
                        }
                        dataset->features[(count * 10) + elemCount] = index;
                        //printf("value of: %lf - at index: %d\n", dataset->features[(count * 10) + elemCount], (count * 10) + elemCount);
                        ++elemCount;
                        ++elemCountGlobal;
                    }break;
                    case 87:
                    {
                        uint8 index;
                        if((strcmp(element, "BENIGN") == 0))
                        {
                            index = 0;
                        }
                        else if((strcmp(element, "UDP") == 0) || (strcmp(element, "Syn") == 0) || (strcmp(element, "Portmap") == 0) ||
                           (strcmp(element, "NetBIOS") == 0) || (strcmp(element, "MSSQL") == 0) || (strcmp(element, "LDAP") == 0) || (strcmp(element, "TFTP") == 0))
                        {
                            index = 1;
                        }
                        else if((strcmp(element, "DrDoS_DNS") == 0) || (strcmp(element, "DrDoS_LDAP") == 0) || (strcmp(element, "DrDoS_MSSQL") == 0) ||
                           (strcmp(element, "DrDoS_NetBIOS") == 0) || (strcmp(element, "DrDoS_NTP") == 0) || (strcmp(element, "DrDoS_SNMP") == 0) ||
                           (strcmp(element, "DrDoS_SSDP") == 0) || (strcmp(element, "DrDoS_UDP") == 0) || (strcmp(element, "UDPLag") == 0))
                        {
                            index = 1;
                        }
                        
                        for(uint8 iterator = 0; iterator < 2; ++iterator)
                        {
                            if(index == iterator)
                            {
                                dataset->labels[(count * 2) + iterator] = 1.00;
                            }
                        }
                        elemCount = 0;
                        elemCountGlobal = 0;
                    }break;
                    default:
                    {
                        ++elemCountGlobal;
                    }
                }
            
                //printf("%s - %u\n", element, count);
                //++count;
                //++elemCountGlobal;
                element = strtok_r(NULL, ",\n", &tempElement);
                
            }
            elemCount = 0;
            elemCountGlobal = 0;
            ++count;
         }
        //printf("Number of features: %d", count);
        firstLine = 1;
        line = strtok_r(NULL, "\n", &tempLine);
    }

    for(uint32 ips = 0; ips < 10000; ++ips)
    {
        free(ip[ips]);
        ip[ips] = NULL;
    }
    free(ip);
    ip = NULL;
}
/*
int main(void)
{    
    const pSChar8 source = "/home/catalin/Downloads/KDDCup99.csv";
    //const pSChar8 des = "/home/catalin/Disertatie/output.txt";
   
    tcpString data;
    data.data = NULL;
    data.size = ZERO;
    cic_dataset cic;
    uint32 readBytes = fileReading(source, &data);
    if(readBytes == READ_ERROR)
    {
        printf("ERROR: Could not read source data!\n");
    }
    
    //printf("size: %lfMB - lines: %u\n", (double32)data.size / (1024*1024), data.lines);
    //parseCsvData_less_features(&data, &cic);   
    parseKDD(&data, &cic);
    for(uint32 it = 0; it < data.lines; ++it)
    {
        for(uint32 it2 = 0; it2 < 41; ++it2)
        {
            printf("%lf ", cic.features[(it * 41) + it2]);
        }
        printf("\n\n\n");
    }
    
    normalizeCsvData(&cic, data.lines);
   for(uint32 line = 0; line < data.lines; ++line)
    {
        for(uint32 column = 0; column < 41; ++column)
        {
            assert(!isnan(cic.features[(line * 41) + column]));
        }
    }
    

    for(uint32 it = 0; it < data.lines; ++it)
    {
        for(uint32 it2 = 0; it2 < 41; ++it2)
        {
            printf("%lf ", cic.features[(it * 41) + it2]);
        }
        printf("\n\n\n");
    }
    
    //uint32 writeBytes = fileWriting(des, &data);
    //if(writeBytes == WRITE_ERROR)
    //{
     //   printf("ERROR: Could not write data into file!\n");
    //}
    
    free(data.data);
    data.data = NULL;
    free(cic.features);
    cic.features = NULL;
    free(cic.labels);
    cic.labels = NULL;
    //free(source);
    //free(des);
    return 0;
}
*/
