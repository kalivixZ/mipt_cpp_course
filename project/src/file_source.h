#include <string>

#include "event_source.h"
namespace nano_edr {
class FileSource : public IEventSource {
 public:
    explicit FileSource(const std::string& path);
    void Run(IEventHandler* handler) override;
    std::size_t lines() const { return lines_; }
 private:
    std::string path_;
    long long lines_ = 0;
};
}  // namespace nano_edr