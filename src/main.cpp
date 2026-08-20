#include <cstdlib>
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
namespace websocket = boost::beast::websocket;
using namespace boost::beast;
using namespace boost::beast::websocket;

namespace {
void log_info(const std::string& message) {
    std::clog << "[info] " << message << std::endl;
}

void log_error(const std::string& message) {
    std::clog << "[error] " << message << std::endl;
}

void log_debug(const std::string& message) {
    std::clog << "[debug] " << message << std::endl;
}
}



#define HOST "stream-sbe.binance.com"
#define PORT "9443"


int main(int argc, char** argv) {
    const char* api_key = std::getenv("BINANCE_API_KEY");
    if (!api_key) {
        log_error("BINANCE_API_KEY environment variable is not set");
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
    log_info("resolving " HOST ":" PORT);
    auto const results = resolver.resolve(HOST, PORT);
    beast::get_lowest_layer(wss).connect(results);
    log_info("TCP connected");
    
    // TCP handshake
    wss.next_layer().handshake(asio::ssl::stream_base::client);
    log_info("TLS handshake complete");

    // Do the websocket handsahke
    std::string connection_string = HOST ":" PORT;
    
    // Set header 
    log_debug("installing websocket request headers");
    wss.set_option(websocket::stream_base::decorator(
    [connection_string, api_key](websocket::request_type& req)
    {
        req.set(http::field::user_agent, "websocket-client");
        req.set("X-MBX-APIKEY", api_key);
    }));
    
    // headache
    websocket::response_type res;
    try {
        log_info("starting websocket handshake");
        wss.handshake(res, connection_string, "/ws/btcusdt@trade");
        log_info("websocket handshake complete");
    } catch (const std::exception&) {
        log_error("handshake failed with HTTP status " + std::to_string(res.result_int()));
        std::clog << res << std::endl;
        throw;
    }
    
    
    wss.control_callback(
        [](frame_type kind, beast::string_view payload) {
            if (kind == websocket::frame_type::ping) {
                std::clog << "[debug] ping received" << std::endl;
            } else if (kind == websocket::frame_type::pong) {
                std::clog << "[debug] pong received" << std::endl;
            } else if (kind == websocket::frame_type::close) {
                std::clog << "[info] close frame received" << std::endl;
            }
        }
    );
    // Consume messages
    beast::flat_buffer buffer;
    while (true) {
        wss.read(buffer);

        if (wss.got_text()) {
            log_info("text frame received");
            std::cout << beast::buffers_to_string(buffer.data()) << std::endl;
        } else {
            // SBE binary payload; needs schema-generated decoder to interpret
            log_info("binary SBE message received: " + std::to_string(buffer.size()) + " bytes");
        }

        buffer.consume(buffer.size());
    }

    return 0;
}