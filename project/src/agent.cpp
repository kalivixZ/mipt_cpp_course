#include "agent.h"

#include <print>
#include <algorithm>
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
        for (std::size_t i = window_.size()- std::min(window_.size(),std::size_t(2));i<window_.size();++i){
            const WindowEntry& entry = window_[i];
            std::print("[CTX] -{}: {} {} pid={}\n",window_.size()-i, entry.ts.ms, entry.type, entry.pid);
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
    std::print("пустых строк или комментариев {}, битых строк{}\n",skipped_line_,broken_line_);
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