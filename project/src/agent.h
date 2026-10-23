#pragma once
#include <vector>

#include "agent_rules.h"
#include "event_list.h"
#include "event_source.h"
#include "os.h"
#include "rule.h"
#include "rule_engine.h"
namespace nano_edr {
class Agent : public IEventHandler {
 public:
    static void Trampoline(const os_event* ev, void* ctx) noexcept;
    Agent(std::size_t window_size, bool quiet);  // из --window-size и --quiet
    void HandleEvent(const Event& event) override;
    void PrintSummary() const;  // сводка в конце прогона
    void OnSkipped() override;
    void OnTick() override;
    void OnBroken() override;

 private:
    EventList window_;  // последние события, не больше window_size
    bool quiet_ = false;
    std::size_t event_cnt_ = 0;
    std::vector<std::pair<std::string, std::size_t>> all_types_;
    RuleEngine engine_;
    std::vector<const nano_edr::IRule*> fired;
    const EventNode* last1_event = nullptr;
    const EventNode* last2_event = nullptr;
    std::size_t skipped_line_ = 0;
    std::size_t broken_line_ = 0;
    std::size_t tick_ = 0;  //чтобы работал тик
};
}  // namespace nano_edr