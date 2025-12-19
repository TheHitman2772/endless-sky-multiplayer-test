/* ServerMain.cpp
 * Copyright (c) 2025 by Endless Sky Development Team
 *
 * Endless Sky is free software: you can redistribute it and/or modify it under the
 * terms of the GNU General Public License as published by the Free Software
 * Foundation, either version 3 of the License, or (at your option) any later version.
 *
 * Endless Sky is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
 * PARTICULAR PURPOSE. See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program. If not, see <https://www.gnu.org/licenses/>.
 */

#include "Server.h"
#include "ServerConfig.h"

#include <iostream>
#include <thread>
#include <csignal>
#include <atomic>

using namespace std;


// Global server instance for signal handling
static Server *g_server = nullptr;
static atomic<bool> g_shutdown(false);


// Signal handler for graceful shutdown (Ctrl+C)
void SignalHandler(int signal)
{
	if(signal == SIGINT || signal == SIGTERM)
	{
		cout << "\nShutdown signal received..." << endl;
		g_shutdown = true;

		if(g_server)
			g_server->Stop();
	}
}



// Console input thread
void ConsoleInputThread(Server *server)
{
	string line;

	while(!g_shutdown && server->IsRunning())
	{
		cout << "> " << flush;

		if(!getline(cin, line))
			break;

		if(line.empty())
			continue;

		if(line == "quit" || line == "exit" || line == "shutdown")
		{
			server->ExecuteConsoleCommand("shutdown");
			break;
		}

		server->ExecuteConsoleCommand(line);
	}
}



// Print usage information
void PrintUsage(const char *programName)
{
	cout << "Endless Sky Dedicated Server\n" << endl;
	cout << "Usage:" << endl;
	cout << "  " << programName << " [options]" << endl;
	cout << "\nOptions:" << endl;
	cout << "  --config <file>    Load configuration from file" << endl;
	cout << "  --port <port>      Server port (default: 31337)" << endl;
	cout << "  --name <name>      Server name" << endl;
	cout << "  --max-players <n>  Maximum players (default: 32)" << endl;
	cout << "  --no-console       Disable console interface" << endl;
	cout << "  --help             Show this help" << endl;
	cout << endl;
}



int main(int argc, char *argv[])
{
	cout << "==================================" << endl;
	cout << "Endless Sky Dedicated Server" << endl;
	cout << "Version: Alpha 0.1.0" << endl;
	cout << "==================================" << endl;
	cout << endl;

	// Parse command line arguments
	ServerConfig config;
	string configFile;
	bool enableConsole = true;

	for(int i = 1; i < argc; ++i)
	{
		string arg = argv[i];

		if(arg == "--help" || arg == "-h")
		{
			PrintUsage(argv[0]);
			return 0;
		}
		else if(arg == "--config" && i + 1 < argc)
		{
			configFile = argv[++i];
		}
		else if(arg == "--port" && i + 1 < argc)
		{
			config.SetPort(static_cast<uint16_t>(stoi(argv[++i])));
		}
		else if(arg == "--name" && i + 1 < argc)
		{
			config.SetServerName(argv[++i]);
		}
		else if(arg == "--max-players" && i + 1 < argc)
		{
			config.SetMaxPlayers(stoul(argv[++i]));
		}
		else if(arg == "--no-console")
		{
			enableConsole = false;
		}
		else
		{
			cerr << "Unknown argument: " << arg << endl;
			PrintUsage(argv[0]);
			return 1;
		}
	}

	// Load configuration file if specified
	if(!configFile.empty())
	{
		cout << "Loading configuration from: " << configFile << endl;
		if(!config.LoadFromFile(configFile))
		{
			cout << "Warning: Could not load config file, using defaults" << endl;
		}
	}

	// Override console setting
	config.SetConsoleEnabled(enableConsole);

	// Validate configuration
	if(!config.IsValid())
	{
		cerr << "Invalid server configuration!" << endl;
		return 1;
	}

	// Create server
	Server server;
	g_server = &server;

	// Install signal handlers
	signal(SIGINT, SignalHandler);
	signal(SIGTERM, SignalHandler);

	// Initialize server
	cout << "Initializing server..." << endl;
	if(!server.Initialize(config))
	{
		cerr << "Failed to initialize server!" << endl;
		return 1;
	}

	// Start server
	cout << "Starting server..." << endl;
	if(!server.Start())
	{
		cerr << "Failed to start server!" << endl;
		return 1;
	}

	// Start console input thread if enabled
	thread consoleThread;
	if(config.IsConsoleEnabled())
	{
		cout << "\nServer console active. Type 'help' for commands.\n" << endl;
		consoleThread = thread(ConsoleInputThread, &server);
	}

	// Run server loop (blocks until shutdown)
	server.Run();

	// Wait for console thread to finish
	if(consoleThread.joinable())
		consoleThread.join();

	// Cleanup
	cout << "Server shutdown complete" << endl;

	return 0;
}
