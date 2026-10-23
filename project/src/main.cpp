#include <charconv>
#include <cstdio>
#include <print>
#include <string>

#include "agent.h"
#include "file_source.h"
#include "os_source.h"

int main(int argc, char** argv) {
    std::size_t value = 0;
    bool quiet = false;
    bool file = false;
    std::string log_path;
    for (int i = 1; i < argc; i++) {
        const std::string arg = argv[i];
        if (arg == "--window-size") {
            if (i + 1 >= argc) {
                std::print(stderr, "нет значения после {}\n", arg);
                return 2;
            }
            const std::string value_str = argv[++i];
            auto [ptr, ec] = std::from_chars(value_str.data(), value_str.data() + value_str.size(), value);
            if (ec != std::errc() || ptr != value_str.data() + value_str.size()) {
                std::print(stderr, "неправильный аргумент: {}\n", value_str);
                return 2;
            }
        } else if (arg == "--quiet") {
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
        } else {
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
        nano_edr::Agent agent(value, quiet);
        std::unique_ptr<nano_edr::IEventSource> source;
        if (file) {
            source = std::make_unique<nano_edr::FileSource>(log_path);
        } else {
            source = std::make_unique<nano_edr::OsSource>(log_path);
        }
        source->Run(&agent);
        agent.PrintSummary();
        if (!quiet) {
            auto* file_source = dynamic_cast<nano_edr::FileSource*>(source.get());
            if (file_source) {
                std::print("строк: {}, из них комментариев: {}\n", file_source->lines(), file_source->comments());
                std::print("битых строк: {}, отвергнутых событий: {}\n", file_source->bad_lines(), file_source->rej_events());
            }
        }
    } catch (const std::exception& e) {
        std::print(stderr, "ошибка: {}", e.what());
        return 1;
    }
    return 0;
}
