#include "agent.h"

#include <print>

#include "event.h"
#include "event_list.h"
namespace nano_edr {
Agent::Agent(std::size_t window_size, bool quiet)
    : window_(window_size),
      quiet_(quiet),
      rules_(BuildAgentRules()) {}
void Agent::HandleEvent(const Event& event) {
    std::size_t detect_cnt = 0;
    event_cnt_++;
    for (std::size_t i = 0; i < rules_.size(); ++i) {
        if (rules_[i].Check(event)) {
            std::print("[DETECT] {}  {}  ts={} pid={}\n", SeverityName(rules_[i].severity()), rules_[i].id(), event.raw_ts(), event.pid());
            ++detect_cnt;
        }
    }
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
    if (detect_cnt != 0 && !quiet_) {
        if (window_.size() > 0) {
            if (window_.size() == 1 && last1_event != nullptr) {
                std::print("[CTX] -1: {}\n", ToString(last1_event->event));
            } else if (window_.size() >= 2 && last1_event != nullptr && last2_event != nullptr) {
                std::print("[CTX] -2: {}\n", ToString(last2_event->event));
                std::print("[CTX] -1: {}\n", ToString(last1_event->event));
            }
        }
    }
    window_.PushBack(event);
    last2_event = last1_event;
    last1_event = window_.back();
}

void Agent::PrintSummary() const {
    if (quiet_) {
        return;
    }
    std::print("всего событий: {}, каждого типа:\n", event_cnt_);

    for (const auto& [type, cnt] : all_types_) {
        std::print("{} : {}\n", type, cnt);
    }
    for (std::size_t i = 0; i < rules_.size(); ++i) {
        std::print("{}  {}\n", rules_[i].id(), rules_[i].hits());
    }
}
void Agent::Trampoline(const os_event* ev, void* ctx) noexcept {
    try {
        if (ev != nullptr && ctx != nullptr) {
            Agent* agent = static_cast<Agent*>(ctx);
            EventParts parts;
            parts.ts = std::to_string(ev->ts);
            if (ev->pid == 0) {
                parts.pid = "";
            } else {
                parts.pid = std::to_string(ev->pid);
            }
            parts.type = ev->type ? ev->type : "";
            for (std::size_t i = 0; i < ev->field_count; i++) {
                parts.fields.push_back({ev->fields[i].key, ev->fields[i].value});
            }
            Event event(parts);
            agent->HandleEvent(event);
        }
    } catch (...) {
        std::print(stderr, "ошибка обработки события\n");
    }
}
}  // namespace nano_edr