#include "conditions.h"

#include "fields.h"
#include "text.h"
namespace nano_edr {
bool FieldEquals::Matches(const Event& event) const {
    const std::string* pkey = FindField(event, key_);
    if (!pkey) {
        return false;
    }
    const std::string value1_normalize = NormalizeLine(*pkey);
    const std::string value2_normalize = NormalizeLine(value_);
    return LineEqual(value1_normalize, value2_normalize);
}
bool FieldContains::Matches(const Event& event) const {
    const std::string* pkey = FindField(event, key_);
    if (!pkey) {
        return false;
    }
    const std::string value1_normalize = NormalizeLine(*pkey);
    const std::string needle_normalize = NormalizeLine(needle_);
    return LineContains(value1_normalize, needle_normalize);
}
bool FieldEndsWith::Matches(const Event& event) const {
    const std::string* pkey = FindField(event, key_);
    if (!pkey) {
        return false;
    }
    const std::string value1_normalize = NormalizeLine(*pkey);
    const std::string suffix_normalize = NormalizeLine(suffix_);
    return LineEndsWith(value1_normalize, suffix_normalize);
}
bool FieldInList::Matches(const Event& event) const {
    const std::string* pkey = FindField(event, key_);
    if (!pkey) {
        return false;
    }
    std::string key_val = NormalizeLine((*pkey));
    bool flag = false;
    if (values_.empty()) {
        return false;
    }
    for (const std::string& val_in_values_ : values_) {
        if (NormalizeLine(val_in_values_) == key_val) {
            flag = true;
            break;
        }
    }
    return flag;
}

bool EventTypeIs::Matches(const Event& event) const {
    for (auto& type : types_) {
        if (event.event_type() == type) {
            return true;
        }
    }
    return false;
}

void AllOf::Add(std::unique_ptr<ICondition> condition) {
    parts_.push_back(std::move(condition));
}
bool AllOf::Matches(const Event& event) const {
    for (const auto& part : parts_) {
        if (!(part->Matches(event))) {
            return false;
        }
    }
    return true;
}

void AnyOf::Add(std::unique_ptr<ICondition> condition) {
    parts_.push_back(std::move(condition));
}
bool AnyOf::Matches(const Event& event) const {
    bool flag = false;
    for (const auto& part : parts_) {
        if (part->Matches(event)) {
            flag = true;
        }
    }
    return flag;
}
}  // namespace nano_edr