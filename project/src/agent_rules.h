#include "rule_engine.h"
#include "entity_model.h"
namespace nano_edr {
RuleEngine BuildAgentRules(const EntityModel* model);
void RegisterAgentRule1(RuleEngine* engine);
void RegisterAgentRule2(RuleEngine* engine);
void RegisterAgentRule3(RuleEngine* engine);
void RegisterAgentRule4(RuleEngine* engine);
void RegisterAgentRule5(RuleEngine* engine);
void RegisterAgentRule6(RuleEngine* engine);
void RegisterAgentRule8(RuleEngine* engine,const EntityModel* model);
void RegisterAgentRule9(RuleEngine* engine,const EntityModel* model);
}
