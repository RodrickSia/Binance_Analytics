#pragma once

#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp>
#include <boost/beast.hpp>
#include <boost/beast/websocket/ssl.hpp>
#include <functional>
#include <string>
#include <string_view>

namespace net {

class WebsocketClient {
public:
    using TextHandler = std::function<void(std::string_view)>;
    using BinaryHandler = std::function<void(const char*, std::size_t)>;

    WebsocketClient(std::string host, std::string port, std::string api_key);

    void set_text_handler(TextHandler handler);
    void set_binary_handler(BinaryHandler handler);

    // Resolves, TCP/TLS/websocket-handshakes against target (e.g. "/ws/btcusdt@trade").
    void connect(const std::string& target);

    // Blocking read loop; dispatches frames to the configured handlers until the socket closes.
    void run();

private:
    std::string host_;
    std::string port_;
    std::string api_key_;

    boost::asio::io_context ioc_;
    boost::asio::ssl::context ctx_;
    boost::beast::websocket::stream<boost::asio::ssl::stream<boost::beast::tcp_stream>> wss_;

    TextHandler on_text_;
    BinaryHandler on_binary_;
};

} // namespace net
