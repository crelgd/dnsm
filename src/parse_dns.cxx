#include "dns.h"
#include <iostream>

namespace dns 
{
    std::string byteToBinary(uint16_t byte)
    {
        std::string out;

        out.reserve(8);

        for (int i = 7; i >= 0; i--)
            out.push_back(((byte >> i) & 1) ? '1' : '0');

        return out;
    }

    std::string ToBinary(char* hex, size_t hex_sizeof)
    {
        std::string out;
        size_t size = hex_sizeof / sizeof(hex[0]);

        for (int i = 0; i < size; i++)
        {
            out.append(byteToBinary(hex[i]));
        }

        return out;
    }

    int ToByte(std::string bit, size_t size)
    {
        int result = 0x00;

        for (int i = 0; i < size; i++)
        {
            /* printf("OFFSET = %d\n", size - i - 1); */

            if (bit[i] != '0')
            {
                result += pow(2, size-i - 1);
                /* printf("RESULT += 2^%d = %d | 2^%d = %d\n", size-i-1, result, size-i-1, pow(2, size-i- 1)); */
            }
        }

        return result;
    }

    void DnsStruct::ReadDNSHeader(std::string DNS)
    {
        WriteData(&data.header.ID,      static_cast<uint16_t>(ToByte(DNS, 16)), 16);
        WriteData(&data.header.QR,      static_cast<BIT>(ToByte(&DNS[fp], 1)), 1);
        WriteData(&data.header.Opcode,  static_cast<uint8_t>(ToByte(&DNS[fp], 4)), 4);
        WriteData(&data.header.AA,      static_cast<BIT>(ToByte(&DNS[fp], 1)), 1);
        WriteData(&data.header.TC,      static_cast<BIT>(ToByte(&DNS[fp], 1)), 1);
        WriteData(&data.header.RD,      static_cast<BIT>(ToByte(&DNS[fp], 1)), 1);
        WriteData(&data.header.RA,      static_cast<BIT>(ToByte(&DNS[fp], 1)), 1);
        WriteData(&data.header.Z,       static_cast<uint8_t>(ToByte(&DNS[fp], 3)), 3);
        WriteData(&data.header.RCODE,   static_cast<uint8_t>(ToByte(&DNS[fp], 4)), 4);
        WriteData(&data.header.QDCOUNT, static_cast<uint16_t>(ToByte(&DNS[fp], 16)), 16);
        WriteData(&data.header.ANCOUNT, static_cast<uint16_t>(ToByte(&DNS[fp], 16)), 16);
        WriteData(&data.header.NSCOUNT, static_cast<uint16_t>(ToByte(&DNS[fp], 16)), 16);
        WriteData(&data.header.ARCOUNT, static_cast<uint16_t>(ToByte(&DNS[fp], 16)), 16);
    }

    void DnsStruct::ReadData(std::string DNS)
    {
        fp = 0; // для чтения заголовка

        ReadDNSHeader(DNS);
    }
}

