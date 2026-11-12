#include "text.h"

#include <cctype>
namespace nano_edr {
std::string NormalizeLine(const std::string& initial_line) {
    std::string line_tolower;
    for (std::size_t i = 0; i < initial_line.size(); ++i) {
        char curr_char = initial_line[i];
        line_tolower += static_cast<char>(std::tolower(static_cast<unsigned char>(curr_char)));
    }
    return line_tolower;
}

bool LineEqual(const std::string& a, const std::string& b) {
    if (a.size() != b.size()) {
        return false;
    }
    for (std::size_t i=0;i<a.size();++i){
        if (std::tolower(static_cast<unsigned char>(a[i])) != std::tolower(static_cast<unsigned char>(b[i]))) {
            return false;
        }
    }
    return true;
}

bool LineContains(const std::string& a, const std::string& b) {
    const std::string a_normalize = NormalizeLine(a);
    const std::string b_normalize = NormalizeLine(b);
    return a_normalize.find(b_normalize) != std::string::npos;
}

bool LineEndsWith(const std::string& a, const std::string& b) {
    const std::string a_normalize = NormalizeLine(a);
    const std::string b_normalize = NormalizeLine(b);
    return a_normalize.ends_with(b_normalize);
}
}  // namespace nano_edr
