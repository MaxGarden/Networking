#include "pch.h"
#include "ConnectionInternal.h"

using namespace Networking;

class ConnectionInternal final : public IConnectionInternal, public std::enable_shared_from_this<ConnectionInternal>
{
public:
    ConnectionInternal(INetwork& network, const std::string& address);
    virtual ~ConnectionInternal() override = default;

    virtual const std::string& GetAddress() const noexcept override final;

    virtual bool IsConnected() const noexcept override final;

    virtual void Close() override final;
    virtual void OnClosed() override final;

    virtual bool Send(const Payload& data) override final;
    virtual void OnReceived(const Payload& data) override final;

    virtual const ConnectionID& GetID() const noexcept override final;

private:
    INetwork& m_network;
    const std::string m_address;
    const ConnectionID m_id;

    bool m_isConnected;
}; 

static ConnectionID GetFreeConnectionID()
{
    static ConnectionID id = 0;
    return id++;
}

ConnectionInternal::ConnectionInternal(INetwork& network, const std::string& address) :
    m_network(network),
    m_address(address),
    m_id(GetFreeConnectionID())
{
}

const std::string& ConnectionInternal::GetAddress() const noexcept
{
    return m_address;
}

bool ConnectionInternal::IsConnected() const noexcept
{
    return m_isConnected;
}

void ConnectionInternal::Close()
{
    if (!m_isConnected)
        return;

    m_network.CloseHandle(shared_from_this());
}

void ConnectionInternal::OnClosed()
{
    m_isConnected = false;
}

bool ConnectionInternal::Send(const Payload& data)
{
    if (!IsConnected())
        return false;

    return m_network.Send(shared_from_this(), data);
}

void ConnectionInternal::OnReceived(const Payload& data)
{
    NETWORKING_ASSERT(false);
    //TODO
}

const ConnectionID& ConnectionInternal::GetID() const noexcept
{
    return m_id;
}

IConnectionSharedPtr IConnectionInternal::Create(INetwork& network, const std::string& address)
{
    return std::make_shared<ConnectionInternal>(network, address);
}