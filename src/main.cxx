#include "server.h"
// #include "dns.h"

// #include "FileRecord.h"

int main()
{
    DNSSrv dns(PORT);

    // fsdns::DNSRecord file;
    // if (file.LoadFile("config.ini") == FILE_NOT_FOUND)
    // {
    //     std::cout << "File NOT FOUND" << std::endl;
    //     return 1;
    // }

    // std::vector<DNSRECORD> rec = file.GetRecords();

    // for (int i = 0; i < rec.size(); i++)
    // {
    //     std::cout << i << ". " << std::endl;

    //     std::cout << "\t Zone: " << rec[i].ZoneName << std::endl;
    //     std::cout << "\t Address: " << rec[i].Address << std::endl;

    //     for (int j = 0; j < rec[i].NameServerRecords.size(); j++)
    //     {
    //         std::cout << "\t NameServer: " << rec[i].NameServerRecords[j] << std::endl;
    //     }
    // }

    if (dns.SocketInit() != 0)
    {
        return 1;
    }

    while (1)
    {
        dns.Runtime();
    }

    return 0;
}

