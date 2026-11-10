#include "FileRecord.h"
#include <iostream>
#include <fstream>

namespace fsdns {
    FILEERR File::LoadFile(const char* filename)
    {
        std::ifstream file(filename, std::iostream::binary);

        if (!file.is_open())
        {
            return FILE_NOT_FOUND;
        }

        char c;

        while (file.get(c))
        {
            filecontent.push_back(c);
        }

        file.close();

        return FILE_OK;
    }

    int DNSRecord::GetZone(DNSRECORD& dnsrec, int curpos)
    {
        for (int i = curpos+1; i < filecontent.size(); i++)
        {
            if (filecontent[i] == ']')
                return 0;

            dnsrec.ZoneName += filecontent[i];
        }

        return 1;
    }

    int DNSRecord::GetAddr(DNSRECORD& dnsrec, int curpos)
    {
        std::string adr;

        for (int j = curpos + 1; j < filecontent.size(); j++)
        {
            if (filecontent[j] == '\n')
            {
                break;
            }

            adr += filecontent[j];
        }

        dnsrec.Address = TransformIP(adr);

        return 1;   
    }

    int DNSRecord::GetNS(DNSRECORD& dnsrec, int curpos)
    {
        std::string out;

        for (int j = curpos + 2; j < filecontent.size(); j++)
        {
            if (filecontent[j] == '\n')
            {
                break;
            }

            out += filecontent[j];
        }

        dnsrec.NameServerRecords.push_back(out);

        return 1;   
    }

    std::vector<DNSRECORD> DNSRecord::GetRecords()
    {
        std::vector<DNSRECORD> rec;

        if (filecontent.empty())
            return rec;
            
        for (int i = 0; i < filecontent.size(); i++)
        {
            if (filecontent[i] == '[')
            {
                rec.emplace_back();
                GetZone(rec.back(), i);
            }
            else if (filecontent[i] == 'A')
            {
                if (rec.empty())
                    continue;

                GetAddr(rec.back(), i);
            }
            else if (i + 1 < filecontent.size() &&
                filecontent[i] == 'N' &&
                filecontent[i + 1] == 'S')
            {
                if (rec.empty())
                    continue;

                GetNS(rec.back(), i);
            }
            
        }

        return rec;
    }

    std::vector<char> TransformIP(std::string IP)
    {
        std::vector<char> OutIP;
        std::string NUM;

        for (int i = 0; i < IP.size(); i++)
        {
            if (IP[i] == '.')
            {
                OutIP.push_back(atoi(NUM.c_str()));
                NUM.clear();
            }

            NUM += IP[i];
        }
        
        if (OutIP.size() < 4)
        {
            return {};
        }

        return OutIP;
    }
}

