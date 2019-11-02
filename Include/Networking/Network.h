#pragma once
#include "Networking.h"

namespace Networking
{
    using Payload = std::vector<byte>;

    class INetwork
    {
    public:
        using OnConnectionClosedCallback = std::function<void(IConnectionSharedPtr connection)>;

    public:
        virtual ~INetwork() = default;

        virtual bool Initialize() = 0;

        virtual void SetOnConnectionClosedCallback(const OnConnectionClosedCallback& callback) = 0;

        virtual bool Send(IConnectionSharedPtr connection, const Payload& data) = 0;
        virtual void CloseHandle(IConnectionSharedPtr connection) = 0;

        virtual void Update() = 0;
    };
}