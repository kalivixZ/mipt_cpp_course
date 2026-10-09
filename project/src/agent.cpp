#include "event.h"
#include "event_list.h"
#include "agent.h"
#include <iostream>
#include <print>
#include "parse.h"
#include "rules.h"
#include <fstream>
namespace nano_edr{
Agent::Agent(std::size_t window_size, bool quiet)
: window_(window_size),
quiet_(quiet)
{}
void Agent::HandleEvent(const Event& event){
    event_cnt_++;
    const std::size_t detect_cnt = CheckRules(event, all_rules_, rules_cnt_);
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
        if (window_.size()>0){
            if (window_.size()==1){
                const Event& one_node=window_.head()->event;
                std::cout<<one_node<<std::endl;
            }else{
                const EventNode* ctx = window_.head();
                for (int i=0;i<window_.size()-2;i++){
                    ctx=ctx->next;
                }
                std::print("[CTX] -2: {}\n",ToString(ctx->event));
                ctx=ctx->next;
                std::print("[CTX] -1: {}\n",ToString(ctx->event));
            }
        }
    }
    window_.PushBack(event);


}

void Agent::PrintSummary() const {
    if (quiet_) {
        return;
    }
    std::print("всего событий: {}, каждого типа:\n",event_cnt_);

    for (const auto& [type, cnt] : all_types_) {
        std::print("{} : {}\n",type,cnt);
    }
}

FileSource::FileSource(const std::string& path)
: path_(path)
{}
void FileSource::Run(Agent* agent){
    std::ifstream log(path_);
    std::string line;
    while (std::getline(log,line)){
        lines_++;
        if (IsBlankOrComment(&line)){
            comments_++;
            continue;
        }

        EventParts out;
        if (!ParseEventParts(line, &out)){
            continue;
        }
        Event ev=Event(out);
        agent->HandleEvent(ev);
    }
}
}