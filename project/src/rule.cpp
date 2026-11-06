#include "rule.h"

#include "rule_engine.h"
namespace nano_edr {
RuleBase::RuleBase(const std::string& id, Severity severity)
    : id_(id), severity_(severity) {};
void RuleBase::AddAction(const std::string& action) {
    actions_.push_back(action);
}

MatchRule::MatchRule(const std::string& id, Severity severity)
    : RuleBase(id, severity) {};

void MatchRule::AddCondition(std::unique_ptr<ICondition> condition) {
    conditions_.push_back(std::move(condition));
}

bool MatchRule::Check(const Event& event) {
    for (const auto& condition : conditions_) {
        if (!condition->Matches(event)) {
            return false;
        }
    }
    CountHit();
    return true;
}

SequenceRule::SequenceRule(const std::string& id, Severity severity, uint64_t window_ms)
    : RuleBase(id, severity),
      window_ms_(window_ms) {}

void SequenceRule::SetFirst(std::unique_ptr<ICondition> condition) {
    first_ = std::move(condition);
}
void SequenceRule::SetSecond(std::unique_ptr<ICondition> condition) {
    second_ = std::move(condition);
}
bool SequenceRule::Check(const Event& event) {
    if (!first_ || !second_) {
        return false;
    }
    Timestamp event_ts_ms = event.ts();
    for (auto i = pending_.begin(); i != pending_.end();) {
        if (event_ts_ms.ms - i->ts_ms > window_ms_) {
            i = pending_.erase(i);
            continue;
        }
        ++i;
    }
    if (second_->Matches(event)) {
        for (auto i = pending_.begin(); i != pending_.end();) {
            if (!same_pid_ || i->pid == event.pid()) {
                i = pending_.erase(i);
                CountHit();
                return true;
            }
            ++i;
        }
    }
    if (first_->Matches(event)) {
        pending_.push_back({event_ts_ms.ms, event.pid()});
    }
    return false;
}
}  // namespace nano_edr