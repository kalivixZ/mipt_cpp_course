
#include "agent_rules.h"
#include "make_rule.h"
#include "combinators.h"
#include <vector>

#include "conditions.h"
#include "event.h"
#include "rule.h"
namespace nano_edr {

void RegisterAgentRule1(RuleEngine* engine) {
    auto rule1 = MakeRule<MatchRule>("script_host_from_temp", Severity::kHigh);

    rule1->AddCondition(MakeCondition(AllOfStatic{
        EventTypeIs(std::vector<EventType>{EventType::kProcessStart}),
        AnyOfStatic{FieldEndsWith("image", "wscript.exe"),
            FieldEndsWith("image", "cscript.exe")
        },
        AnyOfStatic{FieldContains("cmdline", "%temp%"),
            FieldContains("cmdline", "%tmp%"),
            FieldContains("cmdline", "\\temp\\"),
            FieldContains("cmdline", "/temp/")
        }}));

    rule1->AddAction("kill_process");
    engine->AddRule(std::move(rule1));
    }
void RegisterAgentRule2(RuleEngine* engine) {
    auto rule2 = MakeRule<MatchRule>("lolbin_download", Severity::kHigh);

    rule2->AddCondition(MakeCondition(AllOfStatic{
        EventTypeIs(std::vector<EventType>{EventType::kProcessStart}),
        AnyOfStatic{FieldEndsWith("image", "certutil.exe"),
            FieldEndsWith("image", "bitsadmin.exe")
        },
        AnyOfStatic{FieldContains("cmdline", "urlcache"),
            FieldContains("cmdline", "transfer"),
            FieldContains("cmdline", "http:"),
            FieldContains("cmdline", "https:")
        }}));

    rule2->AddAction("kill_process");
    engine->AddRule(std::move(rule2));
    }
void RegisterAgentRule3(RuleEngine* engine) {

    auto rule3 = MakeRule<MatchRule>("hidden_powershell", Severity::kMedium);

    rule3->AddCondition(MakeCondition(AllOfStatic{
        EventTypeIs(std::vector<EventType>{EventType::kProcessStart}),
        AnyOfStatic{FieldEndsWith("image", "powershell.exe"),
            FieldEndsWith("image", "pwsh.exe")
        },
        AnyOfStatic{FieldContains("cmdline", "-w hidden"),
            FieldContains("cmdline", "-windowstyle hidden"),
            FieldContains("cmdline", "-enc"),
            FieldContains("cmdline", "-encodedcommand")
        }}));

    rule3->AddAction("kill_process");
    engine->AddRule(std::move(rule3));
    }
void RegisterAgentRule4(RuleEngine* engine) {

    auto rule4 = MakeRule<MatchRule>("autostart_write", Severity::kHigh);

    rule4->AddCondition(MakeCondition(AnyOfStatic{
        AllOfStatic{
            EventTypeIs(std::vector<EventType>{EventType::kFileCreate, EventType::kFileWrite}),
            AnyOfStatic{FieldContains("path", "\\start menu\\programs\\startup\\"),
                FieldContains("path", "/start menu/programs/startup/")
            }
        },
        AllOfStatic{
            EventTypeIs(std::vector<EventType>{EventType::kFileMove}),
            AnyOfStatic{FieldContains("to", "\\start menu\\programs\\startup\\"),
                FieldContains("to", "/start menu/programs/startup/")
            }
        }}));

    rule4->AddAction("quarantine_file");
    engine->AddRule(std::move(rule4));
    }
void RegisterAgentRule5(RuleEngine* engine) {

    auto rule5 = MakeRule<MatchRule>("ransom_extension", Severity::kCritical);

    rule5->AddCondition(MakeCondition(AnyOfStatic{
        AllOfStatic{EventTypeIs(std::vector<EventType>{EventType::kFileCreate, EventType::kFileWrite}),
            FieldEndsWith("path", ".locked")
        },
        AllOfStatic{EventTypeIs(std::vector<EventType>{EventType::kFileMove}),
            FieldEndsWith("to", ".locked")
        }}));

    rule5->AddAction("quarantine_file");
    engine->AddRule(std::move(rule5));
}
void RegisterAgentRule6(RuleEngine* engine) {
    auto rule6 = MakeRule<SequenceRule>("script_host_beacon", Severity::kCritical, 10000);

    rule6->SetFirst(MakeCondition(AllOfStatic{
        EventTypeIs(std::vector<EventType>{EventType::kProcessStart}),
        AnyOfStatic{FieldEndsWith("image", "wscript.exe"),
            FieldEndsWith("image", "cscript.exe")
        },
        AnyOfStatic{FieldContains("cmdline", "%temp%"),
            FieldContains("cmdline", "%tmp%"),
            FieldContains("cmdline", "\\temp\\"),
            FieldContains("cmdline", "/temp/")
        }}));

    rule6->SetSecond(MakeCondition(
        EventTypeIs(std::vector<EventType>{EventType::kNetConnect})));

    rule6->AddAction("kill_process");
    engine->AddRule(std::move(rule6));

    auto mass_write = MakeRule<ThresholdRule<128>>(
    "ransomware_mass_write", Severity::kCritical,
    20,      // сколько событий
    10000,   // за сколько миллисекунд
    "pid");  // порог считается для каждого pid отдельно
    mass_write->SetCondition(MakeCondition(AllOfStatic(
    EventTypeIs({EventType::kFileCreate, EventType::kFileWrite,
                 EventType::kFileMove}),
    AnyOfStatic(FieldContains("path", "\\Documents\\"),
                FieldContains("path", "\\Desktop\\"),
                FieldContains("to", "\\Documents\\"),
                FieldContains("to", "\\Desktop\\")))));

    mass_write->AddAction("kill_process");
    mass_write->AddAction("quarantine_file");
    engine->AddRule(std::move(mass_write));
    
}

void RegisterAgentRules(RuleEngine* engine) {
    RegisterAgentRule1(engine);
    RegisterAgentRule2(engine);
    RegisterAgentRule3(engine);
    RegisterAgentRule4(engine);
    RegisterAgentRule5(engine);
    RegisterAgentRule6(engine);
}
}  // namespace nano_edr