#include <cstdio>
#include <print>
#include <string>
#include "pid_filter.h"
#include "agent.h"
#include "file_source.h"
#include "os_source.h"

int main(int argc, char** argv) {
    bool quiet = false;
    bool file = false;
    std::string pid;
    bool is_pid=false;
    std::string log_path;
    for (int i = 1; i < argc; i++) {
        const std::string arg = argv[i];
        if (arg == "--quiet") {
            quiet = true;
        } else if (arg == "--file") {
            file = true;
            if (i + 1 >= argc || !log_path.empty()) {
                std::print(stderr, "не задан путь\n");
                return 2;
            }
            const std::string path = argv[++i];
            if (path.starts_with("--")) {
                std::print(stderr, "неизвестная опция: {}\n", path);
                return 2;
            }
            log_path = path;
        } else if (arg=="--pid"){
            if (i + 1 >= argc) {
                std::print(stderr, "не задан pid\n");
                return 2;
            }
            pid=argv[++i];
            if (pid.empty()){
                std::print(stderr,"нет аргумента после опции --pid\n");
                return 2;
            }
        is_pid=true;
        }else {
            if (arg.starts_with("--") || !log_path.empty()) {
                std::print(stderr, "неизвестная опция: {}\n", arg);
                return 2;
            }
            log_path = arg;
        }
    }
    if (log_path.empty()) {
        std::print(stderr, "использование: nano-edr <журнал.log>\n");
        return 2;
    }
    //агент и разделение на источники
    try {
        nano_edr::Agent agent(quiet);

        nano_edr::PidFilter filter_bef_agent(&agent,pid);
        std::unique_ptr<nano_edr::IEventSource> source;

        if (file) {
            source = std::make_unique<nano_edr::FileSource>(log_path);
        } else {
            source = std::make_unique<nano_edr::OsSource>(log_path);
        }
        if (is_pid) {source->Run(&filter_bef_agent);
        }else{
        source->Run(&agent);
        }
        if (!quiet) {
            agent.PrintSummary();
            auto result=filter_bef_agent.filtered();
            std::print("отфильтровано событий: {}\n",result);
        }
    } catch (const std::exception& e) {
        std::print(stderr, "ошибка: {}\n", e.what());
        return 1;
    }
    return 0;
}
