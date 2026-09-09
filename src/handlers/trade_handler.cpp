#include "trade_handler.h"

#include "util/logging.h"

namespace handlers {

void TradeHandler::handle_trades(const std::vector<domain::TradeEvent>& trades) {
    for (const auto& trade : trades) {
        util::log_info(
            trade.symbol + " trade " + std::to_string(trade.trade_id) +
            ": price=" + std::to_string(trade.price) +
            " qty=" + std::to_string(trade.quantity) +
            " buyerMaker=" + std::to_string(trade.is_buyer_maker));
    }
}

} // namespace handlers
