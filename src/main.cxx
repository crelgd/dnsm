#include <stdio.h>
#include "server.h"

int main()
{
    DNSSrv dns(PORT);

    if (dns.SocketInit() != 0)
    {
        return 1;
    }

    dns.Runtime();

    return 0;
}

