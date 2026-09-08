#include <cstdlib>
#include <cstddef>
#include <iostream>
#include <string_view>

#include "net/websocket_client.h"
#include "sbe/decoder.h"
#include "util/logging.h"

#define HOST "stream-sbe.binance.com"
#define PORT "9443"

int main(int argc, char** argv) {
    const char* api_key = std::getenv("BINANCE_API_KEY");
    if (!api_key) {
        util::log_error("BINANCE_API_KEY environment variable is not set");
        return 1;
    }

    net::WebsocketClient client(HOST, PORT, api_key);
    SBEDecoder decoder;

    client.set_text_handler([](std::string_view text) {
        util::log_info("text frame received");
        std::cout << text << std::endl;
    });

    client.set_binary_handler([&decoder](const char* data, std::size_t length) {
        for (const auto& trade : decoder.decode_trade(data, length)) {
            util::log_info(
                trade.symbol + " trade " + std::to_string(trade.trade_id) +
                ": price=" + std::to_string(trade.price) +
                " qty=" + std::to_string(trade.quantity) +
                " buyerMaker=" + std::to_string(trade.is_buyer_maker));
        }
    });

    try {
        client.connect("/ws/btcusdt@trade");
        client.run();
    } catch (const std::exception& e) {
        util::log_error(e.what());
        return 1;
    }

    return 0;
}