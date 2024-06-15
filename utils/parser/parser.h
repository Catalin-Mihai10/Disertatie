#ifndef PARSER_HEADER
#define PARSER_HEADER

#include "../../interface/types.h"
#include <pcap/pcap.h>
#include <netinet/in.h>
#include <net/ethernet.h>

/* DATA */
typedef struct tcpString
{
    pSChar8 data;
    uint32  size,
            lines;
} tcpString;
typedef tcpString* pTcpString;

typedef enum protocols
{
    TCP = 0x00,
    UDP = 0x01,
    UNDEFINED = 0x07
} protocols;
typedef protocols* pProtocols;

typedef struct features
{
    pUInt16     sourceIp,
                destinationIp;
    pUInt32     length,
                payloadSize;
    pProtocols  protocol;
} features;
typedef features* pFeatures;

typedef struct cic_dataset
{
    pDouble32   features,
                labels;
    uint32      rows,
                featuresColumns,
                labelsColumns;
}cic_dataset;
typedef cic_dataset* pCicDataset;

/* INITIALIZATION */
void initializeFeaturesArray(void);
pFeatures getFeatures(void);

/* FILE FUNCTION */
uint32 fileReading(const pSChar8 source, pTcpString data);
uint32 fileWriting(const pSChar8 destination, pTcpString data);


/* PCAP */
#define ETHER_HEADER_SIZE 14

typedef pcap_t* pPcap;
typedef const struct pcap_pkthdr* pPcap_pkthdr;
typedef const unsigned char* cpUChar8;
typedef struct ether_header* pEtherHead;

typedef struct pcapPacket
{
   pPcap_pkthdr header;
   pEtherHead   ethHead;
   pUChar8      arguments;
   cpUChar8     packet;
} pcapPacket;
typedef pcapPacket* pPcapPacket;

/* IP HEADER STRUCTURE */
#define IP_RF 0x8000        // reserved fragment flag
#define IP_DF 0x4000        // don't fragment flag
#define IP_MF 0x2000        // more fragments flag
#define IP_OFFMASK 0x1FFF   //mask for fragmenting bits

uint8 ipHeaderLength(uchar8 versionHeaderLength);

uint8 ipVersion(uchar8 versionHeaderLenght);

typedef struct ipHeader
{
    uchar8 versionHeaderLength,
           typeOfService;
    uint16 length,
           id,
           offset;
    uchar8 ttl,
           protocol;
    uint16 checkSum;
    struct in_addr sourceAddress,
                   destinationAddres;

} ipHeader;
typedef ipHeader* pIpHeader;

/* TCP HEADER STRUCTURE */
#define TCP_FIN 0x01
#define TCP_SYN 0x02
#define TCP_RST 0x04
#define TCP_PUSH 0x08
#define TCP_ACK 0x10
#define TCP_URG 0x20
#define TCP_ECE 0x40
#define TCP_CWR 0x80
#define TCP_FLAGS (TCP_FIN|TCP_SYN|TCP_RST|TCP_ACK|TCP_URG|TCP_ECE|TCP_CWR)

uint8 tcpOffset(uchar8 dataOffset);

typedef struct tcpHeader
{
    uint16 sourcePort,
           destinationPort;
    uint32 sequence,
           acknowledge;
    uchar8 dataOffset,
           flags;
    uint16 window,
           checksum,
           urgentPointer;
} tcpHeader;
typedef tcpHeader* pTcpHeader;

/* UDP HEADER STRUCTURE */
typedef struct udpHeader
{
    uint16 sourcePort,
           destinationPort,
           length,
           checksum;
} udpHeader;
typedef udpHeader* pUdpHeader;

void analyzePacket(pPcapPacket packet);
void handlePackets(pUChar8 arguments, pPcap_pkthdr header, cpUChar8 packet);
void parsePcapFile(const pSChar8 source);
void parseCsvData(pTcpString data, pCicDataset dataset);
void parseCsvData_less_features(pTcpString data, pCicDataset dataset);
void parseKDD(pTcpString data, pCicDataset dataset);
void printPacket(pPcapPacket packet);

void normalizeCsvData(pCicDataset dataset, uint32 size);

#endif /* PARSER_HEADER */
