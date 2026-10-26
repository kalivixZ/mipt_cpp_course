#include "pid_filter.h"
#include <string>
namespace nano_edr{
    PidFilter::PidFilter(IEventHandler* next, const std::string& pid)
        : agent(next),
        pid(pid)
        {}

    void PidFilter::HandleEvent(const Event& event){
        if (event.pid().compare(pid)==0){
            agent->HandleEvent(event);
        }else{
            ++filtered_ev;
        }

    }
    void PidFilter::OnSkipped() {
    agent->OnSkipped();
    }
    void PidFilter::OnBroken() {
    agent->OnBroken();
    }
    void PidFilter::OnTick() {
    agent->OnTick();
    }
    std::size_t PidFilter::filtered() {
    return filtered_ev;
    }
}