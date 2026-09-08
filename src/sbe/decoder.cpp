#include "decoder.h"

#include <cmath>
#include <stdexcept>

#include "spot_stream/BestBidAskStreamEvent.h"
#include "spot_stream/DepthDiffStreamEvent.h"
#include "spot_stream/DepthSnapshotStreamEvent.h"
#include "spot_stream/MessageHeader.h"
#include "spot_stream/TradesStreamEvent.h"

SBEDecoder::DecodedEvent SBEDecoder::decode(const char* data, std::size_t length) {
    char* buffer = const_cast<char*>(data);
    spot_stream::MessageHeader header(buffer, 0, length, spot_stream::MessageHeader::sbeSchemaVersion());

    switch (header.templateId()) {
        case spot_stream::TradesStreamEvent::SBE_TEMPLATE_ID:
            return decode_trade(data, length);
        case spot_stream::BestBidAskStreamEvent::SBE_TEMPLATE_ID:
            return decode_best_bid_ask(data, length);
        case spot_stream::DepthDiffStreamEvent::SBE_TEMPLATE_ID:
            return decode_depth_diff(data, length);
        case spot_stream::DepthSnapshotStreamEvent::SBE_TEMPLATE_ID:
            return decode_depth_snapshot(data, length);
        default:
            throw std::runtime_error("unknown SBE templateId: " + std::to_string(header.templateId()));
    }
}

std::vector<domain::TradeEvent> SBEDecoder::decode_trade(const char* data, std::size_t length) {
    char* buffer = const_cast<char*>(data);

    spot_stream::MessageHeader header(buffer, 0, length, spot_stream::MessageHeader::sbeSchemaVersion());
    if (header.templateId() != spot_stream::TradesStreamEvent::SBE_TEMPLATE_ID) {
        throw std::runtime_error("unexpected SBE templateId: " + std::to_string(header.templateId()));
    }

    spot_stream::TradesStreamEvent event;
    event.wrapForDecode(
        buffer,
        header.encodedLength(),
        header.blockLength(),
        header.version(),
        length);

    const double price_scale = std::pow(10.0, event.priceExponent());
    const double qty_scale = std::pow(10.0, event.qtyExponent());
    const std::int64_t event_time = event.eventTime();
    const std::int64_t transact_time = event.transactTime();

    std::vector<domain::TradeEvent> trades;
    auto& group = event.trades();
    while (group.hasNext()) {
        group.next();
        trades.push_back(domain::TradeEvent{
            event_time,
            transact_time,
            std::string(), // symbol is trailing var-data; filled in below
            group.id(),
            static_cast<double>(group.price()) * price_scale,
            static_cast<double>(group.qty()) * qty_scale,
            group.isBuyerMaker() == spot_stream::BoolEnum::Value::True,
        });
    }

    // Symbol must be read after the group per SBE's sequential-access rule.
    const std::string symbol = event.getSymbolAsString();
    for (auto& trade : trades) {
        trade.symbol = symbol;
    }

    return trades;
}

domain::BestBidAskEvent SBEDecoder::decode_best_bid_ask(const char* data, std::size_t length) {
    char* buffer = const_cast<char*>(data);

    spot_stream::MessageHeader header(buffer, 0, length, spot_stream::MessageHeader::sbeSchemaVersion());
    if (header.templateId() != spot_stream::BestBidAskStreamEvent::SBE_TEMPLATE_ID) {
        throw std::runtime_error("unexpected SBE templateId: " + std::to_string(header.templateId()));
    }

    spot_stream::BestBidAskStreamEvent event;
    event.wrapForDecode(
        buffer,
        header.encodedLength(),
        header.blockLength(),
        header.version(),
        length);

    const double price_scale = std::pow(10.0, event.priceExponent());
    const double qty_scale = std::pow(10.0, event.qtyExponent());

    domain::BestBidAskEvent result{
        event.eventTime(),
        event.bookUpdateId(),
        std::string(), // filled in below
        static_cast<double>(event.bidPrice()) * price_scale,
        static_cast<double>(event.bidQty()) * qty_scale,
        static_cast<double>(event.askPrice()) * price_scale,
        static_cast<double>(event.askQty()) * qty_scale,
    };

    // Symbol must be read after the fixed fields per SBE's sequential-access rule.
    result.symbol = event.getSymbolAsString();
    return result;
}

