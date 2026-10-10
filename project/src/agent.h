#include "event_list.h"
#include <vector>
#include "os_handle.h"
#include "rules.h" 
#include "agent_rules.h"
#include "os.h"
namespace nano_edr{

class Agent {
 public:
    static void Trampoline(const os_event* ev, void* ctx) noexcept;
    Agent(std::size_t window_size, bool quiet);  // из --window-size и --quiet
    void HandleEvent(const Event& event);
    void PrintSummary() const;  // сводка в конце прогона
    // ...
 private:
    EventList window_;  // последние события, не больше window_size
    bool quiet_;
    long long event_cnt_=0;
    std::vector<std::pair<std::string,std::size_t>> all_types_;
    const nano_edr::Rule* all_rules_= AgentRules();
    const std::size_t rules_cnt_ = AgentRuleCount();
    


};

class FileSource {
 public:
    explicit FileSource(const std::string& path);
    void Run(Agent* agent);  // строка → EventParts → Event → HandleEvent
private:
    std::string path_;
    long long lines_=0;
    long long comments_=0;
};
class OsSource {
 public:
    explicit OsSource(const std::string& config_path);  // создаёт OsHandle
    void Run(Agent* agent);  // Subscribe, Start, Wait в цикле
private:
    Agent* agent_;
    OsHandle handle_;
};
}