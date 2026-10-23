#include "os_source.h"
#include <print>
#include <stdexcept>
#include "os.h"
namespace nano_edr {
void OsSource::Trampoline(const os_event* ev, void* ctx) noexcept {
    IEventHandler* agent = static_cast<IEventHandler*>(ctx);
    try {
        if (ev != nullptr && ctx != nullptr) {
            EventParts parts;
            parts.ts = std::to_string(ev->ts);
            if (ev->pid == 0) {
                parts.pid = "";
            } else {
                parts.pid = std::to_string(ev->pid);
            }
            parts.type = ev->type ? ev->type : "";
            for (std::size_t i = 0; i < ev->field_count; i++) {
                parts.fields.push_back({ev->fields[i].key, ev->fields[i].value});
            }
            Event event(parts);
            agent->HandleEvent(event);
        }
    } catch (...) {
        agent->OnBroken();
        std::print(stderr, "ошибка обработки события\n");
    }
}
OsSource::OsSource(const std::string& config_path)
    : agent_(nullptr),
      handle_(config_path) {}
void OsSource::Run(IEventHandler* handler) {
    agent_ = handler;
    handle_.Subscribe(&OsSource::Trampoline, agent_);
    handle_.Start();
    os_status status = handle_.Wait(100);
    while (status == OS_TIMEOUT) {
    handler->OnTick();
    status = handle_.Wait(100);
    }
    if (status != OS_OK) {
        throw std::runtime_error(std::string("os_wait: ") + os_status_str(status));
    }
}
}  // namespace nano_edr