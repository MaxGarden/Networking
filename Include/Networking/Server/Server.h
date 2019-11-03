#pragma once
#include "Networking/Network.h"

namespace Networking
{
    class IServer : public INetwork
    {
    public:
        using OnClientConnectedCallback = std::function<bool(IConnectionSharedPtr connection)>;

    public:
        virtual ~IServer() = default;

        virtual void SetOnClientConnectedCallback(OnClientConnectedCallback&& callback) = 0;
    };
}