#include "event.h"
#include "event_list.h"
#include "agent.h"
#include <exception>
#include <iostream>
#include <print>
#include "parse.h"
#include "rules.h"
#include <fstream>
#include <string>
#include "os_handle.h"
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
                for (std::size_t i=0;i<window_.size()-2;i++){
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
    if (!log){throw std::invalid_argument("ошибка чтения "+path_);}
    std::string line;
    while (std::getline(log,line)){
        lines_++;
        if (IsBlankOrComment(line)){
            comments_++;
            continue;
        } 

        EventParts out;
        if (!ParseEventParts(line, &out)){
            continue;
        }
        try{
            Event ev=Event(out);
            agent->HandleEvent(ev);
        }catch(...){
            continue;
        }
    }
}
void Agent::Trampoline(const os_event* ev, void* ctx) noexcept{
    try {
        if (ev!=nullptr && ctx!=nullptr){
            Agent* agent=static_cast<Agent*>(ctx);
            EventParts parts;
            parts.ts=std::to_string(ev->ts);
            if (ev->pid==0){
                parts.pid="";
            }else{parts.pid=std::to_string(ev->pid);}
            parts.type=ev->type;
            for (std::size_t i=0;i<ev->field_count;i++){
                parts.fields.push_back({ev->fields[i].key,ev->fields[i].value});
            }
            Event event(parts);
            agent->HandleEvent(event);}
        } catch (...) {}
}
OsSource::OsSource(const std::string& config_path)
: agent_(nullptr),
handle_(config_path)
{}
void OsSource::Run(Agent* agent){
    agent_=agent;
    handle_.Subscribe(Agent::Trampoline,agent_);
    handle_.Start();
    os_status status;
    do{
        status=handle_.Wait(100);
    }while(status==OS_TIMEOUT);
    if (status!=OS_OK){
        throw std::runtime_error(os_status_str(status));
    }
}   
}