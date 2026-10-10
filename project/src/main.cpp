#include <charconv>
#include "agent.h"
#include <cstdio>
#include <print>
#include <string>

int main(int argc, char** argv) {
    std::size_t value=0;
    bool quiet=false;
    bool file=false;
    std::string log_path;
    for (int i=1;i<argc;i++){
        const std::string arg=argv[i];
        if (arg=="--window-size"){
            if (i+1>=argc){
                return 2;
            }
            const std::string value_str=argv[++i];
            auto [ptr,ec]=std::from_chars(value_str.data(),value_str.data()+value_str.size(),value);
            if (ec!=std::errc() || ptr!=value_str.data()+value_str.size()){return 2;}
        }
        else if(arg=="--quiet"){
            quiet=true;
        }
        else if (arg=="--file") {
            file=true;
            if (i+1>=argc || !log_path.empty()){
                return 2;
            }
            const std::string arg=argv[++i];
            if (arg.starts_with("--")){return 2;}
            log_path=arg;
        }else{
            if (arg.starts_with("--") || !log_path.empty()) {return 2;}
            log_path = arg;
        }
    }
    if (log_path.empty()){return 2;}
    
    //агент и разделение на источники
    try{
        nano_edr::Agent agent(value,quiet);
        if (file){
            nano_edr::FileSource file(log_path);
            file.Run(&agent);
            agent.PrintSummary();
        }
        else{
            nano_edr::OsSource source(log_path);
            source.Run(&agent);
            agent.PrintSummary();
        }
    } catch(std::exception &e){
        std::print(stderr,"ошибка: {}",e.what());
        return 1;
    }
return 0;
}

