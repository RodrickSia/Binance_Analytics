// Schema tests for SBEDecoder: builds real SBE-encoded frames with the generated
// codec's own encoder API, decodes them with SBEDecoder, and asserts every field
// matches the schema definition in schema/spot_stream.xml. No network required.
#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

#include <array>
#include <cstring>

#include "sbe/decoder.h"
#include "spot_stream/BestBidAskStreamEvent.h"
#include "spot_stream/BoolEnum.h"
#include "spot_stream/DepthDiffStreamEvent.h"
#include "spot_stream/DepthSnapshotStreamEvent.h"
#include "spot_stream/MessageHeader.h"
#include "spot_stream/TradesStreamEvent.h"

using Catch::Matchers::WithinAbs;

namespace {

template <typename Message>
void apply_header(Message& msg, char* buffer, std::size_t length) {
    msg.wrapAndApplyHeader(buffer, 0, length);
}

std::vector<char> encode_trades(
    std::int64_t event_time,
    std::int64_t transact_time,
    std::int8_t price_exponent,
    std::int8_t qty_exponent,
    const std::vector<std::tuple<std::int64_t, std::int64_t, std::int64_t, bool>>& trades,
    const std::string& symbol) {
    std::vector<char> buffer(512, 0);
    spot_stream::TradesStreamEvent event;
    apply_header(event, buffer.data(), buffer.size());

    event.eventTime(event_time)
        .transactTime(transact_time)
        .priceExponent(price_exponent)
        .qtyExponent(qty_exponent);

    auto& group = event.tradesCount(static_cast<std::uint32_t>(trades.size()));
    for (const auto& [id, price, qty, is_buyer_maker] : trades) {
        group.next()
            .id(id)
            .price(price)
            .qty(qty)
            .isBuyerMaker(is_buyer_maker ? spot_stream::BoolEnum::Value::True : spot_stream::BoolEnum::Value::False);
    }

    event.putSymbol(symbol.c_str(), static_cast<std::uint8_t>(symbol.size()));
    buffer.resize(event.sbePosition());
    return buffer;
}

std::vector<char> encode_best_bid_ask(
    std::int64_t event_time,
    std::int64_t book_update_id,
    std::int8_t price_exponent,
    std::int8_t qty_exponent,
    std::int64_t bid_price,
    std::int64_t bid_qty,
    std::int64_t ask_price,
    std::int64_t ask_qty,
    const std::string& symbol) {
    std::vector<char> buffer(256, 0);
    spot_stream::BestBidAskStreamEvent event;
    apply_header(event, buffer.data(), buffer.size());

    event.eventTime(event_time)
        .bookUpdateId(book_update_id)
        .priceExponent(price_exponent)
        .qtyExponent(qty_exponent)
        .bidPrice(bid_price)
        .bidQty(bid_qty)
        .askPrice(ask_price)
        .askQty(ask_qty);

    event.putSymbol(symbol.c_str(), static_cast<std::uint8_t>(symbol.size()));
    buffer.resize(event.sbePosition());
    return buffer;
}

std::vector<char> encode_depth_diff(
    std::int64_t event_time,
    std::int64_t first_update_id,
    std::int64_t last_update_id,
    std::int8_t price_exponent,
    std::int8_t qty_exponent,
    const std::vector<std::pair<std::int64_t, std::int64_t>>& bids,
    const std::vector<std::pair<std::int64_t, std::int64_t>>& asks,
    const std::string& symbol) {
    std::vector<char> buffer(512, 0);
    spot_stream::DepthDiffStreamEvent event;
    apply_header(event, buffer.data(), buffer.size());

    event.eventTime(event_time)
        .firstBookUpdateId(first_update_id)
        .lastBookUpdateId(last_update_id)
        .priceExponent(price_exponent)
        .qtyExponent(qty_exponent);

    auto& bid_group = event.bidsCount(static_cast<std::uint16_t>(bids.size()));
    for (const auto& [price, qty] : bids) {
        bid_group.next().price(price).qty(qty);
    }

    auto& ask_group = event.asksCount(static_cast<std::uint16_t>(asks.size()));
    for (const auto& [price, qty] : asks) {
        ask_group.next().price(price).qty(qty);
    }

    event.putSymbol(symbol.c_str(), static_cast<std::uint8_t>(symbol.size()));
    buffer.resize(event.sbePosition());
    return buffer;
}

std::vector<char> encode_depth_snapshot(
    std::int64_t event_time,
    std::int64_t book_update_id,
    std::int8_t price_exponent,
    std::int8_t qty_exponent,
    const std::vector<std::pair<std::int64_t, std::int64_t>>& bids,
    const std::vector<std::pair<std::int64_t, std::int64_t>>& asks,
    const std::string& symbol) {
    std::vector<char> buffer(512, 0);
    spot_stream::DepthSnapshotStreamEvent event;
    apply_header(event, buffer.data(), buffer.size());

    event.eventTime(event_time)
        .bookUpdateId(book_update_id)
        .priceExponent(price_exponent)
        .qtyExponent(qty_exponent);

    auto& bid_group = event.bidsCount(static_cast<std::uint16_t>(bids.size()));
    for (const auto& [price, qty] : bids) {
        bid_group.next().price(price).qty(qty);
    }

    auto& ask_group = event.asksCount(static_cast<std::uint16_t>(asks.size()));
    for (const auto& [price, qty] : asks) {
        ask_group.next().price(price).qty(qty);
    }

    event.putSymbol(symbol.c_str(), static_cast<std::uint8_t>(symbol.size()));
    buffer.resize(event.sbePosition());
    return buffer;
}

} // namespace

