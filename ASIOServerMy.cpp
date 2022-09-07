// ASIOServerMy.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>

#define ASIO_STANDALONE
#define _WIN32_WINNT 0x0601

#include <asio.hpp>
#include <asio/ts/buffer.hpp>

#include <mutex>
#include <list>
#include <future>


using asio::ip::tcp;
std::vector<char> vBuffer(1 * 4);
asio::streambuf buf(6553);

std::mutex mt;
std::list<std::string> List;

void SafePush(const std::string & str)
{
	std::lock_guard<std::mutex> gate(mt);
	List.push_back(str);
}

void SafePrint()
{
	std::lock_guard<std::mutex> gate(mt);
	for (auto it:List)
	{
		std::cout << "Client: " << it;
	}
	List.clear();
}

void GrabSomeData(asio::ip::tcp::socket& socket, std::string &str, std::error_code& error)
{
	asio::read_until(socket, buf, "\n", error);;
	if (error) std::cout << error;

	std::stringstream ss;
	ss << std::istream(&buf).rdbuf();

	SafePush(ss.str());
	GrabSomeData(socket,str,  error);
}
void WriteSomeData(asio::ip::tcp::socket& socket, std::string& str, std::error_code& error)
{
	std::cin >> str;
	str += '\n';

	asio::write(socket, asio::buffer(str), error);
	WriteSomeData(socket, str, error);
}

int main()
{
	try
	{
		asio::io_context context;
		tcp::acceptor acceptor(context, tcp::endpoint(tcp::v4(), 1337));

		std::cout << "Accepting connections on port 1337\n";
		tcp::socket socket(context);
		acceptor.accept(socket);

		std::cout << "Client connected\n";
		std::string str = "hello from server\n";
		std::error_code error;


		std::thread read_th(
			[&socket, &str, &error]() 
			{
				GrabSomeData(socket, str, error); 
			}
			);
	///	read_th.detach();

		std::thread write_th(
			[&socket, &str, &error]()
			{
				WriteSomeData(socket, str, error);
			}
		);
	//	write_th.detach();

		asio::write(socket, asio::buffer(str), error);

		while(true)
		{
			SafePrint();
			
			std::this_thread::sleep_for(std::chrono::milliseconds(1000));
		}
		read_th.join();
		write_th.join();
	}
	catch (std::exception exc)
	{
		std::cerr << exc.what();
	}
	catch (...)
	{
		std::cerr << "Error!\n";
	}
		//	std::cin.get();
			
		std::cout << "\nBye";
	return 0;

}

// Run program: Ctrl + F5 or Debug > Start Without Debugging menu
// Debug program: F5 or Debug > Start Debugging menu

// Tips for Getting Started: 
//   1. Use the Solution Explorer window to add/manage files
//   2. Use the Team Explorer window to connect to source control
//   3. Use the Output window to see build output and other messages
//   4. Use the Error List window to view errors
//   5. Go to Project > Add New Item to create new code files, or Project > Add Existing Item to add existing code files to the project
//   6. In the future, to open this project again, go to File > Open > Project and select the .sln file
