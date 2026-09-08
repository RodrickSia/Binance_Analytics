#pragma once

#include <cstddef>
#include <variant>
#include <vector>

#include "domain/best_bid_ask_event.h"
#include "domain/depth_diff_event.h"
#include "domain/depth_snapshot_event.h"
#include "domain/trade_event.h"

// Decodes SBE binary WebSocket frames from Binance's SBE market data streams.
class SBEDecoder {
public:
    using DecodedEvent = std::variant<
        std::vector<domain::TradeEvent>,
        domain::BestBidAskEvent,
        domain::DepthDiffEvent,
        domain::DepthSnapshotEvent>;

    // Peeks the frame's templateId and dispatches to the matching decode_* method below.
    // Throws std::runtime_error if the templateId is not one of the 4 known message types.
    DecodedEvent decode(const char* data, std::size_t length);

    // Decodes a TradesStreamEvent frame (templateId 10000, the @trade stream).
    std::vector<domain::TradeEvent> decode_trade(const char* data, std::size_t length);

    // Decodes a BestBidAskStreamEvent frame (templateId 10001, the @bestBidAsk stream).
    domain::BestBidAskEvent decode_best_bid_ask(const char* data, std::size_t length);

    // Decodes a DepthDiffStreamEvent frame (templateId 10003, the @depth stream).
    domain::DepthDiffEvent decode_depth_diff(const char* data, std::size_t length);

    // Decodes a DepthSnapshotStreamEvent frame (templateId 10002, the @depth20 stream).
    domain::DepthSnapshotEvent decode_depth_snapshot(const char* data, std::size_t length);
};

