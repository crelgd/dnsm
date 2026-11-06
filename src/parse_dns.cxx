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

    void DnsStruct::ReadDNSHeader(std::string question)
    {
        WriteData(&data.header.ID,      static_cast<uint16_t>(ToByte(question, 16)), 16);
        WriteData(&data.header.QR,      static_cast<BIT>(ToByte(&question[fp], 1)), 1);
        WriteData(&data.header.Opcode,  static_cast<uint8_t>(ToByte(&question[fp], 4)), 4);
        WriteData(&data.header.AA,      static_cast<BIT>(ToByte(&question[fp], 1)), 1);
        WriteData(&data.header.TC,      static_cast<BIT>(ToByte(&question[fp], 1)), 1);
        WriteData(&data.header.RD,      static_cast<BIT>(ToByte(&question[fp], 1)), 1);
        WriteData(&data.header.RA,      static_cast<BIT>(ToByte(&question[fp], 1)), 1);
        WriteData(&data.header.Z,       static_cast<uint8_t>(ToByte(&question[fp], 3)), 3);
        WriteData(&data.header.RCODE,   static_cast<uint8_t>(ToByte(&question[fp], 4)), 4);
        WriteData(&data.header.QDCOUNT, static_cast<uint16_t>(ToByte(&question[fp], 16)), 16);
        WriteData(&data.header.ANCOUNT, static_cast<uint16_t>(ToByte(&question[fp], 16)), 16);
        WriteData(&data.header.NSCOUNT, static_cast<uint16_t>(ToByte(&question[fp], 16)), 16);
        WriteData(&data.header.ARCOUNT, static_cast<uint16_t>(ToByte(&question[fp], 16)), 16);
    }

    void DnsStruct::ReadLine(uint8_t strSize, std::string question, int cRecord)
    {
        std::string res;

        for (int i = 0; i < strSize; i++)
        {
            char gByte = 0;

            // for (int j = 0; j < 8; j++)
            // {
            //     std::cout << question[j+fp];
            // }
            // std::cout << std::endl;

            WriteData(&gByte, static_cast<char>(ToByte(&question[fp], 8)), 8);

            res += gByte;
        }

        data.request[cRecord].QNAME.push_back(res);
    }

    void DnsStruct::ReadSectionQuestion(std::string question, int cRecord)
    {
        int static_fp = fp;
        int ic = question.size() - static_fp;

        for (int i = 0; i < ic; i++)
        {
            uint8_t label = 0;
            WriteData(&label, static_cast<uint8_t>(ToByte(&question[fp], 2)), 2);

            if (label == 0)
            {
                uint8_t strSize = 0;
                WriteData(&strSize, static_cast<uint8_t>(ToByte(&question[fp], 6)), 6);

                if (strSize == 0)
                {
                    break;
                }

                ReadLine(strSize, question, cRecord); // запись в структуру
            }
        }

        WriteData(&data.request[cRecord].QTYPE, static_cast<uint16_t>(ToByte(&question[fp], 16)), 16);
        WriteData(&data.request[cRecord].QCLASS, static_cast<uint16_t>(ToByte(&question[fp], 16)), 16);
    }

    void DnsStruct::ReadData(std::string question)
    {
        fp = 0; // для чтения заголовка

        ReadDNSHeader(question);

        data.request.resize(data.header.QDCOUNT);

        for (int i = 0; i < data.header.QDCOUNT; i++)
            ReadSectionQuestion(question, i);
    }
}

