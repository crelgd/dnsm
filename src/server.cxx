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

    bool serverExist = false;

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
            if (s == 0xff)
                ds.data.header.RCODE = 0;
            else
                ds.data.header.RCODE = 3;
        }
    }
 
    ds.data.header.ANCOUNT = answerCount;

    ds.data.header.RA = 0;
    ds.data.header.QR = 1;

    answer = dns::GenerateAll(ds.data, crecord);
    answer.insert(answer.end(), answerData.begin(), answerData.end());

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


