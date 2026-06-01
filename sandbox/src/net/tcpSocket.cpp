#include "tcpSocket.hpp"

#include <chrono>
#include <cstring>
#include <thread>
#include <cstdio>

#if defined(_WIN32)
#pragma comment(lib, "Ws2_32.lib")
#endif

namespace
{
	bool ensureSocketInitialized()
	{
#if defined(_WIN32)
		static bool initialized = false;
		if (!initialized)
		{
			WSADATA data{};
			if (WSAStartup(MAKEWORD(2, 2), &data) != 0)
			{
				return false;
			}
			initialized = true;
		}
#endif
		return true;
	}

	bool isWouldBlockError()
	{
#if defined(_WIN32)
		const int error = WSAGetLastError();
		return error == WSAEWOULDBLOCK || error == WSAEINPROGRESS;
#else
		return errno == EWOULDBLOCK || errno == EAGAIN || errno == EINPROGRESS;
#endif
	}

	void closeSocket(SocketHandle& handle)
	{
		if (handle == InvalidSocketHandle)
		{
			return;
		}

#if defined(_WIN32)
		closesocket(handle);
#else
		close(handle);
#endif
		handle = InvalidSocketHandle;
	}
}

TcpSocket::TcpSocket() = default;

TcpSocket::~TcpSocket()
{
	close();
}

bool TcpSocket::connectTo(const std::string& address, uint16_t port)
{
	if (!ensureSocketInitialized())
	{
		std::fprintf(stderr, "[TcpSocket] Failed to initialize socket library\n");
		return false;
	}

	if (socket != InvalidSocketHandle)
	{
		return true;
	}

	std::fprintf(stderr, "[TcpSocket] Attempting to connect to %s:%u\n", address.c_str(), port);

	addrinfo hints{};
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;

	addrinfo* result = nullptr;
	const int gai_result = getaddrinfo(address.c_str(), std::to_string(port).c_str(), &hints, &result);
	if (gai_result != 0 || !result)
	{
#if defined(_WIN32)
		std::fprintf(stderr, "[TcpSocket] getaddrinfo failed for %s:%u with error %d (WSA: %d)\n", 
			address.c_str(), port, gai_result, WSAGetLastError());
#else
		std::fprintf(stderr, "[TcpSocket] getaddrinfo failed for %s:%u: %s\n", 
			address.c_str(), port, gai_strerror(gai_result));
#endif
		return false;
	}

	socket = ::socket(result->ai_family, result->ai_socktype, result->ai_protocol);
	if (socket == InvalidSocketHandle)
	{
#if defined(_WIN32)
		std::fprintf(stderr, "[TcpSocket] socket() failed with WSA error %d\n", WSAGetLastError());
#else
		std::fprintf(stderr, "[TcpSocket] socket() failed: %s\n", std::strerror(errno));
#endif
		freeaddrinfo(result);
		return false;
	}

	setNonBlocking(true);
	setNoDelay(true);

	std::fprintf(stderr, "[TcpSocket] Socket created, attempting connection...\n");

	const int connectResult = ::connect(socket, result->ai_addr, static_cast<int>(result->ai_addrlen));
	freeaddrinfo(result);

	if (connectResult < 0 && !isWouldBlockError())
	{
#if defined(_WIN32)
		const int err = WSAGetLastError();
		std::fprintf(stderr, "[TcpSocket] connect() failed with error %d\n", err);
#else
		std::fprintf(stderr, "[TcpSocket] connect() failed: %s\n", std::strerror(errno));
#endif
		close();
		return false;
	}

	std::fprintf(stderr, "[TcpSocket] Connection initiated (non-blocking mode)\n");
	return true;
}

