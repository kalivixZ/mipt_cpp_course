#include "file_source.h"

#include <fstream>
#include <stdexcept>
#include <string>

#include "event.h"
#include "event_source.h"
#include "parse.h"
namespace nano_edr {
FileSource::FileSource(const std::string& path)
    : path_(path) {}
void FileSource::Run(IEventHandler* handler) {
    std::ifstream log(path_);
    if (!log) {
        throw std::runtime_error("не удалось открыть журнал: " + path_);
    }
    std::string line;
    while (std::getline(log, line)) {
        lines_++;
        if (IsBlankOrComment(line)) {
            comments_++;
            continue;
        }

        EventParts out;
        if (!ParseEventParts(line, &out)) {
            bad_lines_++;
            continue;
        }
        try {
            Event ev = Event(out);
            handler->HandleEvent(ev);
        } catch (...) {
            rej_events_++;
            continue;
        }
    }
}
}  // namespace nano_edr