TEST_CASE("decode_trade produces schema-correct TradeEvent objects", "[decoder][trade]") {
    auto buffer = encode_trades(
        1725000000000000, 1725000000000100, -2, -4,
        {{555, 6012345, 100000, true}, {556, 6012350, 250000, false}},
        "BTCUSDT");

    SBEDecoder decoder;
    std::vector<domain::TradeEvent> trades = decoder.decode_trade(buffer.data(), buffer.size());

    REQUIRE(trades.size() == 2);

    CHECK(trades[0].event_time == 1725000000000000);
    CHECK(trades[0].transact_time == 1725000000000100);
    CHECK(trades[0].symbol == "BTCUSDT");
    CHECK(trades[0].trade_id == 555);
    CHECK_THAT(trades[0].price, WithinAbs(60123.45, 1e-9));
    CHECK_THAT(trades[0].quantity, WithinAbs(10.0, 1e-9));
    CHECK(trades[0].is_buyer_maker == true);

    CHECK(trades[1].trade_id == 556);
    CHECK_THAT(trades[1].price, WithinAbs(60123.50, 1e-9));
    CHECK_THAT(trades[1].quantity, WithinAbs(25.0, 1e-9));
    CHECK(trades[1].is_buyer_maker == false);
}

TEST_CASE("decode_trade handles an empty trades group with a valid symbol", "[decoder][trade]") {
    auto buffer = encode_trades(1, 2, 0, 0, {}, "ETHUSDT");

    SBEDecoder decoder;
    auto trades = decoder.decode_trade(buffer.data(), buffer.size());

    CHECK(trades.empty());
}

TEST_CASE("decode_best_bid_ask produces schema-correct BestBidAskEvent", "[decoder][best_bid_ask]") {
    auto buffer = encode_best_bid_ask(
        1725000000000000, 42, -2, -3,
        6012345, 1500, 6012350, 2500,
        "BTCUSDT");

    SBEDecoder decoder;
    domain::BestBidAskEvent result = decoder.decode_best_bid_ask(buffer.data(), buffer.size());

    CHECK(result.event_time == 1725000000000000);
    CHECK(result.book_update_id == 42);
    CHECK(result.symbol == "BTCUSDT");
    CHECK_THAT(result.bid_price, WithinAbs(60123.45, 1e-9));
    CHECK_THAT(result.bid_qty, WithinAbs(1.5, 1e-9));
    CHECK_THAT(result.ask_price, WithinAbs(60123.50, 1e-9));
    CHECK_THAT(result.ask_qty, WithinAbs(2.5, 1e-9));
}

TEST_CASE("decode_depth_diff produces schema-correct DepthDiffEvent", "[decoder][depth_diff]") {
    auto buffer = encode_depth_diff(
        1725000000000000, 10, 12, -2, -3,
        {{6012345, 1500}, {6012300, 500}},
        {{6012400, 750}},
        "BTCUSDT");

    SBEDecoder decoder;
    domain::DepthDiffEvent result = decoder.decode_depth_diff(buffer.data(), buffer.size());

    CHECK(result.event_time == 1725000000000000);
    CHECK(result.first_book_update_id == 10);
    CHECK(result.last_book_update_id == 12);
    CHECK(result.symbol == "BTCUSDT");

    REQUIRE(result.bids.size() == 2);
    CHECK_THAT(result.bids[0].price, WithinAbs(60123.45, 1e-9));
    CHECK_THAT(result.bids[0].quantity, WithinAbs(1.5, 1e-9));
    CHECK_THAT(result.bids[1].price, WithinAbs(60123.00, 1e-9));
    CHECK_THAT(result.bids[1].quantity, WithinAbs(0.5, 1e-9));

    REQUIRE(result.asks.size() == 1);
    CHECK_THAT(result.asks[0].price, WithinAbs(60124.00, 1e-9));
    CHECK_THAT(result.asks[0].quantity, WithinAbs(0.75, 1e-9));
}