bool TcpSocket::bindAndListen(uint16_t port)
{
	if (!ensureSocketInitialized())
	{
		std::fprintf(stderr, "[TcpSocket] Failed to initialize socket library\n");
		return false;
	}

	if (socket != InvalidSocketHandle)
	{
		return true;
	}

	std::fprintf(stderr, "[TcpSocket] Creating server socket on port %u\n", port);

	socket = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (socket == InvalidSocketHandle)
	{
#if defined(_WIN32)
		std::fprintf(stderr, "[TcpSocket] socket() failed with WSA error %d\n", WSAGetLastError());
#else
		std::fprintf(stderr, "[TcpSocket] socket() failed: %s\n", std::strerror(errno));
#endif
		return false;
	}

	setNonBlocking(true);

	int reuse = 1;
	setsockopt(socket, SOL_SOCKET, SO_REUSEADDR, reinterpret_cast<const char*>(&reuse), sizeof(reuse));

	sockaddr_in addr{};
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = INADDR_ANY;
	addr.sin_port = htons(port);

	if (::bind(socket, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) < 0)
	{
#if defined(_WIN32)
		const int err = WSAGetLastError();
		if (err == WSAEADDRINUSE)
		{
			std::fprintf(stderr, "[TcpSocket] Port %u is already in use (WSAEADDRINUSE)\n", port);
		}
		else
		{
			std::fprintf(stderr, "[TcpSocket] bind() failed with error %d\n", err);
		}
#else
		if (errno == EADDRINUSE)
		{
			std::fprintf(stderr, "[TcpSocket] Port %u is already in use (EADDRINUSE)\n", port);
		}
		else
		{
			std::fprintf(stderr, "[TcpSocket] bind() failed: %s\n", std::strerror(errno));
		}
#endif
		close();
		return false;
	}

	if (::listen(socket, 1) < 0)
	{
#if defined(_WIN32)
		std::fprintf(stderr, "[TcpSocket] listen() failed with WSA error %d\n", WSAGetLastError());
#else
		std::fprintf(stderr, "[TcpSocket] listen() failed: %s\n", std::strerror(errno));
#endif
		close();
		return false;
	}

	std::fprintf(stderr, "[TcpSocket] Server listening on port %u\n", port);
	return true;
}

bool TcpSocket::acceptClient(TcpSocket& outClient)
{
	if (socket == InvalidSocketHandle)
	{
		return false;
	}

	sockaddr_in clientAddr{};
#if defined(_WIN32)
	int len = sizeof(clientAddr);
#else
	socklen_t len = sizeof(clientAddr);
#endif
	SocketHandle client = ::accept(socket, reinterpret_cast<sockaddr*>(&clientAddr), &len);
	if (client == InvalidSocketHandle)
	{
		return false;
	}

	outClient.close();
	outClient.socket = client;
	outClient.setNonBlocking(true);
	outClient.setNoDelay(true);
	return true;
}

int TcpSocket::sendBytes(const void* data, size_t size)
{
	if (socket == InvalidSocketHandle)
	{
		return -1;
	}

	const int bytes = ::send(socket, reinterpret_cast<const char*>(data), static_cast<int>(size), 0);
	if (bytes <= 0)
	{
		if (isWouldBlockError())
		{
			return 0;
		}

		close();
		return -1;
	}

	return bytes;
}

int TcpSocket::receiveBytes(void* data, size_t size)
{
	if (socket == InvalidSocketHandle)
	{
		return -1;
	}

	const int bytes = ::recv(socket, reinterpret_cast<char*>(data), static_cast<int>(size), 0);
	if (bytes > 0)
	{
		return bytes;
	}

	if (bytes == 0)
	{
		close();
		return -1;
	}

	if (isWouldBlockError())
	{
		return 0;
	}

	close();
	return -1;
}

bool TcpSocket::isValid() const
{
	return socket != InvalidSocketHandle;
}

void TcpSocket::close()
{
	closeSocket(socket);
}

bool TcpSocket::setNonBlocking(bool nonBlocking)
{
	if (socket == InvalidSocketHandle)
	{
		return false;
	}
	 
#if defined(_WIN32)
	u_long mode = nonBlocking ? 1 : 0;
	return ioctlsocket(socket, FIONBIO, &mode) == 0;
#else
	const int flags = fcntl(socket, F_GETFL, 0);
	if (flags < 0)
	{
		return false;
	}
	return fcntl(socket, F_SETFL, nonBlocking ? (flags | O_NONBLOCK) : (flags & ~O_NONBLOCK)) == 0;
#endif
}

