#include <vector>
#include "rule.h"
namespace nano_edr {
std::vector<MatchRule> BuildAgentRules() {
    std::vector<MatchRule> rules;
    MatchRule rule1("script_host_from_temp", Severity::kHigh);
    rule1.AddCondition(Condition::Equals("type", {"process_start"}));
    rule1.AddCondition(Condition::EndsWith("image", {"wscript.exe", "cscript.exe"}));
    rule1.AddCondition(Condition::Contains("cmdline", {"\\appdata\\local\\temp\\", "/appdata/local/temp/", "\\windows\\temp\\", "/windows/temp/", "%temp%", "%tmp%"}));
    rule1.AddAction("kill_process");
    rules.push_back(rule1);

    MatchRule rule2("lolbin_download", Severity::kHigh);
    rule2.AddCondition(Condition::Equals("type", {"process_start"}));
    rule2.AddCondition(Condition::EndsWith("image", {"certutil.exe", "bitsadmin.exe"}));
    rule2.AddCondition(Condition::Contains("cmdline", {"urlcache", "transfer", "http:", "https:"}));
    rule2.AddAction("kill_process");
    rules.push_back(rule2);

    MatchRule rule3("hidden_powershell", Severity::kMedium);
    rule3.AddCondition(Condition::Equals("type", {"process_start"}));
    rule3.AddCondition(Condition::EndsWith("image", {"powershell.exe", "pwsh.exe"}));
    rule3.AddCondition(Condition::Contains("cmdline", {"-w hidden", "-windowstyle hidden", "-enc", "-encodedcommand"}));
    rule3.AddAction("kill_process");
    rules.push_back(rule3);

    MatchRule rule4("autostart_write", Severity::kHigh);
    rule4.AddCondition(Condition::Equals("type", {"file_create", "file_write"}));
    rule4.AddCondition(Condition::Contains("path", {"\\start menu\\programs\\startup\\", "/start menu/programs/startup/"}));
    rule4.AddAction("quarantine_file");
    rules.push_back(rule4);

    MatchRule rule5("autostart_write", Severity::kHigh);
    rule5.AddCondition(Condition::Equals("type", {"file_move"}));
    rule5.AddCondition(Condition::Contains("to", {"\\start menu\\programs\\startup\\", "/start menu/programs/startup/"}));
    rule5.AddAction("quarantine_file");
    rules.push_back(rule5);

    MatchRule rule6("ransom_extension", Severity::kCritical);
    rule6.AddCondition(Condition::Equals("type", {"file_create", "file_write"}));
    rule6.AddCondition(Condition::EndsWith("path", {".locked"}));
    rule6.AddAction("quarantine_file");
    rules.push_back(rule6);

    MatchRule rule7("ransom_extension", Severity::kCritical);
    rule7.AddCondition(Condition::Equals("type", {"file_move"}));
    rule7.AddCondition(Condition::EndsWith("to", {".locked"}));
    rule7.AddAction("quarantine_file");
    rules.push_back(rule7);

    return rules;
}
}  // namespace nano_edr