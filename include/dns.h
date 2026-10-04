#pragma once

#include <cstdint>
#include <vector>

#define BIT bool

typedef struct 
{
    uint16_t    ID;
    BIT         QR;
    uint8_t     Opcode;
    BIT         AA;
    BIT         TC;
    BIT         RD;
    BIT         RA;
    uint8_t     Z;
    uint8_t     RCODE;
    uint16_t    QDCOUNT;
    uint16_t    ANCOUNT;
    uint16_t    NSCOUNT;
    uint16_t    ARCOUNT;
} DNS_HEADER;

typedef struct 
{
    std::vector<char>   QNAME;
    uint16_t            QTYPE;
    uint16_t            QCLASS;
} DNS_QUESTION;

typedef struct 
{
    std::vector<char>   QNAME;
    uint16_t            QTYPE;
    uint16_t            RDLENGHT;
} DNS_ANSWER;

typedef struct
{
    DNS_HEADER header;
    DNS_QUESTION request;
    DNS_ANSWER answer;
} DNS_DATA;

namespace dns
{
    class DnsStruct
    {

    }
}