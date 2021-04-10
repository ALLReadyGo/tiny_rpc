#include <boost/asio.hpp>

using namespace boost::asio;

int main(int argc, char const *argv[])
{
    boost::asio::io_service ios;
    ip::tcp::acceptor accep(ios, ip::tcp::endpoint(ip::address::from_string("172.21.4.217"), 8000));
    while(true)
    {
        ip::tcp::socket sock(ios);
        accep.accept(sock);
        sock.close();
    }
    return 0;
}


