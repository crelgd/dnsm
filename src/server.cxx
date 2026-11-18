#include <stdio.h>
#include "server.h"
#include "dns.h"
#include <iostream>
#include <vector>

DNSSrv::DNSSrv(unsigned short ListeningPort)
{
    iResult = WSAStartup(MAKEWORD(2, 2), &wsadata);
    if (iResult != 0)
    {
        printf("WSAStartup failed: %d\n", iResult);
    }

    RecvAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    RecvAddr.sin_port = htons(ListeningPort);
    RecvAddr.sin_family = AF_INET;

    if (file.LoadFile(CONFIG_FILE) == FILE_NOT_FOUND)
    {
        std::cout << "config.ini not found" << std::endl;
        return;
    }

    crecord = file.GetRecords();
    cache_non = crecord.size();
}

DNSSrv::~DNSSrv()
{
    printf("Closing socket\n");
    iResult = closesocket(Socket);
    if (iResult == SOCKET_ERROR)
    {
        printf("Closing socket error\n");
    }

    printf("Exiting");
    WSACleanup();
}

int DNSSrv::SocketInit()
{
    Socket = socket(
        AF_INET,
        SOCK_DGRAM,
        IPPROTO_UDP
    );

    iResult = bind(Socket, (SOCKADDR*) &RecvAddr, sizeof(RecvAddr));
    if (iResult == SOCKET_ERROR) {
        printf("bind failed with error: %d\n", WSAGetLastError());
        return 1;
    }

    return 0;
}

int DNSSrv::Listen()
{
    listenVAL = recvfrom(Socket, RequestBfr, sizeof(RequestBfr), 
        NULL, (SOCKADDR*) &SenderAddr, &SenderAddrSize);
    if (listenVAL == SOCKET_ERROR)
    {
        printf("Datagram error\n");
        return 1;
    }

    return 0;
}

int DNSSrv::Runtime()
{
    Listen();

    std::string DNSRequest = dns::ToBinary(RequestBfr, sizeof(RequestBfr));

    dns::DnsStruct ds = dns::DnsStruct();
    ds.ReadData(DNSRequest);

    std::vector<char> answer;
    std::vector<char> answerData;
    int answerCount = 0;

    int change_cache = 0;

    for (int i = 0; i < ds.data.header.QDCOUNT; i++) {
        std::string search = dns::BuildPath(ds.data.request[i].QNAME);

        int s = 0;
        std::vector<char> answerDataBfr = dns::AnswerGenerate(ds.data.request[i], crecord, search, &s);

        if (!answerDataBfr.empty())
        {
            answerData.insert(answerData.end(), answerDataBfr.begin(),answerDataBfr.end());

            ds.data.header.AA = 1;

            if (s != 255)  
                answerCount += s;
            else answerCount++;
        }
        else {
            if (s == 0xff) {
                ds.data.header.RCODE = 0;
                change_cache = 1;
            }
            else {
                ds.data.header.RCODE = 3;
                change_cache = 0;
            }
        }
    }

    std::vector<char> annotherDNSAnswer;

    if (ds.data.header.RCODE == 3 || change_cache)
    {
        answer = requestS.RequestDNS(RequestBfr, listenVAL);

        DNSRECORD drc = SaveRecordToCache(dns::ToBinary(answer.data(), answer.size()), answer, listenVAL);

        if (change_cache)
        {
            for (int i = 0; i < crecord.size(); i++)
            {
                if (drc.ZoneName == crecord[i].ZoneName)
                {
                    if (!drc.Address.empty())
                        crecord[i].Address = drc.Address;
                    else if (!drc.NameServerRecords.empty())
                        crecord[i].NameServerRecords = drc.NameServerRecords;
                    
                    break;
                }    
            }
        } else 
            crecord.push_back(drc);

        if (answer.empty())
        {
            printf("Annother dns err\n");
            return 1;
        }
    }
    else {
        ds.data.header.ANCOUNT = answerCount;

        ds.data.header.RA = 0;
        ds.data.header.QR = 1;

        answer = dns::GenerateAll(ds.data, crecord);
        answer.insert(answer.end(), answerData.begin(), answerData.end());
    }
    
    iResult = sendto(Socket, answer.data(), answer.size(), 0, (SOCKADDR*)&SenderAddr, SenderAddrSize);
    if (iResult == SOCKET_ERROR) {
        printf("send failed: %d\n", WSAGetLastError());
        closesocket(Socket);
        WSACleanup();
        return 1;
    }

    // std::cout << "DNS Request ID: "      << ds.data.header.ID                         << std::endl;
    // std::cout << "DNS Request QR: "      << static_cast<int>(ds.data.header.QR)       << std::endl;
    // std::cout << "DNS Request Opcode: "  << static_cast<int>(ds.data.header.Opcode)   << std::endl;
    // std::cout << "DNS Request AA: "      << static_cast<int>(ds.data.header.AA)       << std::endl;
    // std::cout << "DNS Request TC: "      << static_cast<int>(ds.data.header.TC)       << std::endl;
    // std::cout << "DNS Request RD: "      << static_cast<int>(ds.data.header.RD)       << std::endl;
    // std::cout << "DNS Request RA: "      << static_cast<int>(ds.data.header.RA)       << std::endl;
    // std::cout << "DNS Request Z: "       << static_cast<int>(ds.data.header.Z)        << std::endl;
    // std::cout << "DNS Request RCODE: "   << static_cast<int>(ds.data.header.RCODE)    << std::endl;
    // std::cout << "DNS Request QDCOUNT: " << ds.data.header.QDCOUNT                    << std::endl;
    // std::cout << "DNS Request ANCOUNT: " << ds.data.header.ANCOUNT                    << std::endl;
    // std::cout << "DNS Request NSCOUNT: " << ds.data.header.NSCOUNT                    << std::endl;
    // std::cout << "DNS Request ARCOUNT: " << ds.data.header.ARCOUNT                    << std::endl;


    // for (int j = 0; j < ds.data.header.QDCOUNT; j++)
    // {
    // for (int i = 0; i < ds.data.request[j].QNAME.size(); i++)
    // {
    //     std::cout << ds.data.request[j].QNAME[i] << std::endl;
    // }
    // }

    return 0;
}

