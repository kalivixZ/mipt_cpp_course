#include "event_list.h"
#include "agent.h"
namespace nano_edr{

Agent::Agent(std::size_t window_size, bool quiet)
: window_(window_size),
quiet_(quiet)

{}
//void HandleEvent(const Event& event);
//void PrintSummary() const;  // сводка в конце прогона
}