#pragma once
#include <vector>
#include "ring_buffer.h"
#include "agent_rules.h"
#include "event_list.h"
#include "event_source.h"
#include "rule.h"
#include "rule_engine.h"
namespace nano_edr {
static std::size_t first_print=0;
struct WindowEntry {
    Timestamp ts;
    std::string type;
    std::string pid;
};
class Agent : public IEventHandler {
 public:
   static constexpr std::size_t kWindowCapacity = 256;
    explicit Agent(bool quiet);
    void HandleEvent(const Event& event) override;
    void PrintSummary() const;  // сводка в конце прогона
    void OnSkipped() override;
    void OnTick() override;
    void OnBroken() override;

 private:
    RingBuffer<WindowEntry, kWindowCapacity> window_;
    bool quiet_ = false;
    RuleEngine engine_;
    std::size_t event_cnt_ = 0;
    std::vector<std::pair<std::string, std::size_t>> all_types_;
    std::vector<const nano_edr::IRule*> fired;
    std::size_t skipped_line_ = 0;
    std::size_t broken_line_ = 0;
    std::size_t tick_ = 0;  //чтобы работал тик
};
}  // namespace nano_edr