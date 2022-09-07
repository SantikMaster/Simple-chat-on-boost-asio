// ASIOMyClient.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <thread>

#define ASIO_STANDALONE
#define _WIN32_WINNT 0x0601
#include <asio.hpp>
#include <asio/ts/buffer.hpp>

#include <mutex>
#include <list>
using asio::ip::tcp;
asio::streambuf buf(65536);
std::list<std::string> List;
std::mutex mt;

void SafePush(const std::string& str)
{
    std::lock_guard<std::mutex> gate(mt);
    List.push_back(str);
}

void SafePrint()
{
    std::lock_guard<std::mutex> gate(mt);
    for (auto it : List)
    {
        std::cout << "Server: " << it;
    }
    List.clear();
}

void ReadSomeData(asio::ip::tcp::socket& socket, std::string& str, std::error_code& error)
{
    asio::read_until(socket, buf, "\n", error);;
    if (error) std::cout << error;

    std::stringstream ss;
    ss << std::istream(&buf).rdbuf();

    SafePush(ss.str());
    ReadSomeData(socket, str, error);
}

void WriteSomeData(asio::ip::tcp::socket& socket, std::string& str, std::error_code& error)
{ 
    if (error)
        std::cout << error << 'n';

   if (socket.is_open())
    {
        socket.wait(socket.wait_write);
        std::cin >> str;
        str += '\n';

        asio::write(socket, asio::buffer(str), error);
    }
      WriteSomeData(socket, str, error);  
}

int main()
{
    try
    {
        std::cout << "My IP:\n";

        asio::io_service io_service;
        tcp::resolver resolver1(io_service);
        tcp::resolver::query query(asio::ip::host_name(), "");
        tcp::resolver::iterator iter = resolver1.resolve(query);
        tcp::resolver::iterator end; // End marker.

       while (iter != end)
        {
            tcp::endpoint ep = *iter++;
            std::cout << ep << std::endl;
        }


        asio::io_context context;
        tcp::resolver resolver(context);
        std::error_code ec;

        //   auto endpoint = resolver.resolve("192.168.0.103", "1337", ec);
        auto endpoint = resolver.resolve("127.0.0.1", "1337", ec);
        tcp::socket socket(context);
        asio::connect(socket, endpoint, ec);

        std::string str = "hello from client\n";
        asio::write(socket, asio::buffer(str));


        std::thread th_write(
            [&socket]()
            {
                std::string str = "";
                std::error_code error;
                WriteSomeData(socket, str, error);
            }

        );
        th_write.detach();

        std::thread th_read(
            [&socket]()
            {
                std::string str = "";
                std::error_code error;
                ReadSomeData(socket, str, error);
            }

        );
        th_read.detach();
        while (true) 
        {
            SafePrint();
        }
    }
    catch (std::exception exc)
    {
        std::cout << exc.what() << 'n';

    }
    catch (...)
    {
        std::cout << "Error";
    }

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
