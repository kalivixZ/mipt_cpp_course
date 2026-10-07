#include "event.h"
#include "fields.h"
#include "rules.h"

namespace nano_edr{

bool ScriptHostFromTemp(const Event& event){
    if (!IsProcessStart(event)){return false;}
    
    const std::string& image=GetRequiredField(event,"image");
    std::string normalize_image=NormalizePath(image);
    if (!(normalize_image.ends_with("wscript.exe") || normalize_image.ends_with("cscript.exe"))){return false;}
    if (!(CommandLineContains(event,"\\appdata\\local\\temp\\") || CommandLineContains(event,"\\windows\\temp\\"))){return false;}
    return true; 
}
bool LolbinDownload(const Event& event){
    if (!IsProcessStart(event)){return false;}

    const std::string& image=GetRequiredField(event,"image");
    std::string normalize_image=NormalizePath(image);
    if (!(normalize_image.ends_with("certutil.exe") || normalize_image.ends_with("bitsadmin.exe"))){return false;}
    if (!(CommandLineContains(event,"urlcache")
     || CommandLineContains(event,"transfer")
      || CommandLineContains(event,"http:")
       || CommandLineContains(event,"https:"))){return false;}
    return true; 
}
bool HiddenPowershell(const Event& event){
    if (!IsProcessStart(event)){return false;}
    
    const std::string& image=GetRequiredField(event,"image");
    std::string normalize_image=NormalizePath(image);
    if (!(normalize_image.ends_with("powershell.exe") || normalize_image.ends_with("pwsh.exe"))){return false;}
    if (!(CommandLineContains(event,"-w hidden")
     || CommandLineContains(event,"-windowstyle hidden")
      || CommandLineContains(event,"-enc")
       || CommandLineContains(event,"-encodedcommand"))){return false;}
    return true; 
}
bool AutostartWrite(const Event& event){
    if (!IsFileWrite(event)){return false;}
    
    const std::string* key;
    if (event.type=="file_move"){
        key=FindField(event,"to");
    }else{
        key=FindField(event,"path");
    }
    if (!key){return false;}
    const std::string normalize_key=NormalizePath(*key);
    if (normalize_key.find("\\start menu\\programs\\startup\\")==std::string::npos){return false;}
    return true;
}
bool RansomExtension(const Event& event){
    if (!IsFileWrite(event)){return false;}
    
    const std::string* key;
    if (event.type=="file_move"){
        key=FindField(event,"to");
    }else{
        key=FindField(event,"path");
    }
    if (!key){return false;}
    const std::string normalize_key=NormalizePath(*key);
    if (!normalize_key.ends_with(".locked")){return false;}
    return true;
}
constexpr Rule kRules[] = {
    {"script_host_from_temp", ScriptHostFromTemp, Severity::kHigh},
    {"lolbin_download", LolbinDownload, Severity::kHigh},
    {"hidden_powershell", HiddenPowershell, Severity::kMedium},
    {"autostart_write", AutostartWrite, Severity::kHigh},
    {"ransom_extension", RansomExtension, Severity::kCritical}
};
const Rule* AgentRules(){
    return kRules;
};
size_t AgentRuleCount(){
    return sizeof(kRules)/sizeof(kRules[0]);
}
}