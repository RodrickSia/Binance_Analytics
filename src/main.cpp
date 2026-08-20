#include <iostream>
#include <boost/bind/bind.hpp>
#include <boost/asio.hpp>
#include <boost/beast.hpp>
#include <boost/beast/websocket/ssl.hpp>
#include <boost/asio/ssl.hpp>
#include <boost/asio/ip/tcp.hpp>

namespace asio = boost::asio;
namespace beast = boost::beast;
namespace http = beast::http;
using namespace boost::beast;
using namespace boost::beast::websocket;



#define HOST "stream-sbe.binance.com"
#define PORT "9443"


int main(int argc, char** argv) {
    const char* api_key = std::getenv("BINANCE_API_KEY");
    if (!api_key) {
        std::cerr << "BINANCE_API_KEY environment variable is not set" << std::endl;
        return 1;
    }

    // Init the io context, tcp client, tcp resolver and the ssl objects

    asio::io_context ioc;
    tcp_stream sock(ioc);
    asio::ip::tcp::resolver resolver(ioc);
    asio::ssl::context ctx(asio::ssl::context::tlsv12);

    // create the wss object
    stream<asio::ssl::stream<tcp_stream>> wss(asio::make_strand(ioc), ctx);

    // TLS handshake
    // get the ip of the domain and port
    // Make connect to the resolved list of ips
    auto const results = resolver.resolve(HOST, PORT);
    beast::get_lowest_layer(wss).connect(results);
    
    // TCP handshake
    wss.next_layer().handshake(asio::ssl::stream_base::client);

    // Do the websocket handsahke
    std::string connection_string = HOST ":" PORT;
    
    // Set header 
    std::cout << api_key;
    wss.set_option(websocket::stream_base::decorator(
    [connection_string, api_key](websocket::request_type& req)
    {
        req.set(http::field::user_agent, "websocket-client");
        req.set("X-MBX-APIKEY", api_key);
    }));
    
    // headache
    websocket::response_type res;
    try {
        wss.handshake(res, connection_string, "/ws/btcusdt@trade");
    } catch (const std::exception&) {
        std::cerr << "Handshake response status: " << res.result_int() << std::endl;
        std::cerr << res << std::endl;
        throw;
    }

    // Consume messages
    beast::flat_buffer buffer;
    while (true) {
        wss.read(buffer);

        if (wss.got_text()) {
            std::cout << beast::buffers_to_string(buffer.data()) << std::endl;
        } else {
            // SBE binary payload; needs schema-generated decoder to interpret
            std::cout << "[binary SBE message, " << buffer.size() << " bytes]" << std::endl;
        }

        buffer.consume(buffer.size());
    }

    return 0;
}