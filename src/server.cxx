#include <stdio.h>
#include "server.h"
#include "dns.h"
#include <iostream>

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
    int listen = recvfrom(Socket, RequestBfr, sizeof(RequestBfr), 
        NULL, (SOCKADDR*) &SenderAddr, &SenderAddrSize);
    if (listen == SOCKET_ERROR)
    {
        printf("Datagram error\n");
        return 1;
    }

    for (int i = 0; i < listen; i++)
    {
        printf("%c", RequestBfr[i]);
    }
    printf("\n");

    return 0;
}

int DNSSrv::Runtime()
{
    Listen();

    std::string DNSRequest = dns::ToBinary(RequestBfr, sizeof(RequestBfr));    

    dns::DnsStruct ds = dns::DnsStruct();

    ds.ReadData(DNSRequest);

    std::cout << "DNS Request ID: "      << ds.data.header.ID                         << std::endl;
    std::cout << "DNS Request QR: "      << static_cast<int>(ds.data.header.QR)       << std::endl;
    std::cout << "DNS Request Opcode: "  << static_cast<int>(ds.data.header.Opcode)   << std::endl;
    std::cout << "DNS Request AA: "      << static_cast<int>(ds.data.header.AA)       << std::endl;
    std::cout << "DNS Request TC: "      << static_cast<int>(ds.data.header.TC)       << std::endl;
    std::cout << "DNS Request RD: "      << static_cast<int>(ds.data.header.RD)       << std::endl;
    std::cout << "DNS Request RA: "      << static_cast<int>(ds.data.header.RA)       << std::endl;
    std::cout << "DNS Request Z: "       << static_cast<int>(ds.data.header.Z)        << std::endl;
    std::cout << "DNS Request RCODE: "   << static_cast<int>(ds.data.header.RCODE)    << std::endl;
    std::cout << "DNS Request QDCOUNT: " << ds.data.header.QDCOUNT                    << std::endl;
    std::cout << "DNS Request ANCOUNT: " << ds.data.header.ANCOUNT                    << std::endl;
    std::cout << "DNS Request NSCOUNT: " << ds.data.header.NSCOUNT                    << std::endl;
    std::cout << "DNS Request ARCOUNT: " << ds.data.header.ARCOUNT                    << std::endl;


    for (int j = 0; j < ds.data.header.QDCOUNT; j++)
    {
    for (int i = 0; i < ds.data.request[j].QNAME.size(); i++)
    {
        std::cout << ds.data.request[j].QNAME[i] << std::endl;
    }
    }

    return 0;
}


