#include "agent.h"

#include <print>

#include "agent_rules.h"
#include "event.h"
#include "event_list.h"
namespace nano_edr {
Agent::Agent(bool quiet)
    :quiet_(quiet),
    engine_() { RegisterAgentRules(&engine_); }
void Agent::HandleEvent(const Event& event) {
    WindowEntry entry{event.ts(), event.type(), event.pid()};
    std::size_t detect_cnt;
    event_cnt_++;
    detect_cnt = engine_.ProcessEvent(event, &fired);
    if (detect_cnt != 0 && !quiet_) {
        if (window_.size() > 0) {
            if (window_.size() == 1) {
                const WindowEntry& last_entry = window_[window_.size() - 1];
                std::print("[CTX] -1: {} {} pid={}\n", last_entry.ts.ms, last_entry.type, last_entry.pid);
            } else if (window_.size() >= 2) {
                const WindowEntry& last1_entry = window_[window_.size() - 2];
                const WindowEntry& last2_entry = window_[window_.size() - 1];
                std::print("[CTX] -2: {} {} pid={}\n", last1_entry.ts.ms, last1_entry.type, last1_entry.pid);
                std::print("[CTX] -1: {} {} pid={}\n", last2_entry.ts.ms, last2_entry.type, last2_entry.pid);
            }
        }
    }
    for (const auto& detected : fired) {
        std::print("[DETECT] {}  {}  ts={} pid={}\n", SeverityName(detected->severity()), detected->id(), event.raw_ts(), event.pid());
    }
    fired.clear();
    bool flag = false;
    for (auto& [type, cnt] : all_types_) {
        if (event.type() == type) {
            cnt++;
            flag = true;
            break;
        }
    }
    if (!flag) {
        all_types_.push_back({event.type(), 1});
    }
    window_.PushBack(entry);
}

void Agent::PrintSummary() const {
    if (quiet_) {
        return;
    }
    std::print("пустых строк или комментариев {}, из них битых {}\n",skipped_line_,broken_line_);
    std::print("всего событий: {}, каждого типа:\n", event_cnt_);
    for (const auto& [type, cnt] : all_types_) {
        std::print("{} : {}\n", type, cnt);
    }
    for (std::size_t i = 0; i < engine_.size(); ++i) {
        std::print("{}  {}\n", engine_.rule(i).id(), engine_.rule(i).hits());
    }
    std::print("[STAT] в окне {} из {} мест\n",window_.size(),window_.capacity());
}

void Agent::OnSkipped() {
    ++skipped_line_;
}
void Agent::OnBroken() {
    ++broken_line_;
}
void Agent::OnTick() {
    ++tick_;
}
}  // namespace nano_edr