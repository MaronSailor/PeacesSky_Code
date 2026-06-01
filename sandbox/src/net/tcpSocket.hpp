#pragma once

#include "netPackets.hpp"

#include <atomic>
#include <cstdint>
#include <mutex>
#include <queue>
#include <string>
#include <thread>
#include <vector>

#if defined(_WIN32)
#define NOMINMAX
#include <winsock2.h>
#include <ws2tcpip.h>
using SocketHandle = SOCKET;
constexpr SocketHandle InvalidSocketHandle = INVALID_SOCKET;
#else
#include <arpa/inet.h>
#include <fcntl.h>
#include <netinet/tcp.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <unistd.h>
#include <errno.h>
using SocketHandle = int;
constexpr SocketHandle InvalidSocketHandle = -1;
#endif

class TcpSocket
{
public:
	TcpSocket();
	~TcpSocket();

	bool connectTo(const std::string& address, uint16_t port);
	bool bindAndListen(uint16_t port);
	bool acceptClient(TcpSocket& outClient);
	int sendBytes(const void* data, size_t size);
	int receiveBytes(void* data, size_t size);
	bool isValid() const;
	void close();
	bool setNonBlocking(bool nonBlocking);
	bool setNoDelay(bool noDelay);

	// 1=connected, 0=connecting, -1=failed
	int checkConnectStatus() const;

private:
	SocketHandle socket = InvalidSocketHandle;
};

class TcpClient
{
public:
	bool connectToServer(const std::string& address, uint16_t port);
	void disconnect();
	bool isConnected() const;
	bool isConnecting() const;
	bool hasConnectionFailed() const;
	void pollConnection();
	bool sendPacket(const NetMessagePacket& packet);
	bool receivePacket(NetMessagePacket& outPacket);

private:
	enum class ConnectionState { Idle, Connecting, Connected, Failed };
	ConnectionState connectionState = ConnectionState::Idle;
	TcpSocket socket;
	std::vector<uint8_t> receiveBuffer;
};

class TcpServer
{
public:
	bool start(uint16_t port);
	void stop();
	void pollAccept();
	bool isClientConnected() const;
	bool sendPacket(const NetMessagePacket& packet);
	bool receivePacket(NetMessagePacket& outPacket);

private:
	TcpSocket listenSocket;
	TcpSocket clientSocket;
	bool clientConnected = false;
	std::vector<uint8_t> receiveBuffer;
};

// ---------------------------------------------------------------------------
// NetworkThread — Core 3
// Runs all TCP socket I/O on a dedicated background thread.
// The game logic thread exchanges data via thread-safe packet queues.
// ---------------------------------------------------------------------------
class NetworkThread
{
public:
	enum class Role { None, Server, Client };

	NetworkThread() = default;
	~NetworkThread() { stop(); }

	// Starts the background I/O thread as a server or client.
	// Any previously running thread is stopped first.
	void startServer(uint16_t port);
	void startClient(const std::string& address, uint16_t port);

	// Stops the background thread and resets all state.
	void stop();

	// Thread-safe state queries (game logic thread)
	bool isConnected()           const { return m_connected.load();           }
	bool isConnecting()          const { return m_connecting.load();          }
	bool hasConnectionFailed()   const { return m_failed.load();              }
	bool isClientPresent()       const { return m_clientPresent.load();       }
	// Returns true (and clears the flag) when the remote peer disconnected mid-game
	bool hasRemoteDisconnected()       { return m_clientDisconnected.exchange(false); }

	// Thread-safe packet exchange (game logic thread)
	void pushOutgoing(const NetMessagePacket& packet);
	bool popIncoming(NetMessagePacket& outPacket);

private:
	void serverLoop();
	void clientLoop();

	Role        m_role = Role::None;
	std::thread m_thread;

	std::atomic<bool> m_running           { false };
	std::atomic<bool> m_connected         { false };
	std::atomic<bool> m_connecting        { false };
	std::atomic<bool> m_failed            { false };
	std::atomic<bool> m_clientPresent     { false };
	std::atomic<bool> m_clientDisconnected{ false };

	std::string  m_address;
	uint16_t     m_port = 0;

	std::mutex                   m_inMutex;
	std::queue<NetMessagePacket> m_inQueue;

	std::mutex                   m_outMutex;
	std::queue<NetMessagePacket> m_outQueue;

	TcpServer m_server;
	TcpClient m_client;
};