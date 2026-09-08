#include <cstdlib>
#include <cstddef>
#include <iostream>
#include <string_view>
#include <type_traits>

#include "net/websocket_client.h"
#include "sbe/decoder.h"
#include "util/env.h"
#include "util/logging.h"

#define HOST "stream-sbe.binance.com"
#define PORT "9443"

int main(int argc, char** argv) {
    util::load_dotenv();

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
        std::visit([](auto&& event) {
            using T = std::decay_t<decltype(event)>;
            if constexpr (std::is_same_v<T, std::vector<domain::TradeEvent>>) {
                for (const auto& trade : event) {
                    util::log_info(
                        trade.symbol + " trade " + std::to_string(trade.trade_id) +
                        ": price=" + std::to_string(trade.price) +
                        " qty=" + std::to_string(trade.quantity) +
                        " buyerMaker=" + std::to_string(trade.is_buyer_maker));
                }
            } else if constexpr (std::is_same_v<T, domain::BestBidAskEvent>) {
                util::log_info(
                    event.symbol + " bestBidAsk bid=" + std::to_string(event.bid_price) +
                    "@" + std::to_string(event.bid_qty) +
                    " ask=" + std::to_string(event.ask_price) +
                    "@" + std::to_string(event.ask_qty));
            } else if constexpr (std::is_same_v<T, domain::DepthDiffEvent>) {
                util::log_info(
                    event.symbol + " depthDiff bids=" + std::to_string(event.bids.size()) +
                    " asks=" + std::to_string(event.asks.size()));
            } else if constexpr (std::is_same_v<T, domain::DepthSnapshotEvent>) {
                util::log_info(
                    event.symbol + " depthSnapshot bids=" + std::to_string(event.bids.size()) +
                    " asks=" + std::to_string(event.asks.size()));
            }
        }, decoder.decode(data, length));
    });

    try {
        client.connect("/stream?streams=btcusdt@trade/btcusdt@bestBidAsk/btcusdt@depth/btcusdt@depth20");
        client.run();
    } catch (const std::exception& e) {
        util::log_error(e.what());
        return 1;
    }

    return 0;
}