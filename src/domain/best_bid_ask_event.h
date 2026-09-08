#pragma once

#include <cstdint>
#include <string>

namespace domain {

struct BestBidAskEvent {
    std::int64_t event_time;      // microseconds since epoch
    std::int64_t book_update_id;
    std::string symbol;
    double bid_price;
    double bid_qty;
    double ask_price;
    double ask_qty;
};

} // namespace domain
