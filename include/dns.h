#pragma once

#include <string>
#include <cstdint>
#include <vector>
#include <cstring>
#include <iostream>

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
    uint16_t    QDCOUNT; // количество секций запросов
    uint16_t    ANCOUNT;
    uint16_t    NSCOUNT;
    uint16_t    ARCOUNT;
} DNS_HEADER;

typedef struct 
{
    std::vector<std::string>    QNAME;
    uint16_t                    QTYPE;
    uint16_t                    QCLASS;
} DNS_QUESTION;

typedef struct 
{
    std::vector<std::string>   QNAME;
    uint16_t            QTYPE;
    uint16_t            RDLENGHT;
} DNS_ANSWER;

typedef struct
{
    DNS_HEADER header;
    std::vector<DNS_QUESTION> request;
    DNS_ANSWER answer;
} DNS_DATA;

namespace dns
{
    std::string ToBinary(char* hex, size_t);

    class DnsStruct
    {
    public:
        void ReadData(std::string question);

    public:
        DNS_DATA data;

    private:
        template <typename T>
        void WriteData(T* dest, T datad, size_t data_sizeof)
        {
            memcpy(dest, &datad, sizeof(T));
            fp += data_sizeof;
        }

        void ReadDNSHeader(std::string question);
        void ReadSectionQuestion(std::string question, int cRecord);
        void ReadLine(uint8_t strSize, std::string question, int cRecord);

    private:
        int fp;
    };
}

