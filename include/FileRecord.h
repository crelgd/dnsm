#pragma once

#include <vector>
#include <string>

typedef struct {
    std::string ZoneName;
    std::vector<char> Address;
    std::vector<std::string> NameServerRecords;
} DNSRECORD;

typedef enum {
    FILE_OK,
    FILE_NOT_FOUND
} FILEERR;

namespace fsdns {
    class File
    {
    public:
        FILEERR LoadFile(const char* filename);            

    public:
        std::vector<char> filecontent;
    };

    class DNSRecord : public File
    {
    public:
        std::vector<DNSRECORD> GetRecords();

    private:
        int GetZone(DNSRECORD& dnsrec, int curpos);
        int GetAddr(DNSRECORD& dnsrec, int curpos);
        int GetNS(DNSRECORD& dnsrec, int curpos);
    };

    std::vector<char> TransformIP(std::string IP);
}

