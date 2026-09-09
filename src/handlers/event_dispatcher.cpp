#include "event_dispatcher.h"

#include <type_traits>

namespace handlers {

void EventDispatcher::handle(const SBEDecoder::DecodedEvent& event) {
    std::visit([this](auto&& e) {
        using T = std::decay_t<decltype(e)>;
        if constexpr (std::is_same_v<T, std::vector<domain::TradeEvent>>) {
            trade_handler_.handle_trades(e);
        } else if constexpr (std::is_same_v<T, domain::BestBidAskEvent>) {
            best_bid_ask_handler_.handle_best_bid_ask(e);
        } else if constexpr (std::is_same_v<T, domain::DepthDiffEvent>) {
            depth_handler_.handle_depth_diff(e);
        } else if constexpr (std::is_same_v<T, domain::DepthSnapshotEvent>) {
            depth_handler_.handle_depth_snapshot(e);
        }
    }, event);
}

} // namespace handlers
