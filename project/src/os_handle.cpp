#include "os_handle.h"

#include <stdexcept>
#include <string>
namespace nano_edr {
OsHandle::OsHandle(const std::string& config_path) {
    os_status stat = os_init(config_path.c_str(), &handle_);
    if (stat != OS_OK) {
        throw std::runtime_error("путь: " + config_path + ' ' + os_status_str(stat));
    }
}
OsHandle::~OsHandle() {
    os_free(handle_);
}
void OsHandle::Subscribe(os_event_cb callback, void* context) {
    os_status sub = os_event_subscribe(handle_, callback, context);
    if (sub != OS_OK) {
        throw std::runtime_error(std::string("подписка не удалась: ") + os_status_str(sub));
    }
}
void OsHandle::Start() {
    os_status start = os_start(handle_);
    if (start != OS_OK) {
        throw std::runtime_error(std::string("старт не удался: ") + os_status_str(start));
    }
}
os_status OsHandle::Wait(uint32_t timeout_ms) {
    os_status wait = os_wait(handle_, timeout_ms);
    return wait;
}

void OsHandle::Stop() {
    os_stop(handle_);
}

os_status OsHandle::Kill(uint32_t pid, uint64_t expected_start_time) {
    return os_process_kill(handle_, pid, expected_start_time);
}

os_status OsHandle::MoveFile(const std::string& from, const std::string& to) {
    return os_file_move(handle_, from.c_str(), to.c_str());
}

os_status OsHandle::DeleteFile(const std::string& path) {
    return os_file_delete(handle_, path.c_str());
}

os_status OsHandle::ProcessInfo(uint32_t pid, os_process_info* out) {
    return os_process_get_info(handle_, pid, out);
}

}  // namespace nano_edr