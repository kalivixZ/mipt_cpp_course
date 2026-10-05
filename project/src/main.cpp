#include <charconv>
#include <cstddef>
#include <cstdio>
#include <fstream>
#include <print>
#include <string>
#include <vector>
#include "event_list.h"
#include "parse.h"
#include "rules.h"
#include "agent_rules.h"

int main(int argc, char** argv) {
    try{
    nano_edr::EventList list{.capacity = 64};
    std::string log_path;
    bool quiet=false;
    //перебор арг
    for (int i = 1; i < argc; ++i) {
        const std::string arg = argv[i];

        if (arg == "--window-size") {
            if (i + 1 >= argc) {return 2;}

            const std::string value_text = argv[++i];
            std::size_t value = 0;
            const auto [ptr, ec] = std::from_chars(value_text.data(),value_text.data() + value_text.size(),value);
            if (ec != std::errc{} ||ptr != value_text.data() + value_text.size()) {return 2;}
            list.capacity = value;
        } else if (arg == "--quiet") {
            quiet = true;
        } else {
            if (arg.starts_with("--") || !log_path.empty()) {return 2;}
            log_path = arg;
        }
    }
    if (log_path.empty()) {
        std::print(stderr, "использование: nano-edr <журнал.log>\n");
        return 2;
    }
    std::ifstream log(log_path);
    if (!log) {
        std::print(stderr, "не удалось открыть журнал: {}\n", log_path);
        return 2;
    }

    std::shared_ptr<EventNode> node = std::make_shared<EventNode>();
    auto node1 = node;
    auto node2 = node;

    std::unique_ptr<EventNode> unique_node = std::make_unique<EventNode>();
    auto node3 = unique_node;

    std::unique_ptr<EventNode> unique_node2 = std::make_unique<EventNode>();

    long long lines = 0;
    long long comments = 0;
    std::string line;
    std::vector<std::pair<std::string, std::size_t>>all_types;
    const nano_edr::Rule* all_rules=nano_edr::AgentRules();
    std::size_t rules_count = nano_edr::AgentRuleCount();

    while (std::getline(log, line)) {
        ++lines;
        /*std::size_t first = line.find_first_not_of(" \t");
        if (first != std::string::npos &&
            (line[first] == '#' || line[first] == ';')) {
            ++comments;
            continue;
        }*/
        if (nano_edr::IsBlankOrComment(&line)) {
            ++comments;
            continue;
        }
        
        //парсинг+вывод
        nano_edr::Event event;
        if (!nano_edr::ParseEventLine(&line, &event)) {continue;}
        const std::size_t detect_count = nano_edr::CheckRules(event,all_rules,rules_count);

        bool flag=false;
        for (auto& [type,count]:all_types){
            if (event.type==type){
                count+=1;
                flag=true;
            }
        }
        if (!flag){
            all_types.push_back({event.type,1});
        }
        //контекст
        if (detect_count!=0 && !quiet) { //два последних узла на вывод
            if (list.size==1){
                const nano_edr::Event& one_node=list.head->event;
                std::print("[CTX] -1: ts={} type={} pid={}\n",one_node.ts,one_node.type,one_node.pid);
            } else if (list.size >= 2) {
                const nano_edr::EventNode* context = list.head;
                if (list.size > 2) {
                    for (std::size_t i = 0; i < list.size - 2; ++i) { //по списку до -2ого узла
                        context = context->next;
                    }
                }

                std::print("[CTX] -2: ts={} type={} pid={}\n",context->event.ts,context->event.type,context->event.pid);
                context = context->next;
                std::print("[CTX] -1: ts={} type={} pid={}\n",context->event.ts,context->event.type,context->event.pid);
            }
        }

        nano_edr::ListPushBack(&list, &event);
    }

    if (!quiet) {
        for (const auto& [type_name, count] : all_types) {
            std::print("тип {}, количество {}\n", type_name, count);
        }
        std::print("строк {}, из них комментариев {}\n", lines, comments);
    }

    return 0;
}catch (const std::exception& error){
    std::print("{}\n",error.what());
    return 1;}
}