bool TcpSocket::setNoDelay(bool noDelay)
{
	if (socket == InvalidSocketHandle)
	{
		return false;
	}

	const int flag = noDelay ? 1 : 0;
	return setsockopt(socket, IPPROTO_TCP, TCP_NODELAY, reinterpret_cast<const char*>(&flag), sizeof(flag)) == 0;
}

int TcpSocket::checkConnectStatus() const
{
	if (socket == InvalidSocketHandle)
	{
		return -1;
	}

	fd_set writeSet;
	fd_set errorSet;
	FD_ZERO(&writeSet);
	FD_ZERO(&errorSet);
	FD_SET(socket, &writeSet);
	FD_SET(socket, &errorSet);

	timeval tv{};
#if defined(_WIN32)
	const int nfds = 0;
#else
	const int nfds = static_cast<int>(socket) + 1;
#endif
	const int result = ::select(nfds, nullptr, &writeSet, &errorSet, &tv);

	if (result < 0)
	{
#if defined(_WIN32)
		std::fprintf(stderr, "[TcpSocket] select() failed with WSA error %d\n", WSAGetLastError());
#else
		std::fprintf(stderr, "[TcpSocket] select() failed: %s\n", std::strerror(errno));
#endif
		return -1;
	}

	if (result == 0)
	{
		return 0;
	}

	if (FD_ISSET(socket, &errorSet))
	{
		int error = 0;
#if defined(_WIN32)
		int len = sizeof(error);
#else
		socklen_t len = sizeof(error);
#endif
		getsockopt(socket, SOL_SOCKET, SO_ERROR, reinterpret_cast<char*>(&error), &len);
		std::fprintf(stderr, "[TcpSocket] Connection error detected: %d\n", error);
		return -1;
	}

	if (FD_ISSET(socket, &writeSet))
	{
		int error = 0;
#if defined(_WIN32)
		int len = sizeof(error);
#else
		socklen_t len = sizeof(error);
#endif
		getsockopt(socket, SOL_SOCKET, SO_ERROR, reinterpret_cast<char*>(&error), &len);
		if (error == 0)
		{
			std::fprintf(stderr, "[TcpSocket] Connection established successfully\n");
			return 1;
		}
		else
		{
			std::fprintf(stderr, "[TcpSocket] Connection failed with socket error %d\n", error);
			return -1;
		}
	}

	return 0;
}

bool TcpClient::connectToServer(const std::string& address, uint16_t port)
{
	if (!socket.connectTo(address, port))
	{
		connectionState = ConnectionState::Failed;
		return false;
	}
	connectionState = ConnectionState::Connecting;
	return true;
}

void TcpClient::disconnect()
{
	connectionState = ConnectionState::Idle;
	socket.close();
	receiveBuffer.clear();
}

bool TcpClient::isConnected() const
{
	return connectionState == ConnectionState::Connected && socket.isValid();
}

bool TcpClient::isConnecting() const
{
	return connectionState == ConnectionState::Connecting;
}

bool TcpClient::hasConnectionFailed() const
{
	return connectionState == ConnectionState::Failed;
}

void TcpClient::pollConnection()
{
	if (connectionState != ConnectionState::Connecting)
	{
		return;
	}

	const int status = socket.checkConnectStatus();
	if (status == 1)
	{
		connectionState = ConnectionState::Connected;
	}
	else if (status == -1)
	{
		socket.close();
		connectionState = ConnectionState::Failed;
	}
}

bool TcpClient::sendPacket(const NetMessagePacket& packet)
{
	if (!isConnected())
	{
		return false;
	}

	const uint8_t* data = reinterpret_cast<const uint8_t*>(&packet);
	int totalSent = 0;
	while (totalSent < static_cast<int>(sizeof(packet)))
	{
		const int sent = socket.sendBytes(data + totalSent, sizeof(packet) - totalSent);
		if (sent < 0)
		{
			disconnect();
			return false;
		}
		if (sent == 0)
		{
			return false;
		}
		totalSent += sent;
	}

	return true;
}

