
#include "agent_rules.h"

#include <vector>

#include "conditions.h"
#include "event.h"
#include "rule.h"
namespace nano_edr {

void RegisterAgentRules(RuleEngine* engine) {
    auto rule1 = std::make_unique<MatchRule>("script_host_from_temp", Severity::kHigh);
    rule1->AddCondition(std::make_unique<EventTypeIs>(std::vector<EventType>{EventType::kProcessStart}));
    auto image_condition = std::make_unique<AnyOf>();
    image_condition->Add(std::make_unique<FieldEndsWith>("image", "wscript.exe"));
    image_condition->Add(std::make_unique<FieldEndsWith>("image", "cscript.exe"));
    rule1->AddCondition(std::move(image_condition));
    auto temp_command = std::make_unique<AnyOf>();
    temp_command->Add(std::make_unique<FieldContains>("cmdline", "%temp%"));
    temp_command->Add(std::make_unique<FieldContains>("cmdline", "%tmp%"));
    temp_command->Add(std::make_unique<FieldContains>("cmdline", "\\temp\\"));
    temp_command->Add(std::make_unique<FieldContains>("cmdline", "/temp/"));
    rule1->AddCondition(std::move(temp_command));
    rule1->AddAction("kill_process");
    engine->AddRule(std::move(rule1));

    auto rule2 = std::make_unique<MatchRule>("lolbin_download", Severity::kHigh);
    rule2->AddCondition(std::make_unique<EventTypeIs>(std::vector<EventType>{EventType::kProcessStart}));

    auto lolbin_image = std::make_unique<AnyOf>();
    lolbin_image->Add(std::make_unique<FieldEndsWith>("image", "certutil.exe"));
    lolbin_image->Add(std::make_unique<FieldEndsWith>("image", "bitsadmin.exe"));
    rule2->AddCondition(std::move(lolbin_image));

    auto download_command = std::make_unique<AnyOf>();
    download_command->Add(std::make_unique<FieldContains>("cmdline", "urlcache"));
    download_command->Add(std::make_unique<FieldContains>("cmdline", "transfer"));
    download_command->Add(std::make_unique<FieldContains>("cmdline", "http:"));
    download_command->Add(std::make_unique<FieldContains>("cmdline", "https:"));
    rule2->AddCondition(std::move(download_command));
    rule2->AddAction("kill_process");
    engine->AddRule(std::move(rule2));

    auto rule3 = std::make_unique<MatchRule>("hidden_powershell", Severity::kMedium);
    rule3->AddCondition(std::make_unique<EventTypeIs>(std::vector<EventType>{EventType::kProcessStart}));

    auto powershell_image = std::make_unique<AnyOf>();
    powershell_image->Add(std::make_unique<FieldEndsWith>("image", "powershell.exe"));
    powershell_image->Add(std::make_unique<FieldEndsWith>("image", "pwsh.exe"));
    rule3->AddCondition(std::move(powershell_image));

    auto hidden_command = std::make_unique<AnyOf>();
    hidden_command->Add(std::make_unique<FieldContains>("cmdline", "-w hidden"));
    hidden_command->Add(std::make_unique<FieldContains>("cmdline", "-windowstyle hidden"));
    hidden_command->Add(std::make_unique<FieldContains>("cmdline", "-enc"));
    hidden_command->Add(std::make_unique<FieldContains>("cmdline", "-encodedcommand"));
    rule3->AddCondition(std::move(hidden_command));
    rule3->AddAction("kill_process");
    engine->AddRule(std::move(rule3));

    auto rule4 = std::make_unique<MatchRule>("autostart_write", Severity::kHigh);

    auto startup_create_or_write = std::make_unique<AllOf>();
    startup_create_or_write->Add(std::make_unique<EventTypeIs>(std::vector<EventType>{EventType::kFileCreate, EventType::kFileWrite}));

    auto startup_path = std::make_unique<AnyOf>();
    startup_path->Add(std::make_unique<FieldContains>("path", "\\start menu\\programs\\startup\\"));
    startup_path->Add(std::make_unique<FieldContains>("path", "/start menu/programs/startup/"));
    startup_create_or_write->Add(std::move(startup_path));

    auto startup_move = std::make_unique<AllOf>();
    startup_move->Add(std::make_unique<EventTypeIs>(std::vector<EventType>{EventType::kFileMove}));

    auto startup_destination = std::make_unique<AnyOf>();
    startup_destination->Add(std::make_unique<FieldContains>("to", "\\start menu\\programs\\startup\\"));
    startup_destination->Add(std::make_unique<FieldContains>("to", "/start menu/programs/startup/"));
    startup_move->Add(std::move(startup_destination));

    auto startup_condition = std::make_unique<AnyOf>();
    startup_condition->Add(std::move(startup_create_or_write));
    startup_condition->Add(std::move(startup_move));

    rule4->AddCondition(std::move(startup_condition));
    rule4->AddAction("quarantine_file");
    engine->AddRule(std::move(rule4));

    auto rule5 = std::make_unique<MatchRule>("ransom_extension", Severity::kCritical);

    auto locked_create_or_write = std::make_unique<AllOf>();
    locked_create_or_write->Add(std::make_unique<EventTypeIs>(std::vector<EventType>{EventType::kFileCreate, EventType::kFileWrite}));
    locked_create_or_write->Add(std::make_unique<FieldEndsWith>("path", ".locked"));

    auto locked_move = std::make_unique<AllOf>();
    locked_move->Add(std::make_unique<EventTypeIs>(std::vector<EventType>{EventType::kFileMove}));
    locked_move->Add(std::make_unique<FieldEndsWith>("to", ".locked"));

    auto locked_condition = std::make_unique<AnyOf>();
    locked_condition->Add(std::move(locked_create_or_write));
    locked_condition->Add(std::move(locked_move));

    rule5->AddCondition(std::move(locked_condition));
    rule5->AddAction("quarantine_file");
    engine->AddRule(std::move(rule5));

    auto rule6 = std::make_unique<SequenceRule>("script_host_beacon", Severity::kCritical, 10000);
    auto all_conditions = std::make_unique<AllOf>();
    all_conditions->Add(std::make_unique<EventTypeIs>(std::vector<EventType>{EventType::kProcessStart}));

    auto host_image = std::make_unique<AnyOf>();
    host_image->Add(std::make_unique<FieldEndsWith>("image", "wscript.exe"));
    host_image->Add(std::make_unique<FieldEndsWith>("image", "cscript.exe"));
    all_conditions->Add(std::move(host_image));

    auto temp_catalog = std::make_unique<AnyOf>(); //без временного каталога срабатывание на чистом сценарии
    temp_catalog->Add(
        std::make_unique<FieldContains>("cmdline", "%temp%"));
    temp_catalog->Add(
        std::make_unique<FieldContains>("cmdline", "%tmp%"));
    temp_catalog->Add(
        std::make_unique<FieldContains>("cmdline", "\\temp\\"));
    temp_catalog->Add(
        std::make_unique<FieldContains>("cmdline", "/temp/"));
    all_conditions->Add(std::move(temp_catalog));

    rule6->SetFirst(std::move(all_conditions));

    rule6->SetSecond(std::make_unique<EventTypeIs>(std::vector<EventType>{EventType::kNetConnect}));
    rule6->AddAction("kill_process");
    engine->AddRule(std::move(rule6));
}
}  // namespace nano_edr