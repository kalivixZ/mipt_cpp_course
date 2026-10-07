#include "fields.h"
#include <charconv>
#include <stdexcept>
#include "event.h"
#include <cctype>
namespace nano_edr{

//nullptr если поле отсутствует, не ошибка, т.к. поле не обязательное;
//возвращаем указатель т.к. nullptr удобен в этой ситуации, копировать значение не к чему
    const std::string* FindField(const Event& event, const std::string& key){
    for (const Field& field: event.fields){
        if (field.key==key){
            const std::string *p=&field.value;
            return p;
        }
    }
    return nullptr;
}

//нарушение контракта формата, нет обязательного поля, значит нужно бросить исключение. 
//бросили ошибку, выведется отсутствующее поле
const std::string& GetRequiredField(const Event& event, const std::string& key){
    const std::string *p=nano_edr::FindField(event,key);
    if (!p){throw std::invalid_argument("Нет обязательного поля " + key);}
    return *p;
}

//случай с битой строкой или отсутствием поля является ожидаемым (ошибки входных данных),
//+ обработать сразу будет быстрее
bool GetIntField(const Event& event, const std::string& key, uint64_t* out){
    const std::string *p=nano_edr::FindField(event,key);
    if (!p){return false;} //если поле отсутствует
    else{ 
        uint64_t temp_out; //чтобы не записать НЕЧТОО по указателю
        const auto [ptr,ec]=std::from_chars((*p).data(),(*p).data()+(*p).size(),temp_out);
        if (ec!=std::errc() || (ptr!=(*p).data()+(*p).size())){return false;} //учитывает переполнение или если не разобралось
        *out=temp_out;
        return true;
    }
}
//отсутствие поля предусмотрено постусловием функции, а некорректное число просто ожидаемая ошибка ввода, а значит исключение не нужно
uint64_t GetIntField(const Event& event, const std::string& key,uint64_t fallback){
    const std::string *p=nano_edr::FindField(event,key);
    if (!p){return fallback;} //если поле отсутствует
    else{ 
        uint64_t temp_fallback;
        const auto [ptr,ec]=std::from_chars((*p).data(),(*p).data()+(*p).size(),temp_fallback);
        if (ec!=std::errc() || (ptr!=(*p).data()+(*p).size())){return fallback;} //учитывает переполнение или если не разобралось
        return temp_fallback;
    }
}


////
//проверка на несовпадение типа это смысл функции а значит ожидаемый исход, исключение не нужно
bool IsProcessStart(const Event& event){
    if (event.type=="process_start"){return true;}
    return false;
}
bool IsFileWrite(const Event& event){
        if (event.type=="file_write"){return true;}
    return false;
}
bool IsNetConnect(const Event& event){
            if (event.type=="net_connect"){return true;}
    return false;
}
////

//опять же, если нет необязательного поля, то это ожидаемый исход.
//если я брошу в main исключение то остановлю сразу всю программу
bool PathEndsWith(const Event& event, const std::string& suffix){
    
    const std::string *path=FindField(event,"path");
    if (!path){return false;}

    const std::string normalize_path=NormalizePath(*path);
    const std::string normalize_suffix=NormalizePath(suffix);
    if (normalize_path.ends_with(normalize_suffix)){return true;}

    return false;
}

std::string NormalizePath(const std::string& path){
    std::string path_toupper;
    std::string new_path;
    for (std::size_t i=0;i<path.size();i++){
        char symbol=path[i];
        if (symbol=='/' || symbol=='\\' || symbol=='%'){path_toupper+=symbol;continue;}
        symbol=static_cast<char>(std::toupper(static_cast<unsigned char>(symbol)));
        path_toupper+=symbol;
    }
    /*    CHECK(NormalizePath("%TMP%\\A.JS").find("\\appdata\\local\\temp\\") !=
          std::string::npos);*/
    
    for (std::size_t i=0;i<path_toupper.size();i++){
        if (path_toupper.compare(i,6,"%TEMP%")==0){
            new_path+="\\appdata\\local\\temp\\";
            i+=6;
            continue;
        }
        if (path_toupper.compare(i,5,"%TMP%")==0){
            new_path+="\\appdata\\local\\temp\\";
            i+=5;
            continue; //если конец строки
        }
        char symbol=path_toupper[i];
        if (symbol=='/' || symbol =='\\'){
            if (new_path.empty() || new_path.back() != '\\'){new_path+='\\';}else{continue;}
        }else{
            symbol=static_cast<char>(std::tolower(static_cast<unsigned char>(symbol)));
            new_path+=symbol;
        }
    }
    return new_path;
}
//поля cmdline нет - ожидаемый исход
bool CommandLineContains(const Event& event, const std::string& needle){
    const std::string* cmdline=FindField(event,"cmdline");
    if (!cmdline){return false;};
    std::string normalize_cmd=NormalizePath(*cmdline);
    std::string normalize_needle=NormalizePath(needle);

    if (normalize_cmd.find(normalize_needle) != std::string::npos){return true;}
    return false;

}
}