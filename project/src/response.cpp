#include <string>

#include "detection.h"
#include "entity_model.h"
#include "event.h"
#include "os_handle.h"
#include "response.h"
#include "fields.h"
#include <print>

namespace nano_edr {

// --- ResponsePlanner -------------------------------------------------------

ResponsePlanner::ResponsePlanner(const EntityModel* model,
                                 std::string quarantine_dir)
    : model_(model), quarantine_dir_(quarantine_dir) {}

bool SystemPath(const std::string& path){
    std::string path_toupper;
    for (std::size_t i = 0; i < path.size(); i++) {
        char symbol = path[i];
        symbol = static_cast<char>(std::toupper(static_cast<unsigned char>(symbol)));
        path_toupper += symbol;
    }
    return path_toupper.rfind("C:\\WINDOWS\\", 0) == 0 ||
           path_toupper.rfind("C:\\PROGRAM FILES\\", 0) == 0;
}

ResponsePlan ResponsePlanner::Plan(const Detection& detection) const {
    ResponsePlan plan;
    // процесс детекта и все его потомки; процесс детекта стоит первым
    const std::vector<std::string> tree =
        model_->GetProcessTree(detection.pid);
    for (const std::string& name : detection.actions) {
        if (name == "kill_process") {
            for (std::size_t i = tree.size(); i > 0; --i) {
            const std::string& pid = tree[i - 1];
            Action action;
            action.kind = ActionKind::kKillProcess;
            action.pid = pid;
            // запись есть у каждого процесса из tree;
            // start — ноль, если запуска процесса модель не видела
            action.expected_start = model_->FindProcess(pid)->start;
            action.reason = detection.rule;
            if (pid != detection.pid) {
                action.reason += ", потомок " + detection.pid;
            }
            plan.Add(std::move(action));
            }
        } else if (name == "quarantine_source") {
            if (model_==nullptr){continue;}
            const FileRecord* source = model_->FindSpawnSource(detection.pid);
            if (source != nullptr) {
                if (SystemPath(source->path)) {
                continue;
                }
                Action action;
                action.kind = ActionKind::kQuarantineFile;
                action.path = source->path;
                action.to = QuarantinePath(source->path, detection.pid);
                action.reason = detection.rule + ", источник запуска";
                if (action.to.empty()) {
                continue;
                }
                plan.Add(std::move(action));
            }else{continue;}
        }else if (name == "quarantine_image") {
            if (model_ == nullptr) {
                continue;
            }
            const ProcessRecord* process = model_->FindProcess(detection.pid);
            if (process == nullptr || process->image.empty() ||
                SystemPath(process->image)) {
                continue;
            }
            Action action;
            action.kind = ActionKind::kQuarantineFile;
            action.path = process->image;
            action.to = QuarantinePath(process->image, detection.pid);
            action.reason = detection.rule + ", образ процесса";
            if (action.to.empty()) {
                continue;
            }
            plan.Add(std::move(action));
        }else if (name == "delete_dropped") {
            if (model_ == nullptr) {
                continue;
            }
            const std::vector<std::string> tree =
            model_->GetProcessTree(detection.pid);
            for (const std::string& pid : tree) {
                const auto files = model_->GetFilesCreatedBy(pid, true);
                for (const std::string& path : files) {
                    const FileRecord* file = model_->FindFile(path);
                    if (file == nullptr || SystemPath(file->path)) {
                        continue;
                    }
                    Action action;
                    action.kind = ActionKind::kDeleteFile;
                    action.path = file->path;
                    action.reason = detection.rule + ", принесен "+pid;
                    plan.Add(std::move(action));
                }
            }
        }else if (name == "delete_evidence") {
            for (const auto& event : detection.evidence) {
                if (event == nullptr) {
                    continue;
                }

                const std::string& field_name =
                    (event->type() == "file_move") ? "to" : "path";

                const std::string* field = FindField(*event, field_name);

                if (field == nullptr || SystemPath(*field)) {
                    continue;
                }
                Action action;
                action.kind = ActionKind::kDeleteFile;
                action.path = *field;
                action.reason = detection.rule + ", файл из детекта";
                plan.Add(std::move(action));
            }
    }
        // незнакомое имя пропускается
    }
    return plan;
}

std::string ResponsePlanner::QuarantinePath(const std::string&path,const std::string&pid) const {
    if (quarantine_dir_.empty()){return "";}
    else{
        const std::size_t pos = path.find_last_of('\\');
        const std::string file_name=path.substr(pos+1);
        return quarantine_dir_+'\\'+pid+'-'+file_name;
    }
}
// --- Responder -------------------------------------------------------------

Responder::Responder(OsHandle* os, bool dry_run) : os_(os), dry_run_(dry_run) {}

void Responder::Execute(const ResponsePlan& plan) {
    for (const Action& action : plan.actions()) {
        ++stats_.planned;

        // цель, по которой уже отработали: на одном процессе срабатывает
        // несколько правил
        bool repeat = false;
        for (const std::string& target : done_) {
            if (target == action.target()) {
                repeat = true;
                break;
            }
        }
        if (repeat) {
            ++stats_.skipped_repeat;
            continue;
        }
        done_.push_back(action.target());

        if (dry_run_ || os_ == nullptr) {
            std::print("[PLAN] {}\n", ToString(action));
            continue;
        }
        if (action.kind==ActionKind::kKillProcess){Kill(action);
        } else if (action.kind == ActionKind::kQuarantineFile) {
            MoveToQuarantine(action);
        } else if (action.kind == ActionKind::kDeleteFile) {
            Delete(action);
        }
    }
}


}  // namespace nano_edr
