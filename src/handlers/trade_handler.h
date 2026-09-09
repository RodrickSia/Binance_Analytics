#pragma once

#include <vector>

#include "domain/trade_event.h"

namespace handlers {

class TradeHandler {
public:
    void handle_trades(const std::vector<domain::TradeEvent>& trades);
};

} // namespace handlers