// мне лень крч красиво расписывать
DNSRECORD DNSSrv::SaveRecordToCache(std::string dns_data, std::vector<char> dns_data_source, int request_counter)
{   
    if (dns_data.size() < request_counter)
        return {};

    DNSRECORD out = {};

    dns::DnsStruct dnssc;
    dnssc.ReadData(dns_data);

    std::string search = dns::BuildPath(dnssc.data.request[0].QNAME);
    out.ZoneName = search;

    int jout = request_counter;

    std::vector<std::vector<char>> iplist;
    
    std::vector<std::string> NSrec;

    for (int i = 0; i < dnssc.data.header.ANCOUNT; i++)
    {
        while (jout < dns_data_source.size() - request_counter)
        {
            uint8_t dsize = dns_data_source[jout];
            if (dsize == 0x00)
                break;

            if ((dsize & 0xc0) != 0)
            {
                jout++;
                break;
            }

            jout += dsize + 1;
        }

        if (jout + 1 >= dns_data_source.size())
            return {};

        jout++;

        uint8_t starb = dns_data_source[jout];
        uint8_t mladb = dns_data_source[jout+1];
        uint16_t TYPE = (static_cast<uint16_t>(starb) << 8) | mladb;

        jout += 8;

        switch (TYPE)
        {
        case 1:
            {
                starb = dns_data_source[jout];
                mladb = dns_data_source[jout+1];

                jout += 2;

                uint16_t size = (static_cast<uint16_t>(starb) << 8) | mladb;

                if (jout + size > dns_data_source.size())
                    return {};

                std::vector<char> ip;
                for (int c = 0; c < size; c++)
                {
                    ip.push_back(dns_data_source[jout+c]);
                }        
                iplist.push_back(ip);

                jout += size;

                out.Address.push_back(ip);
            }
            break;

        case 2:
            {
                starb = dns_data_source[jout];
                mladb = dns_data_source[jout+1];

                jout += 2;

                uint16_t size = (static_cast<uint16_t>(starb) << 8) | mladb;
                std::vector<std::string> convertbfr;

                size_t end = jout + size;

                while (jout < end)
                {
                    if (jout >= end)
                        return {};

                    uint16_t dmsize = dns_data_source[jout]; jout++;

                    if ((dmsize & 0xc0) == 0)
                    {
                        std::string dmn;
                        for (int i = 0; i < dmsize; i++)
                        {
                            dmn += dns_data_source[jout++];
                        }
                        
                        convertbfr.push_back(dmn);
                    }
                    else if ((dmsize & 0xC0) == 0xC0)
                    {
                        uint16_t pointer =
                            ((dmsize & 0x3F) << 8) |
                            static_cast<uint8_t>(dns_data_source[jout]);
                        jout ++;

                        while (dns_data_source[pointer] != 0x00)
                        {
                            std::string dmn;

                            uint8_t dd = dns_data_source[pointer];
                            pointer++;

                            for (int i = 0; i < dd; i++)
                            {
                                dmn += dns_data_source[pointer++];
                            }
                            convertbfr.push_back(dmn);
                        }
                    }
                    else
                        return {};
                }

                std::string converted = dns::BuildPath(convertbfr);

                out.NameServerRecords.push_back(converted);
            }
            break;
        }
    }

    return out;
}

