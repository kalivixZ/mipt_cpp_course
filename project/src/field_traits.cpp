#include "field_traits.h"
#include <charconv>
#include "text.h"
namespace nano_edr {

bool FieldTraits<std::string>::Parse(const std::string&, std::string*) {
    return true;
}
bool FieldTraits<uint64_t>::Parse(const std::string& text, uint64_t* out) {
    if (text.empty()){return false;}
    uint64_t temp_out;
    const auto [ptr, ec] = std::from_chars(text.data(), text.data() + text.size(), temp_out);
    if (ec != std::errc() || (ptr != text.data() + text.size())) {
         return false;
    }
    *out = temp_out;
    return true;
    
}
bool FieldTraits<bool>::Parse(const std::string&text, bool* out) {
    const std::string normalized_text = NormalizeLine(text);
    if (normalized_text == "1" || normalized_text == "true" || normalized_text == "yes") {
        *out = true;
        return true;
    }
    if (normalized_text == "0" || normalized_text == "false" || normalized_text == "no") {
        *out = false;
        return true;
    }else{return false;}
}
}   // namespace nano_edr