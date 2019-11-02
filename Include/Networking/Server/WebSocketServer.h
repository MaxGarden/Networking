#pragma once
#include "Server.h"

namespace Networking
{
    class IWebSocketServer : public IServer
    {
    public:
        virtual ~IWebSocketServer() = default;

        static IWebSocketServerUniquePtr CreateWebby(unsigned short listeningPort, size_t maxClients, size_t requestBufferSize = 8192u, size_t ioBufferSize = 8192u);
    };
}