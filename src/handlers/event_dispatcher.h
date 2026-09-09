#pragma once

#include "handlers/best_bid_ask_handler.h"
#include "handlers/depth_handler.h"
#include "handlers/trade_handler.h"
#include "sbe/decoder.h"

namespace handlers {

// Routes a decoded SBE event to the handler for its concrete model type.
class EventDispatcher {
public:
    void handle(const SBEDecoder::DecodedEvent& event);

private:
    TradeHandler trade_handler_;
    BestBidAskHandler best_bid_ask_handler_;
    DepthHandler depth_handler_;
};

} // namespace handlers
