#include "os_source.h"

#include <stdexcept>

#include "agent.h"
#include "os.h"
namespace nano_edr {
OsSource::OsSource(const std::string& config_path)
    : agent_(nullptr),
      handle_(config_path) {}
void OsSource::Run(IEventHandler* handler) {
    agent_ = handler;
    handle_.Subscribe(Agent::Trampoline, agent_);
    handle_.Start();
    os_status status;
    do {
        status = handle_.Wait(100);
    } while (status == OS_TIMEOUT);
    if (status != OS_OK) {
        throw std::runtime_error(std::string("os_wait: ") + os_status_str(status));
    }
}
}  // namespace nano_edr