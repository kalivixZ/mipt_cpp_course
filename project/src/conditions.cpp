#include "conditions.h"

#include "fields.h"
#include "text.h"
namespace nano_edr {
bool FieldEquals::operator()(const Event& event) const {
    const std::string* pkey = FindField(event, key_);
    if (!pkey) {
        return false;
    }
    const std::string value1_normalize = NormalizeLine(*pkey);
    const std::string value2_normalize = NormalizeLine(value_);
    return LineEqual(value1_normalize, value2_normalize);
}
bool FieldContains::operator()(const Event& event) const {
    const std::string* pkey = FindField(event, key_);
    if (!pkey) {
        return false;
    }
    const std::string value1_normalize = NormalizeLine(*pkey);
    const std::string needle_normalize = NormalizeLine(needle_);
    return LineContains(value1_normalize, needle_normalize);
}
bool FieldEndsWith::operator()(const Event& event) const {
    const std::string* pkey = FindField(event, key_);
    if (!pkey) {
        return false;
    }
    const std::string value1_normalize = NormalizeLine(*pkey);
    const std::string suffix_normalize = NormalizeLine(suffix_);
    return LineEndsWith(value1_normalize, suffix_normalize);
}
bool FieldInList::operator()(const Event& event) const {
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

Condition Condition::Equals(const std::string& key, const std::vector<std::string>& values) {
    Condition obj(Condition::Match::kEquals, key, values);
    return obj;
}
Condition Condition::Contains(const std::string& key, const std::vector<std::string>& values) {
    Condition obj(Condition::Match::kContains, key, values);
    return obj;
}
Condition Condition::EndsWith(const std::string& key, const std::vector<std::string>& values) {
    Condition obj(Condition::Match::kEndsWith, key, values);
    return obj;
}
// «Любое из значений» — то есть ИЛИ внутри одного поля. ИЛИ между разными
// полями так не выражается, и это настоящее ограничение: правило, которому
// нужно «path в автозапуске ИЛИ to в автозапуске», приходится записывать
// двумя правилами с одним идентификатором. Комбинаторы появятся на 3.1.
bool Condition::operator()(const Event& event) const {
    switch (match_) {
        case Match::kEquals: {
            bool flag = false;
            for (const std::string& val : values_) {
                FieldEquals c(key_, val);
                flag = c(event);
                if (flag) {
                    break;
                }
            }
            return flag;
        }
        case Match::kContains: {
            bool flag = false;
            for (const std::string& val : values_) {
                FieldContains c(key_, val);
                flag = c(event);
                if (flag) {
                    break;
                }
            }
            return flag;
        }
        case Match::kEndsWith: {
            bool flag = false;
            for (const std::string& val : values_) {
                FieldEndsWith c(key_, val);
                flag = c(event);
                if (flag) {
                    break;
                }
            }
            return flag;
        }
        default: return false;
    }
}
}  // namespace nano_edr