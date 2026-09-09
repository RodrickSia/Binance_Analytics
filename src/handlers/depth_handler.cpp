#include "depth_handler.h"

#include "util/logging.h"

namespace handlers {

void DepthHandler::handle_depth_diff(const domain::DepthDiffEvent& event) {
    util::log_info(
        event.symbol + " depthDiff bids=" + std::to_string(event.bids.size()) +
        " asks=" + std::to_string(event.asks.size()));
}

void DepthHandler::handle_depth_snapshot(const domain::DepthSnapshotEvent& event) {
    util::log_info(
        event.symbol + " depthSnapshot bids=" + std::to_string(event.bids.size()) +
        " asks=" + std::to_string(event.asks.size()));
}

} // namespace handlers
