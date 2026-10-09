#include "event_list.h"
#include <vector>
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
    long long lines_;
    long long comments;
    std::vector<std::pair<std::string,std::size_t>> all_types_;



};

class FileSource {
 public:
    explicit FileSource(const std::string& path);
    void Run(Agent* agent);  // строка → EventParts → Event → HandleEvent
    // ...
};
}