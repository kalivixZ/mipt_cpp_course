#include <cstdio>
#include <print>
#include <string>
#include "pid_filter.h"
#include "agent.h"
#include "file_source.h"
#include "os_source.h"
#include "alert_sink.h"
#include "edr_handle.h"
#include "field_traits.h"
struct Options{
    std::string out{}; //имя файла
    bool quiet=false;
    std::string path{}; //log path
    bool dry_run=false;

};
int main(int argc, char** argv) {
    bool file = false;
    std::string pid;
    bool is_pid=false;
    bool is_copies=false;
    Options options{};
    for (int i = 1; i < argc; i++) {
        const std::string arg = argv[i];
        if (arg == "--quiet") {
            options.quiet = true;
        } else if (arg == "--file") {
            file = true;
            if (i + 1 >= argc || !options.path.empty()) {
                std::print(stderr, "не задан путь\n");
                return 2;
            }
            const std::string log_path = argv[++i];
            if (log_path.starts_with("--")) {
                std::print(stderr, "неизвестная опция: {}\n", log_path);
                return 2;
            }
            options.path = log_path;
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
        }else if(arg=="--out"){
            if (i + 1 >= argc) {
                std::print(stderr, "не задано имя файла\n");
                return 2;
            }
            const std::string path = argv[++i];

            if (path.starts_with("--") || path.empty()) {
                std::print(stderr, "не задано имя файла\n");
                return 2;
            }
            options.out = path;
        }else if(arg=="--copies"){
            is_copies=true;
        }else if(arg=="--dry-run"){
            options.dry_run=true;
        }else {
            if (arg.starts_with("--") || !options.path.empty()) {
                std::print(stderr, "неизвестная опция: {}\n", arg);
                return 2;
            }
            options.path = arg;
        }
    }
    if (options.path.empty()) {
        std::print(stderr, "использование: nano-edr <журнал.log>\n");
        return 2;
    }

    //агент и разделение на источники
    try {
        nano_edr::EdrHandle edr(options.path);
        const std::string text = edr.Config("dedup_period_ms", "10000");
        uint64_t dedup_period_ms = 0;
        if (!nano_edr::FieldTraits<uint64_t>::Parse(text, &dedup_period_ms)) {
            std::print(stderr, "[WARN] dedup_period_ms не разбирается: {}\n", text);
            dedup_period_ms = 10000;
        }
        nano_edr::Agent agent(options.quiet, is_copies,dedup_period_ms);
        nano_edr::PidFilter filter_bef_agent(&agent,pid);
        std::unique_ptr<nano_edr::IEventSource> source;

        if (file) {
            source = std::make_unique<nano_edr::FileSource>(options.path);
        } else {
            source = std::make_unique<nano_edr::OsSource>(options.path);
        }
        nano_edr::FanOutSink targets;
        targets.Add(nano_edr::ConsoleSink());
        targets.Add(nano_edr::EdrSink(&edr));
        if (!options.out.empty()) {
            targets.Add(nano_edr::FileSink(options.out));
        }
        
        agent.Subscribe(std::move(targets));
        agent.EnableResponse(source->os(), options.dry_run,
                     edr.Config("quarantine_dir", "C:\\quarantine"));

        if (is_pid) {source->Run(&filter_bef_agent);
        }else{
        source->Run(&agent);
        }
        if (edr.Flush() != EDR_OK) {
        std::print(stderr, "[WARN] детекты не отправлены\n");
        }
        if (!options.quiet || is_copies) {
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
