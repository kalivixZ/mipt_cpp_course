#include "event_list.h"
#include <vector>
#include "rules.h" 
namespace nano_edr{

class Agent {
 public:
    Agent(std::size_t window_size, bool quiet);  // из --window-size и --quiet
    void HandleEvent(const Event& event);
    void PrintSummary() const;  // сводка в конце прогона
    // ...
 private:
    EventList window_;  // последние события, не больше window_size
    bool quiet_;
    inline static long long event_cnt_;
    inline static std::vector<std::pair<std::string,std::size_t>> all_types_;
    const nano_edr::Rule* all_rules_= AgentRules();
    const std::size_t rules_cnt_ = AgentRuleCount();
    


};

class FileSource {
 public:
    explicit FileSource(const std::string& path);
    void Run(Agent* agent);  // строка → EventParts → Event → HandleEvent
private:
    std::string path_;
    inline static long long lines_;
    inline static long long comments_;
};
}