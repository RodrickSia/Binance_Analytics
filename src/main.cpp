#include <cstdlib>
#include <cstddef>
#include <iostream>
#include <string_view>

#include "handlers/event_dispatcher.h"
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
    handlers::EventDispatcher dispatcher;

    client.set_text_handler([](std::string_view text) {
        util::log_info("text frame received");
        std::cout << text << std::endl;
    });

    client.set_binary_handler([&](const char* data, std::size_t length) {
        dispatcher.handle(decoder.decode(data, length));
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