domain::DepthDiffEvent SBEDecoder::decode_depth_diff(const char* data, std::size_t length) {
    char* buffer = const_cast<char*>(data);

    spot_stream::MessageHeader header(buffer, 0, length, spot_stream::MessageHeader::sbeSchemaVersion());
    if (header.templateId() != spot_stream::DepthDiffStreamEvent::SBE_TEMPLATE_ID) {
        throw std::runtime_error("unexpected SBE templateId: " + std::to_string(header.templateId()));
    }

    spot_stream::DepthDiffStreamEvent event;
    event.wrapForDecode(
        buffer,
        header.encodedLength(),
        header.blockLength(),
        header.version(),
        length);

    const double price_scale = std::pow(10.0, event.priceExponent());
    const double qty_scale = std::pow(10.0, event.qtyExponent());

    domain::DepthDiffEvent result;
    result.event_time = event.eventTime();
    result.first_book_update_id = event.firstBookUpdateId();
    result.last_book_update_id = event.lastBookUpdateId();

    auto& bids = event.bids();
    result.bids.reserve(bids.count());
    while (bids.hasNext()) {
        bids.next();
        result.bids.push_back(domain::PriceLevel{
            static_cast<double>(bids.price()) * price_scale,
            static_cast<double>(bids.qty()) * qty_scale,
        });
    }

    auto& asks = event.asks();
    result.asks.reserve(asks.count());
    while (asks.hasNext()) {
        asks.next();
        result.asks.push_back(domain::PriceLevel{
            static_cast<double>(asks.price()) * price_scale,
            static_cast<double>(asks.qty()) * qty_scale,
        });
    }

    // Symbol must be read after both groups per SBE's sequential-access rule.
    result.symbol = event.getSymbolAsString();
    return result;
}

domain::DepthSnapshotEvent SBEDecoder::decode_depth_snapshot(const char* data, std::size_t length) {
    char* buffer = const_cast<char*>(data);

    spot_stream::MessageHeader header(buffer, 0, length, spot_stream::MessageHeader::sbeSchemaVersion());
    if (header.templateId() != spot_stream::DepthSnapshotStreamEvent::SBE_TEMPLATE_ID) {
        throw std::runtime_error("unexpected SBE templateId: " + std::to_string(header.templateId()));
    }

    spot_stream::DepthSnapshotStreamEvent event;
    event.wrapForDecode(
        buffer,
        header.encodedLength(),
        header.blockLength(),
        header.version(),
        length);

    const double price_scale = std::pow(10.0, event.priceExponent());
    const double qty_scale = std::pow(10.0, event.qtyExponent());

    domain::DepthSnapshotEvent result;
    result.event_time = event.eventTime();
    result.book_update_id = event.bookUpdateId();

    auto& bids = event.bids();
    result.bids.reserve(bids.count());
    while (bids.hasNext()) {
        bids.next();
        result.bids.push_back(domain::PriceLevel{
            static_cast<double>(bids.price()) * price_scale,
            static_cast<double>(bids.qty()) * qty_scale,
        });
    }

    auto& asks = event.asks();
    result.asks.reserve(asks.count());
    while (asks.hasNext()) {
        asks.next();
        result.asks.push_back(domain::PriceLevel{
            static_cast<double>(asks.price()) * price_scale,
            static_cast<double>(asks.qty()) * qty_scale,
        });
    }

    // Symbol must be read after both groups per SBE's sequential-access rule.
    result.symbol = event.getSymbolAsString();
    return result;
}

