#include "pch.h"
#include "Networking/Server/WebSocketServer.h"
#include "ConnectionInternal.h"
#include <webby/webby.h>

using namespace Networking;

class WebSocketWebbyServer final : public IWebSocketServer
{
public:
    WebSocketWebbyServer(unsigned short listeningPort, size_t maxClients, size_t requestBufferSize, size_t isBufferSize);
    virtual ~WebSocketWebbyServer() override final;

    virtual bool Initialize() override final;
    virtual void Finalize() override final;

    virtual void SetOnClientConnectedCallback(OnClientConnectedCallback&& callback) override final;
    virtual void SetOnConnectionClosedCallback(OnConnectionClosedCallback&& callback) override final;

    virtual bool Send(const IConnectionSharedPtr& connection, const Payload& data) override final;
    virtual void CloseConnection(const IConnectionSharedPtr& connection) override final;

    virtual void Update() override final;

    int OnDispatch(WebbyConnection* webbyConnection);
    int OnConnect(WebbyConnection* webbyConnection);
    int OnFrame(WebbyConnection* webbyConnection, const WebbyWsFrame* frame);
    void OnConnected(WebbyConnection* webbyConnection);
    void OnConnectionClosed(WebbyConnection* webbyConnection);

private:
    WebbyConnection* FindWebbyConnection(const IConnectionSharedPtr& handle) const;
    IConnectionInternalSharedPtr FindConnection(WebbyConnection* webbyConnection) const;

private:
    const WebbyServerConfig m_serverConfig;

    WebbyServer* m_server;
    std::unique_ptr<byte> m_serverMemory;
    OnClientConnectedCallback m_onConnectedCallback;
    OnConnectionClosedCallback m_onCloseConnectionCallback;
    std::vector<IConnectionInternalSharedPtr> m_connections;
};

static WebSocketWebbyServer* currentInstance = nullptr;

static WebbyServerConfig CreateServerConfig(unsigned short listeningPort, size_t maxClients, size_t requestBufferSize, size_t ioBufferSize)
{
    WebbyServerConfig result = { 0 };
    result.bind_address = "0.0.0.0";
    result.listening_port = listeningPort;
    result.flags = WEBBY_SERVER_WEBSOCKETS;
    result.connection_max = static_cast<int>(maxClients);
    result.request_buffer_size = static_cast<int>(requestBufferSize);
    result.io_buffer_size = static_cast<int>(ioBufferSize);

    result.dispatch = [](auto connection) 
    {
        return currentInstance->OnDispatch(connection); 
    };

    result.ws_connect = [](auto connection) 
    {
        return currentInstance->OnConnect(connection); 
    };

    result.ws_frame = [](auto connection, const WebbyWsFrame* frame) 
    {
        return currentInstance->OnFrame(connection, frame); 
    };

    result.ws_connected = [](auto connection) 
    {
        currentInstance->OnConnected(connection); 
    };

    result.ws_closed = [](auto connection) 
    {
        currentInstance->OnConnectionClosed(connection);
    };

    return result;
}

WebSocketWebbyServer::WebSocketWebbyServer(unsigned short listeningPort, size_t maxClients, size_t requestBufferSize, size_t ioBufferSize) :
    m_server{ nullptr },
    m_serverConfig{ CreateServerConfig(listeningPort, maxClients, requestBufferSize, ioBufferSize) }
{
}

WebSocketWebbyServer::~WebSocketWebbyServer()
{
    Finalize();
}

bool WebSocketWebbyServer::Initialize()
{
    if (m_server)
        return true;

    NETWORKING_ASSERT(m_onConnectedCallback);
    if (!m_onConnectedCallback)
        return false;

    const auto serverMemeorySize = WebbyServerMemoryNeeded(&m_serverConfig);
    m_serverMemory.reset(new byte[serverMemeorySize]);
    m_server = WebbyServerInit(&m_serverConfig, m_serverMemory.get(), serverMemeorySize);

    return m_server != nullptr;
}

void WebSocketWebbyServer::Finalize()
{
    if (!m_server)
        return;

    std::vector<IConnectionInternalSharedPtr> connections(m_connections.cbegin(), m_connections.cend());

    for (const auto& connection : connections)
        CloseConnection(connection);

    WebbyServerShutdown(m_server);
    m_server = nullptr;
    m_serverMemory.reset();
}

void WebSocketWebbyServer::SetOnClientConnectedCallback(OnClientConnectedCallback&& callback)
{
    NETWORKING_ASSERT(!m_onConnectedCallback);
    m_onConnectedCallback = std::move(callback);
}

