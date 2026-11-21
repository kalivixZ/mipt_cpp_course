#include "rule_engine.h"

#include "alert_sink.h"
#include "copy_stats.h"
#include "detection.h"
namespace nano_edr {

void RuleEngine::AddRule(std::unique_ptr<IRule> rule) {
    rules_.push_back(std::move(rule));
}
// Подписать получателя детектов. Их может быть сколько угодно, включая
// ноль: движок без получателей просто молчит, и тесты занятия 2.3,
// не знающие про получателей вообще, продолжают работать.
void RuleEngine::Subscribe(AlertSink sink) {
    sinks_.push_back(sink);
}

std::size_t RuleEngine::ProcessEvent(const Event& event, std::vector<const IRule*>* fired) {
    std::size_t detect_cnt = 0;
    for (const auto& rule : rules_) {
        if (rule->Check(event)) {
            ++detect_cnt;
            Detection detection = MakeDetection(*rule, event);
            if (fired) {
                (*fired).push_back(rule.get());
            }
            for (const auto& sink : sinks_) {
                sink(detection);  //оператор() у получателей которые подходят под сигнатуру
            }
        }
    }
    return detect_cnt;
}
Detection RuleEngine::MakeDetection(const IRule& rule, const Event& event) const {
    Detection detection{};
    detection.rule = rule.id();
    detection.severity = rule.severity();
    detection.ts = event.ts();
    detection.pid = event.pid();
    detection.evidence.push_back(std::make_unique<Event>(event));
    ++copy_stats().evidence_copied;

    if (model_) {
        detection.chain = model_->DescribeChain(event.pid());
    } else {
        detection.chain = "";
    }
    detection.actions = rule.actions();
    return detection;
}
}  // namespace nano_edr