#include "os_handle.h"
#include <stdexcept>
#include <string>
namespace nano_edr{
OsHandle::OsHandle(const std::string& config_path){
    os_status stat=os_init(config_path.c_str(),&handle_);
    if (stat!=OS_OK){
        throw std::runtime_error("путь: "+ config_path +' '+ os_status_str(stat));
    }

}
OsHandle::~OsHandle(){
    os_free(handle_);
}
void OsHandle::Subscribe(os_event_cb callback, void* context){
    os_status sub=os_event_subscribe(handle_,callback,context);
    if (sub!=OS_OK){
        throw std::runtime_error(std::string("подписка не удалась")+ os_status_str(sub));
    }
}
void OsHandle::Start(){
    os_status start=os_start(handle_);
    if (start!=OS_OK){
        throw std::runtime_error(std::string("старт не удался")+ os_status_str(start));
    }
}
os_status OsHandle::Wait(uint32_t timeout_ms){
    os_status wait =os_wait(handle_, timeout_ms);
    return wait;
}

void OsHandle::Stop(){
    os_stop(handle_);
}



}