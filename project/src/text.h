#include <string>
namespace nano_edr{
std::string NormalizeLine(const std::string& initial_line);

//сравнение на идентичность после нормализации
bool LineEqual(const std::string& a,const std::string& b);
//вхождение
bool LineContains(const std::string&a, const std::string&b);
//сравнения на окончание
bool LineEndsWith(const std::string&a, const std::string&b);
}