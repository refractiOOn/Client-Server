#include "HandleWrapper.h"
#include <iostream>
#include <vector>

struct Server
{
	Server()
	{
		InitializeCriticalSection(&m_criticalSection);
	}
	~Server()
	{
		DeleteCriticalSection(&m_criticalSection);
	}
	std::vector<size_t> m_freeRandoms;
	std::vector<size_t> m_usedRandoms;
	bool m_freeRandomsLocked = true;
	CRITICAL_SECTION m_criticalSection;
	const size_t m_randomsInPack = 5;
	size_t m_clientsRequested = 1;
};

DWORD __stdcall Client(LPVOID args)
{
	Server* server = static_cast<Server*>(args);
	EnterCriticalSection(&(server->m_criticalSection));
	if (server->m_freeRandoms.empty())
	{
		if (!server->m_freeRandomsLocked)
		{
			server->m_freeRandomsLocked = true;
		}
		LeaveCriticalSection(&(server->m_criticalSection));
		while (server->m_freeRandoms.size() < server->m_randomsInPack);
		EnterCriticalSection(&(server->m_criticalSection));
	}
	size_t random = server->m_freeRandoms.back();
	server->m_freeRandoms.pop_back();
	server->m_usedRandoms.push_back(random);
	std::cout << "Client got a random " << random << std::endl;
	LeaveCriticalSection(&(server->m_criticalSection));
	return 0;
}

DWORD __stdcall ServerThread(LPVOID args)
{
	Server* server = static_cast<Server*>(args);
	while (server->m_clientsRequested)
	{
		EnterCriticalSection(&(server->m_criticalSection));
		if (server->m_freeRandomsLocked)
		{
			for (int i = 0; i < server->m_randomsInPack;)
			{
				srand(time(0));
				size_t random = 11111 + rand() % 99999;
				bool isUnique = true;
				for (std::vector<size_t>::const_iterator it = server->m_freeRandoms.cbegin(); it != server->m_freeRandoms.cend(); ++it)
				{
					if (*it == random)
					{
						isUnique = false;
					}
				}
				for (std::vector<size_t>::const_iterator it = server->m_usedRandoms.cbegin(); it != server->m_usedRandoms.cend(); ++it)
				{
					if (*it == random)
					{
						isUnique = false;
					}
				}
				if (isUnique)
				{
					server->m_freeRandoms.push_back(random);
					++i;
				}
			}
			server->m_freeRandomsLocked = false;
		}
		LeaveCriticalSection(&(server->m_criticalSection));
	}
	return 0;
}

int main()
{
	Server server;
	std::vector<HANDLE> clients;
	HandleWrapper serverThread = CreateThread(0, 0, ServerThread, &server, 0, 0);
	while (server.m_clientsRequested)
	{
		for (size_t i = 0; i < server.m_clientsRequested; ++i)
		{
			clients.push_back(CreateThread(0, 0, Client, &server, 0, 0));
			Sleep(200);
			WaitForSingleObject(clients.back(), INFINITE);
		}
		std::cin >> server.m_clientsRequested;
	}
	WaitForSingleObject(serverThread.Get(), INFINITE);
	for (size_t i = 0; i < clients.size(); ++i)
	{
		CloseHandle(clients[i]);
	}
}