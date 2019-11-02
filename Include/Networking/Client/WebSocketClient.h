#pragma once
#include "Client.h"

namespace Networking
{
    class IWebSocketClient : public IClient
    {
    public:
        virtual ~IWebSocketClient() override = default;

        static IWebSocketClientUniquePtr CreateEasyWs();
    };
}