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
        nano_edr::FileSource source1(log_path);
        nano_edr::OsSource source2(log_path);
        if (file) {
            source1.Run(&agent);
        } else {
            source2.Run(&agent);
        }
        agent.PrintSummary();
        if (!quiet) {
            std::print("строк: {}, из них комментариев: {}\n", source1.lines(), source1.comments());
            std::print("битых строк: {}, отвергнутых событий: {}\n", source1.bad_lines(), source1.rej_events());
        }
    } catch (const std::exception& e) {
        std::print(stderr, "ошибка: {}", e.what());
        return 1;
    }
    return 0;
}