TEST_CASE("decode_depth_snapshot produces schema-correct DepthSnapshotEvent", "[decoder][depth_snapshot]") {
    auto buffer = encode_depth_snapshot(
        1725000000000000, 99, -2, -3,
        {{6012345, 1500}},
        {{6012400, 750}, {6012450, 250}},
        "ETHUSDT");

    SBEDecoder decoder;
    domain::DepthSnapshotEvent result = decoder.decode_depth_snapshot(buffer.data(), buffer.size());

    CHECK(result.event_time == 1725000000000000);
    CHECK(result.book_update_id == 99);
    CHECK(result.symbol == "ETHUSDT");

    REQUIRE(result.bids.size() == 1);
    CHECK_THAT(result.bids[0].price, WithinAbs(60123.45, 1e-9));

    REQUIRE(result.asks.size() == 2);
    CHECK_THAT(result.asks[0].price, WithinAbs(60124.00, 1e-9));
    CHECK_THAT(result.asks[1].price, WithinAbs(60124.50, 1e-9));
}

TEST_CASE("decode() dispatches on templateId to the matching schema type", "[decoder][dispatch]") {
    SBEDecoder decoder;

    SECTION("TradesStreamEvent") {
        auto buffer = encode_trades(1, 2, 0, 0, {{1, 100, 1, true}}, "BTCUSDT");
        auto decoded = decoder.decode(buffer.data(), buffer.size());
        REQUIRE(std::holds_alternative<std::vector<domain::TradeEvent>>(decoded));
        CHECK(std::get<std::vector<domain::TradeEvent>>(decoded)[0].symbol == "BTCUSDT");
    }

    SECTION("BestBidAskStreamEvent") {
        auto buffer = encode_best_bid_ask(1, 2, 0, 0, 100, 1, 100, 1, "BTCUSDT");
        auto decoded = decoder.decode(buffer.data(), buffer.size());
        REQUIRE(std::holds_alternative<domain::BestBidAskEvent>(decoded));
    }

    SECTION("DepthDiffStreamEvent") {
        auto buffer = encode_depth_diff(1, 2, 3, 0, 0, {}, {}, "BTCUSDT");
        auto decoded = decoder.decode(buffer.data(), buffer.size());
        REQUIRE(std::holds_alternative<domain::DepthDiffEvent>(decoded));
    }

    SECTION("DepthSnapshotStreamEvent") {
        auto buffer = encode_depth_snapshot(1, 2, 0, 0, {}, {}, "BTCUSDT");
        auto decoded = decoder.decode(buffer.data(), buffer.size());
        REQUIRE(std::holds_alternative<domain::DepthSnapshotEvent>(decoded));
    }
}

TEST_CASE("decode() throws on an unknown templateId", "[decoder][error]") {
    std::vector<char> buffer(8, 0);
    spot_stream::MessageHeader header(buffer.data(), 0, buffer.size(), spot_stream::MessageHeader::sbeSchemaVersion());
    header.blockLength(0).templateId(9999).schemaId(1).version(0);

    SBEDecoder decoder;
    CHECK_THROWS_AS(decoder.decode(buffer.data(), buffer.size()), std::runtime_error);
}

TEST_CASE("type-specific decode_* methods reject mismatched templateIds", "[decoder][error]") {
    auto buffer = encode_best_bid_ask(1, 2, 0, 0, 100, 1, 100, 1, "BTCUSDT");

    SBEDecoder decoder;
    CHECK_THROWS_AS(decoder.decode_trade(buffer.data(), buffer.size()), std::runtime_error);
    CHECK_THROWS_AS(decoder.decode_depth_diff(buffer.data(), buffer.size()), std::runtime_error);
    CHECK_THROWS_AS(decoder.decode_depth_snapshot(buffer.data(), buffer.size()), std::runtime_error);
}