void WebSocketWebbyServer::SetOnConnectionClosedCallback(OnConnectionClosedCallback&& callback)
{
    NETWORKING_ASSERT(!m_onCloseConnectionCallback);
    m_onCloseConnectionCallback = std::move(callback);
}

bool WebSocketWebbyServer::Send(const IConnectionSharedPtr& connection, const Payload& data)
{
    const auto webbyConnection = FindWebbyConnection(connection);

    if (!webbyConnection)
        return false;

    if (WebbyBeginSocketFrame(webbyConnection, WEBBY_WS_OP_BINARY_FRAME) == 0)
    {
        if (WebbyWrite(webbyConnection, data.data(), data.size()) == 0)
        {
            if (WebbyEndSocketFrame(webbyConnection) == 0)
                return true;
        }
    }

    return false;
}

void WebSocketWebbyServer::CloseConnection(const IConnectionSharedPtr& connection)
{
    const auto webbyConnection = FindWebbyConnection(connection);
    NETWORKING_ASSERT(webbyConnection);

    if (!webbyConnection)
        return;

    WebbyBeginSocketFrame(webbyConnection, WEBBY_WS_OP_CLOSE);
    WebbyEndSocketFrame(webbyConnection);

    static_cast<IConnectionInternal*>(connection.get())->OnClosed();
}

void WebSocketWebbyServer::Update()
{
    if (!m_server)
        return;

    const auto buffer = currentInstance;
    currentInstance = this;
    WebbyServerUpdate(m_server);
    currentInstance = buffer;
}

int WebSocketWebbyServer::OnDispatch(WebbyConnection* webbyConnection)
{
    if (!webbyConnection)
        return 1;

    return 0;
}

int WebSocketWebbyServer::OnConnect(WebbyConnection* webbyConnection)
{
    return webbyConnection ? 0 : 1;
}

int WebSocketWebbyServer::OnFrame(WebbyConnection* webbyConnection, const WebbyWsFrame* frame)
{
    static const size_t bufferSize = 128;

    if (!webbyConnection || !webbyConnection->user_data)
        return 0;

    const auto connection = FindConnection(webbyConnection);

    if (!connection)
        return 0;

    std::vector<byte> payload;

    auto remain = static_cast<size_t>(frame->payload_length) - payload.size();

    while (remain > 0)
    {
        std::vector<byte> buffer;
        buffer.resize(std::min(bufferSize, remain), 0);

        if (WebbyRead(webbyConnection, &buffer[0], buffer.size()) != 0)
            break;

        payload.insert(payload.end(), buffer.begin(), buffer.end());
        remain = static_cast<size_t>(frame->payload_length) - payload.size();
    }

    connection->OnReceived(payload);
    return 0;
}

void WebSocketWebbyServer::OnConnected(WebbyConnection* webbyConnection)
{
    if (!webbyConnection || !m_onConnectedCallback)
        return;

    const auto connection = IConnectionInternal::Create(*this, webbyConnection->address);
    webbyConnection->user_data = connection.get();
    m_connections.emplace_back(connection);

    m_onConnectedCallback(connection);
}

void WebSocketWebbyServer::OnConnectionClosed(WebbyConnection* webbyConnection)
{
    if (!webbyConnection || !webbyConnection->user_data || !m_onCloseConnectionCallback)
        return;

    if (const auto connection = FindConnection(webbyConnection))
    {
        connection->OnClosed();
        if (m_onCloseConnectionCallback)
            m_onCloseConnectionCallback(connection);
    }

    m_connections.erase(find_if(m_connections.begin(), m_connections.end(), [&webbyConnection](const auto& connection) 
    { 
        return connection.get() == webbyConnection->user_data;
    }));

    webbyConnection->user_data = nullptr;
}

WebbyConnection* WebSocketWebbyServer::FindWebbyConnection(const IConnectionSharedPtr& connection) const
{
    return connection ? WebbyFindConnectionFromUserData(m_server, connection.get()) : nullptr;
}

IConnectionInternalSharedPtr WebSocketWebbyServer::FindConnection(WebbyConnection* webbyConnection) const
{
    const auto iterator = find_if(m_connections.begin(), m_connections.end(), [&webbyConnection](const auto& connection) 
    {
        return connection.get() == webbyConnection->user_data;
    });

    return iterator != m_connections.end() ? *iterator : nullptr;
}

IWebSocketServerUniquePtr IWebSocketServer::CreateWebby(unsigned short listeningPort, size_t maxClients, size_t requestBufferSize, size_t ioBufferSize)
{
    return std::make_unique<WebSocketWebbyServer>(listeningPort, maxClients, requestBufferSize, ioBufferSize);
}