#include "agent.h"
#include "os_handle.h"
namespace nano_edr {
class OsSource {
 public:
    explicit OsSource(const std::string& config_path);  // создаёт OsHandle
    void Run(Agent* agent);                             // Subscribe, Start, Wait в цикле
 private:
    Agent* agent_;
    OsHandle handle_;
};
}  // namespace nano_edr