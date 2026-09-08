#pragma once

#include <cstdint>
#include <string>

namespace domain {

struct TradeEvent {
    std::int64_t event_time;    // microseconds since epoch
    std::int64_t transact_time; // microseconds since epoch
    std::string symbol;
    std::int64_t trade_id;
    double price;
    double quantity;
    bool is_buyer_maker;
};

} // namespace domain
