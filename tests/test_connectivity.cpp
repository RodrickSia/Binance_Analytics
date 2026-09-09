// Connectivity tests: opens a real TLS websocket connection to Binance's public
// SBE market-data endpoint and verifies both the handshake and that live frames
// decode into schema-valid objects. Requires network access and BINANCE_API_KEY;
// skipped otherwise. Not part of the default `ctest` run (label "integration").
#include <catch2/catch_test_macros.hpp>

#include <cstdlib>
#include <optional>
#include <variant>

#include "net/websocket_client.h"
#include "sbe/decoder.h"

namespace {

constexpr const char* kHost = "stream-sbe.binance.com";
constexpr const char* kPort = "9443";
constexpr const char* kTarget = "/stream?streams=btcusdt@trade";
constexpr int kMaxFramesToInspect = 50;

std::optional<std::string> api_key() {
    if (const char* value = std::getenv("BINANCE_API_KEY")) {
        return std::string(value);
    }
    return std::nullopt;
}

} // namespace

TEST_CASE("WebsocketClient completes a TLS handshake against Binance's SBE endpoint", "[integration][connectivity]") {
    auto key = api_key();
    if (!key) {
        SKIP("BINANCE_API_KEY not set; skipping live connectivity test");
    }

    net::WebsocketClient client(kHost, kPort, *key);
    REQUIRE_NOTHROW(client.connect(kTarget));
}

TEST_CASE("A live trade frame decodes into a schema-valid TradeEvent", "[integration][connectivity][schema]") {
    auto key = api_key();
    if (!key) {
        SKIP("BINANCE_API_KEY not set; skipping live connectivity test");
    }

    net::WebsocketClient client(kHost, kPort, *key);
    SBEDecoder decoder;

    std::vector<domain::TradeEvent> received_trades;
    client.set_binary_handler([&](const char* data, std::size_t length) {
        auto decoded = decoder.decode(data, length);
        if (std::holds_alternative<std::vector<domain::TradeEvent>>(decoded)) {
            auto trades = std::get<std::vector<domain::TradeEvent>>(decoded);
            received_trades.insert(received_trades.end(), trades.begin(), trades.end());
        }
    });

    REQUIRE_NOTHROW(client.connect(kTarget));

    for (int i = 0; i < kMaxFramesToInspect && received_trades.empty(); ++i) {
        REQUIRE_NOTHROW(client.read_one());
    }

    REQUIRE_FALSE(received_trades.empty());

    const auto& trade = received_trades.front();
    CHECK(trade.symbol == "BTCUSDT");
    CHECK(trade.trade_id > 0);
    CHECK(trade.price > 0.0);
    CHECK(trade.quantity > 0.0);
    CHECK(trade.event_time > 0);
    CHECK(trade.transact_time > 0);
}
