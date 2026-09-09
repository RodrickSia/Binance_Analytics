#pragma once

#include "domain/depth_diff_event.h"
#include "domain/depth_snapshot_event.h"

namespace handlers {

// Two distinct models (diff vs. snapshot) get two distinctly named methods, not an overload set.
class DepthHandler {
public:
    void handle_depth_diff(const domain::DepthDiffEvent& event);
    void handle_depth_snapshot(const domain::DepthSnapshotEvent& event);
};

} // namespace handlers
