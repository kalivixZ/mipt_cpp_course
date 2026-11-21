#include "detection.h"
#include <string>
#include "rules.h"
namespace nano_edr{
std::string ToString(const Detection& detection){
   std::string res = "[DETECT] " + static_cast<std::string>(SeverityName(detection.severity)) + "  " + detection.rule + "  ts=" + std::to_string(detection.ts.ms) + " pid=" + detection.pid;
    if (!detection.chain.empty()) {
        res += " chain=" + detection.chain;
    }
    res += '\n';
    return res;
}
}