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
            handler->OnSkipped();
            continue;
        }

        EventParts out;
        if (!ParseEventParts(line, &out)) {
            handler->OnBroken();
            continue;
        }
        try {
            Event ev = Event(out);
            handler->HandleEvent(ev);
        } catch (...) {
            handler->OnBroken();
            continue;
        }
    }
}
}  // namespace nano_edr