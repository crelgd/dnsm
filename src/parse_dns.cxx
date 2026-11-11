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

    std::vector<std::string> FragmentPath(std::string path)
    {
        std::vector<std::string> Mout;
        std::string out;

        for (int i = 0; i < path.size(); i++)
        {
            if (path[i] == '.')
            {
                if (!out.empty())
                    Mout.push_back(out);

                out.clear();
                continue;
            }

            if (path[i] == ' ')
                continue;

            out += path[i];
        }

        if (!out.empty())
            Mout.push_back(out);

        return Mout;
    }

    // отнимать 1 от вывода, чтоб убрать точку с конца
    std::string BuildPath(std::vector<std::string> path)
    {
        std::string out;

        for (int i = 0; i < path.size(); i++)
        {
            out += path[i];
            out += '.';
        }

        return out;
    }

    // прост
    void Push16(std::vector<char>& vec, uint16_t value)
    {
        vec.push_back(value >> 8);
        vec.push_back(value & 0xFF);
    }

    void Push32(std::vector<char>& vec, uint32_t value)
    {
        vec.push_back(value >> 24);
        vec.push_back(value >> 16);
        vec.push_back(value >> 8);
        vec.push_back(value);
    }

    std::string ToBinary16Bit(uint16_t val)
    {
        uint8_t mlb = val & 0xFF;
        uint8_t stb = (val >> 8) & 0xFF;

        std::string out;

        out += byteToBinary(stb);
        out += byteToBinary(mlb);

        return out;
    }

    std::vector<char> GenerateAll(DNS_DATA& data, std::vector<DNSRECORD> conf)
    {
        std::vector<char> answer;
        std::string bin_transform;

        bin_transform += ToBinary16Bit(data.header.ID); // ID
        bin_transform += byteToBinary( // QR, Opcode, AA, TC, RD
            (static_cast<uint8_t>(data.header.QR) << 7) |
            ((data.header.Opcode & 0xf) << 3) |
            (static_cast<uint8_t>(data.header.AA) << 2) |
            (static_cast<uint8_t>(data.header.TC) << 1) |
            static_cast<uint8_t>(data.header.RD)
        );
        bin_transform += byteToBinary( // RA, Z, RCODE
            (static_cast<uint8_t>(data.header.RA) << 7) |
            ((data.header.Z & 0xf) << 4) |
            ((data.header.RCODE & 0xf))
        );
        bin_transform += ToBinary16Bit(data.header.QDCOUNT);
        bin_transform += ToBinary16Bit(data.header.ANCOUNT);
        bin_transform += ToBinary16Bit(data.header.NSCOUNT);
        bin_transform += ToBinary16Bit(data.header.ARCOUNT); 

        for (int j = 0; j < data.header.QDCOUNT; j++) {
            for (int i = 0; i < data.request[j].QNAME.size(); i++)
            {
                bin_transform += byteToBinary(data.request[j].QNAME[i].size() & 0x3F); // size
                for (int u = 0; u < data.request[j].QNAME[i].size(); u++)
                {
                    bin_transform += byteToBinary(data.request[j].QNAME[i][u]);
                }
            }

            bin_transform += byteToBinary(0);
            bin_transform += ToBinary16Bit(data.request[j].QTYPE);
            bin_transform += ToBinary16Bit(data.request[j].QCLASS);
        }

        for (int i = 0; i < bin_transform.size(); i+=8)
        {
            std::string bfr(8, '\0');
            memcpy(&bfr[0], &bin_transform[i], 8);

            answer.push_back(ToByte(bfr, 8));
        }

        return answer;
    }

    std::vector<char> NewAnswer(DNS_QUESTION& qs, std::vector<DNSRECORD> conf, int rc)
    {
        std::vector<char> answer;

        std::vector<std::string> rrr = FragmentPath(conf[rc].ZoneName);

        for (int i = 0; i < rrr.size(); i++)
        {
            char lsize = rrr[i].size() & 0x3F;
            answer.push_back(lsize);

            answer.insert(answer.end(), rrr[i].begin(), rrr[i].end());
        }

        answer.push_back(0x00);

        Push16(answer, qs.QTYPE);
        Push16(answer, 1);
        Push32(answer, 3090);

        return answer;
    }

    std::vector<char> AnswerGenerate(DNS_QUESTION& qs, std::vector<DNSRECORD> conf, std::string Searchble, int* bdata)
    {
        std::vector<char> answer;

        int RecordPointer = 0;
        bool notfound = false;

        for (int i = 0; i < conf.size(); i++)
        {
            if (Searchble == conf[i].ZoneName)
            {
                RecordPointer = i;

                *bdata = 0xff;

                switch (qs.QTYPE)
                {
                case 1:
                    {
                        if (!conf[i].Address.empty())
                            notfound = false;
                    }
                    break;

                case 2:
                    {
                        if (!conf[i].NameServerRecords.empty())
                            notfound = false;
                    }
                    break;

                default:
                    notfound = true;
                    break;
                }

                break;
            }
            else{
                notfound = true;
            };
        }

        if (notfound)
        { 
            return {};
        }

        switch (qs.QTYPE)
        {
        case 1:
            {
                std::vector<char> an = NewAnswer(qs, conf, RecordPointer);
                answer.insert(answer.end(), an.begin(), an.end());

                uint16_t dataSize = conf[RecordPointer].Address.size();
                Push16(answer, dataSize);
                for (int i = 0; i < dataSize; i++)
                {
                    answer.push_back(conf[RecordPointer].Address[i]);
                }
                break;
            }

        case 2:
            {
                *bdata = conf[RecordPointer].NameServerRecords.size();

                for (int i = 0; i < *bdata; i++)
                {
                    std::vector<std::string> path = FragmentPath(conf[RecordPointer].NameServerRecords[i]);

                    std::vector<char> an = NewAnswer(qs, conf, RecordPointer);
                    answer.insert(answer.end(), an.begin(), an.end());

                    int sv = answer.size();
                    uint16_t size = 0;
                    Push16(answer, 0);

                    for (int j = 0; j < path.size(); j++)
                    {
                        answer.push_back(path[j].size() & 0x3f);
                        size++;
                        for (int l = 0; l < path[j].size(); l++)
                        {
                            answer.push_back(path[j][l]);
                            size ++;
                        }
                    }

                    answer.push_back(0);
                    size ++;

                    answer[sv]     = (size >> 8) & 0xFF;
                    answer[sv + 1] = size & 0xFF;
                }

                
            }

            break;
        }

        return answer;
    }
}

