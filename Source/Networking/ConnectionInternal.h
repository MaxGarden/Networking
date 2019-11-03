#pragma once
#include "Networking/Connection.h"

namespace Networking
{
    class IConnectionInternal : public IConnection
    {
    public:
        virtual ~IConnectionInternal() override = default;

        virtual const ConnectionID& GetID() const noexcept = 0;

        virtual void OnReceived(const Payload& data) = 0;
        virtual void OnClosed() = 0;

        static IConnectionInternalSharedPtr Create(INetwork& network, const std::string& address);
    };
}