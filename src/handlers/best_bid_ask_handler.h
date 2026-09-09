#pragma once

#include "domain/best_bid_ask_event.h"

namespace handlers {

class BestBidAskHandler {
public:
    void handle_best_bid_ask(const domain::BestBidAskEvent& event);
};

} // namespace handlers
