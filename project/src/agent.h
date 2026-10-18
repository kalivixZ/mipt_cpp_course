#pragma once
#include <vector>

#include "agent_rules.h"
#include "event_list.h"
#include "os.h"
#include "rule.h"
namespace nano_edr {
class Agent {
 public:
    static void Trampoline(const os_event* ev, void* ctx) noexcept;
    Agent(std::size_t window_size, bool quiet);  // из --window-size и --quiet
    void HandleEvent(const Event& event);
    void PrintSummary() const;  // сводка в конце прогона
 private:
    EventList window_;  // последние события, не больше window_size
    bool quiet_ = false;
    std::size_t event_cnt_ = 0;
    std::vector<std::pair<std::string, std::size_t>> all_types_;
    std::vector<MatchRule> rules_;
    const EventNode* last1_event = nullptr;
    const EventNode* last2_event = nullptr;
};
}  // namespace nano_edr