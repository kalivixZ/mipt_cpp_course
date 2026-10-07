#include "parse.h"

#include <cstddef>
#include <string>

namespace nano_edr {
namespace {

bool IsSpace(char ch) {
    return ch == ' ' || ch == '\t';
}

bool ParsePair(const std::string* line, std::size_t* pos, Field* out) {
    const std::size_t size = line->size();
    const std::size_t key_start = *pos;
    std::size_t equal_pos = key_start;

    while (equal_pos < size && (*line)[equal_pos] != '=') {
        if (IsSpace((*line)[equal_pos])) {
            return false;
        }
        ++equal_pos;
    }

    if (equal_pos == key_start || equal_pos == size) {
        return false;
    }

    const std::string key = line->substr(key_start, equal_pos - key_start);
    *pos = equal_pos + 1;

    std::string value;
    if (*pos < size && (*line)[*pos] == '"') {
        ++(*pos);
        const std::size_t value_start = *pos;
        while (*pos < size && (*line)[*pos] != '"') {
            ++(*pos);
        }

        if (*pos == size) {
            return false;
        }

        value = line->substr(value_start, *pos - value_start);
        ++(*pos);

        if (*pos < size && !IsSpace((*line)[*pos])) {
            return false;
        }
    } else {
        const std::size_t value_start = *pos;
        while (*pos < size && !IsSpace((*line)[*pos])) {
            ++(*pos);
        }
        value = line->substr(value_start, *pos - value_start);
    }

    out->key = key;
    out->value = value;
    return true;
}

}  // namespace

bool IsBlankOrComment(const std::string* line) {
    if (line == nullptr) {
        return false;
    }

    std::size_t pos = 0;
    while (pos < line->size() && IsSpace((*line)[pos])) {
        ++pos;
    }

    return pos == line->size() || (*line)[pos] == '#' || (*line)[pos] == ';';
}

bool ParseEventLine(const std::string* line, Event* out) {
    if (line == nullptr || out == nullptr || IsBlankOrComment(line)) {
        return false;
    }

    *out = Event{};

    bool has_ts = false;
    bool has_type = false;
    bool has_pid = false;
    std::size_t pos = 0;

    while (pos < line->size()) {
        while (pos < line->size() && IsSpace((*line)[pos])) {
            ++pos;
        }
        if (pos == line->size()) {
            break;
        }

        Field field;
        if (!ParsePair(line, &pos, &field)) {
            return false;
        }

        if (field.key == "ts" && !has_ts) {
            out->ts = field.value;
            has_ts = true;
        } else if (field.key == "type" && !has_type) {
            out->type = field.value;
            has_type = true;
        } else if (field.key == "pid" && !has_pid) {
            out->pid = field.value;
            has_pid = true;
        } else {
            out->fields.push_back(field);
        }
    }

    return has_ts && has_type;
}

}  // namespace nano_edr
