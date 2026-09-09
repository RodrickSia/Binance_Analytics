#include "best_bid_ask_handler.h"

#include "util/logging.h"

namespace handlers {

void BestBidAskHandler::handle_best_bid_ask(const domain::BestBidAskEvent& event) {
    util::log_info(
        event.symbol + " bestBidAsk bid=" + std::to_string(event.bid_price) +
        "@" + std::to_string(event.bid_qty) +
        " ask=" + std::to_string(event.ask_price) +
        "@" + std::to_string(event.ask_qty));
}

} // namespace handlers