bool TcpClient::receivePacket(NetMessagePacket& outPacket)
{
	if (!isConnected())
	{
		return false;
	}

	if (receiveBuffer.size() < sizeof(NetMessagePacket))
	{
		uint8_t temp[128];
		const int bytes = socket.receiveBytes(temp, sizeof(temp));
		if (bytes < 0)
		{
			disconnect();
			return false;
		}

		if (bytes == 0)
		{
			return false;
		}

		receiveBuffer.insert(receiveBuffer.end(), temp, temp + bytes);
	}

	if (receiveBuffer.size() < sizeof(NetMessagePacket))
	{
		return false;
	}

	std::memcpy(&outPacket, receiveBuffer.data(), sizeof(NetMessagePacket));
	receiveBuffer.erase(receiveBuffer.begin(), receiveBuffer.begin() + sizeof(NetMessagePacket));
	return true;
}

bool TcpServer::start(uint16_t port)
{
	clientConnected = false;
	receiveBuffer.clear();
	return listenSocket.bindAndListen(port);
}

void TcpServer::stop()
{
	clientConnected = false;
	listenSocket.close();
	clientSocket.close();
	receiveBuffer.clear();
}

void TcpServer::pollAccept()
{
	if (clientConnected)
	{
		return;
	}

	if (listenSocket.acceptClient(clientSocket))
	{
		clientConnected = true;
	}
}

bool TcpServer::isClientConnected() const
{
	return clientConnected && clientSocket.isValid();
}

bool TcpServer::sendPacket(const NetMessagePacket& packet)
{
	if (!isClientConnected())
	{
		return false;
	}

	const uint8_t* data = reinterpret_cast<const uint8_t*>(&packet);
	int totalSent = 0;
	while (totalSent < static_cast<int>(sizeof(packet)))
	{
		const int sent = clientSocket.sendBytes(data + totalSent, sizeof(packet) - totalSent);
		if (sent < 0)
		{
			clientConnected = false;
			clientSocket.close();
			return false;
		}
		if (sent == 0)
		{
			return false;
		}
		totalSent += sent;
	}

	return true;
}

bool TcpServer::receivePacket(NetMessagePacket& outPacket)
{
	if (!isClientConnected())
	{
		return false;
	}

	if (receiveBuffer.size() < sizeof(NetMessagePacket))
	{
		uint8_t temp[128];
		const int bytes = clientSocket.receiveBytes(temp, sizeof(temp));
		if (bytes < 0)
		{
			clientConnected = false;
			clientSocket.close();
			receiveBuffer.clear();
			return false;
		}

		if (bytes == 0)
		{
			return false;
		}

		receiveBuffer.insert(receiveBuffer.end(), temp, temp + bytes);
	}

	if (receiveBuffer.size() < sizeof(NetMessagePacket))
	{
		return false;
	}

	std::memcpy(&outPacket, receiveBuffer.data(), sizeof(NetMessagePacket));
	receiveBuffer.erase(receiveBuffer.begin(), receiveBuffer.begin() + sizeof(NetMessagePacket));
	return true;
}

// ---------------------------------------------------------------------------
// NetworkThread implementation (Core 3)
// ---------------------------------------------------------------------------

void NetworkThread::startServer(uint16_t port)
{
	stop();
	m_port = port;
	m_role = Role::Server;
	m_running.store(true);
	m_connected.store(false);
	m_connecting.store(false);
	m_failed.store(false);
	m_clientPresent.store(false);
	m_clientDisconnected.store(false);
	{ std::lock_guard<std::mutex> lk(m_inMutex);  while (!m_inQueue.empty())  m_inQueue.pop();  }
	{ std::lock_guard<std::mutex> lk(m_outMutex); while (!m_outQueue.empty()) m_outQueue.pop(); }
	m_thread = std::thread(&NetworkThread::serverLoop, this);
}

