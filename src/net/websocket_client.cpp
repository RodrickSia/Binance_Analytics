#include "websocket_client.h"

#include "../util/logging.h"

namespace asio = boost::asio;
namespace beast = boost::beast;
namespace http = beast::http;
namespace websocket = boost::beast::websocket;

namespace net {

WebsocketClient::WebsocketClient(std::string host, std::string port, std::string api_key)
    : host_(std::move(host)),
      port_(std::move(port)),
      api_key_(std::move(api_key)),
      ctx_(asio::ssl::context::tlsv12),
      wss_(asio::make_strand(ioc_), ctx_) {
}

void WebsocketClient::set_text_handler(TextHandler handler) {
    on_text_ = std::move(handler);
}

void WebsocketClient::set_binary_handler(BinaryHandler handler) {
    on_binary_ = std::move(handler);
}

void WebsocketClient::connect(const std::string& target) {
    asio::ip::tcp::resolver resolver(ioc_);

    util::log_info("resolving " + host_ + ":" + port_);
    auto const results = resolver.resolve(host_, port_);
    beast::get_lowest_layer(wss_).connect(results);
    util::log_info("TCP connected");

    wss_.next_layer().handshake(asio::ssl::stream_base::client);
    util::log_info("TLS handshake complete");

    std::string connection_string = host_ + ":" + port_;

    util::log_debug("installing websocket request headers");
    wss_.set_option(websocket::stream_base::decorator(
        [this](websocket::request_type& req) {
            req.set(http::field::user_agent, "websocket-client");
            req.set("X-MBX-APIKEY", api_key_);
        }));

    websocket::response_type res;
    try {
        util::log_info("starting websocket handshake");
        wss_.handshake(res, connection_string, target);
        util::log_info("websocket handshake complete");
    } catch (const std::exception&) {
        util::log_error("handshake failed with HTTP status " + std::to_string(res.result_int()));
        throw;
    }

    wss_.control_callback(
        [](websocket::frame_type kind, beast::string_view) {
            if (kind == websocket::frame_type::ping) {
                util::log_debug("ping received");
            } else if (kind == websocket::frame_type::pong) {
                util::log_debug("pong received");
            } else if (kind == websocket::frame_type::close) {
                util::log_info("close frame received");
            }
        });
}

void WebsocketClient::run() {
    beast::flat_buffer buffer;
    while (true) {
        wss_.read(buffer);

        if (wss_.got_text()) {
            if (on_text_) {
                on_text_(beast::buffers_to_string(buffer.data()));
            }
        } else if (on_binary_) {
            auto data = buffer.data();
            std::string bytes = beast::buffers_to_string(data);
            on_binary_(bytes.data(), bytes.size());
        }

        buffer.consume(buffer.size());
    }
}

} // namespace net
