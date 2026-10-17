#include "agent.h"
namespace nano_edr {
class FileSource {
 public:
    explicit FileSource(const std::string& path);
    void Run(Agent* agent);  // строка → EventParts → Event → HandleEvent
    std::size_t lines() const { return lines_; }
    std::size_t comments() const { return comments_; }
    std::size_t rej_events() const { return rej_events_; }
    std::size_t bad_lines() const { return bad_lines_; }

 private:
    std::string path_;
    long long lines_ = 0;
    long long comments_ = 0;
    long long rej_events_ = 0;
    long long bad_lines_ = 0;
};
}  // namespace nano_edr