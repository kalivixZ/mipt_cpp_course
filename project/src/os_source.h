#include "event_source.h"
#include "os_handle.h"

namespace nano_edr {
class OsSource : public IEventSource {
 public:
    explicit OsSource(const std::string& config_path);  // создаёт OsHandle
    void Run(IEventHandler* handler) override;          // Subscribe, Start, Wait в цикле
 private:
    IEventHandler* agent_;
    OsHandle handle_;
};
}  // namespace nano_edr