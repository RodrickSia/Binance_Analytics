#pragma once

#include <cstdint>
#include <string>
#include <vector>

#include "domain/price_level.h"

namespace domain {

struct DepthSnapshotEvent {
    std::int64_t event_time; // microseconds since epoch
    std::int64_t book_update_id;
    std::string symbol;
    std::vector<PriceLevel> bids;
    std::vector<PriceLevel> asks;
};

} // namespace domain
