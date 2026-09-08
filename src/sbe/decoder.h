#pragma once

#include <cstddef>
#include <vector>

#include "domain/trade_event.h"

// Decodes SBE binary WebSocket frames from Binance's SBE market data streams.
class SBEDecoder {
public:
    // Decodes a TradesStreamEvent frame (templateId 10000, the @trade stream).
    // Throws std::runtime_error if the frame's templateId does not match.
    std::vector<domain::TradeEvent> decode_trade(const char* data, std::size_t length);
};