void NetworkThread::startClient(const std::string& address, uint16_t port)
{
	stop();
	m_address = address;
	m_port    = port;
	m_role    = Role::Client;
	m_running.store(true);
	m_connected.store(false);
	m_connecting.store(true);   // set before thread starts so callers see it immediately
	m_failed.store(false);
	m_clientPresent.store(false);
	m_clientDisconnected.store(false);
	{ std::lock_guard<std::mutex> lk(m_inMutex);  while (!m_inQueue.empty())  m_inQueue.pop();  }
	{ std::lock_guard<std::mutex> lk(m_outMutex); while (!m_outQueue.empty()) m_outQueue.pop(); }
	m_thread = std::thread(&NetworkThread::clientLoop, this);
}

void NetworkThread::stop()
{
	m_running.store(false);
	if (m_thread.joinable())
		m_thread.join();
	m_role = Role::None;
}

void NetworkThread::pushOutgoing(const NetMessagePacket& packet)
{
	std::lock_guard<std::mutex> lk(m_outMutex);
	m_outQueue.push(packet);
}

bool NetworkThread::popIncoming(NetMessagePacket& outPacket)
{
	std::lock_guard<std::mutex> lk(m_inMutex);
	if (m_inQueue.empty())
		return false;
	outPacket = m_inQueue.front();
	m_inQueue.pop();
	return true;
}

void NetworkThread::serverLoop()
{
	m_server.start(m_port);

	while (m_running.load())
	{
		if (!m_clientPresent.load())
		{
			m_server.pollAccept();
			if (m_server.isClientConnected())
				m_clientPresent.store(true);
		}

		if (m_clientPresent.load())
		{
			// Drain incoming socket → inbound queue
			NetMessagePacket pkt{};
			bool socketAlive = true;
			while (m_server.receivePacket(pkt))
			{
				std::lock_guard<std::mutex> lk(m_inMutex);
				m_inQueue.push(pkt);
			}
			// If client socket dropped while we were reading, server marks it not connected
			if (!m_server.isClientConnected())
			{
				m_clientPresent.store(false);
				m_clientDisconnected.store(true);
				socketAlive = false;
				// Discard any queued outgoing packets meant for the old client
				std::lock_guard<std::mutex> lk(m_outMutex);
				while (!m_outQueue.empty()) m_outQueue.pop();
			}

			if (socketAlive)
			{
				// Drain outbound queue → socket
				std::lock_guard<std::mutex> lk(m_outMutex);
				while (!m_outQueue.empty())
				{
					m_server.sendPacket(m_outQueue.front());
					m_outQueue.pop();
				}
			}
		}

		std::this_thread::sleep_for(std::chrono::milliseconds(1));
	}

	m_server.stop();
}

void NetworkThread::clientLoop()
{
	if (!m_client.connectToServer(m_address, m_port))
	{
		m_failed.store(true);
		m_connecting.store(false);
		return;
	}

	while (m_running.load())
	{
		if (!m_connected.load())
		{
			m_client.pollConnection();
			if (m_client.hasConnectionFailed())
			{
				m_failed.store(true);
				m_connecting.store(false);
				return;
			}
			if (m_client.isConnected())
			{
				m_connected.store(true);
				m_connecting.store(false);
			}
		}
		else
		{
			// Drain incoming socket → inbound queue
			NetMessagePacket pkt{};
			while (m_client.receivePacket(pkt))
			{
				std::lock_guard<std::mutex> lk(m_inMutex);
				m_inQueue.push(pkt);
			}
			// If the host dropped the connection
			if (!m_client.isConnected())
			{
				m_connected.store(false);
				m_clientDisconnected.store(true);
				return;
			}

			// Drain outbound queue → socket
			std::lock_guard<std::mutex> lk(m_outMutex);
			while (!m_outQueue.empty())
			{
				m_client.sendPacket(m_outQueue.front());
				m_outQueue.pop();
			}
		}

		std::this_thread::sleep_for(std::chrono::milliseconds(1));
	}

	m_client.disconnect();
}