#include "event.h"

#include <charconv>
#include <stdexcept>
namespace nano_edr {

EventType EventTypeFromString(const std::string& type) {
    if (type == "process_start") {
        return EventType::kProcessStart;
    }
    if (type == "process_end") {
        return EventType::kProcessEnd;
    }
    if (type == "file_create") {
        return EventType::kFileCreate;
    }
    if (type == "file_write") {
        return EventType::kFileWrite;
    }
    if (type == "file_delete") {
        return EventType::kFileDelete;
    }
    if (type == "file_move") {
        return EventType::kFileMove;
    }
    if (type == "net_connect") {
        return EventType::kNetConnect;
    }

    return EventType::kOther;
}

Timestamp ParseTimestamp(const EventParts& parts) {
    const std::string& raw_ts = parts.ts;
    if (raw_ts.empty()) {
        throw std::invalid_argument("empty ts");
    }
    uint64_t temp_fallback;
    const auto [ptr, ec] = std::from_chars(raw_ts.data(), raw_ts.data() + raw_ts.size(), temp_fallback);
    if (ec != std::errc() || (ptr != raw_ts.data() + raw_ts.size())) {
        throw std::invalid_argument("invalid ts: " + raw_ts);
    }
    return Timestamp{temp_fallback};
}

Event::Event(const EventParts& parts)
    : ts_(ParseTimestamp(parts)),
      raw_ts_(parts.ts),
      type_(parts.type),
      event_type_(EventTypeFromString(parts.type)),
      pid_(parts.pid),
      fields_(parts.fields) {
    if (type_.empty()) {
        throw std::invalid_argument("empty type");
    }
}

std::string ToString(const Event& event) {
    std::string to_str;
    to_str += "ts=" + event.raw_ts() + " type=" + event.type() + " pid=" + event.pid();
    for (const Field& field : event.fields()) {
        to_str += " " + field.key + "=" + field.value;
    }
    return to_str;
}

std::ostream& operator<<(std::ostream& out, const Event& event) {
    out << ToString(event);
    return out;
}
}  // namespace nano_edr