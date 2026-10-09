#include "event.h"
#include "event_list.h"
#include "agent.h"
#include <iostream>
#include <print>
#include "rules.h"
namespace nano_edr{

Agent::Agent(std::size_t window_size, bool quiet)
: window_(window_size),
quiet_(quiet)
{}
void Agent::HandleEvent(const Event& event){
    const std::size_t detect_cnt = CheckRules(event, all_rules, rules_cnt);
    bool flag=false;
    for (auto&[type,cnt]:all_types_){
        if (event.type()==type){
            cnt++;
            flag=true;
        }
    }
    if (!flag){
        all_types_.push_back({event.type(),1});
    }
    if (detect_cnt!=0 && !quiet_){
        if (window_.size()==1){
            const Event& one_node=window_.head->event;
            std::cout<<one_node<<std::endl;
        }else{
            const EventNode* ctx = window_.head;
            for (int i=0;i<window_.size()-2;i++){
                ctx=ctx->next;
            }
            std::print("[CTX] -2:"+ToString(*ctx));
            ctx=ctx->next;
            std::print("[CTX] -1:"+ToString(*ctx));
        }
    }
    window_.PushBack(event);


}
//void PrintSummary() const;  // сводка в конце прогона
}