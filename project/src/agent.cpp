#include "agent.h"
#include "alert_sink.h"
#include <print>
#include <algorithm>
#include "agent_rules.h"
#include "copy_stats.h"
#include "event.h"
#include "dedup_sink.h"
#include "window_entry.h"
namespace nano_edr {
Agent::Agent(bool quiet, bool copies, uint64_t dedup_period_ms)
    : engine_(BuildAgentRules(&model_)),
      quiet_(quiet),
      copies_(copies),
      dedup_period_ms_(dedup_period_ms) {
    engine_.SetModel(&model_);
}
void Agent::HandleEvent(const Event& event) {
    print_context=false;
    model_.Observe(event);
    if (responder_ != nullptr && responder_->Recognize(event)) {
        // запись в window_, как для обычного события
        return;  // до ProcessEvent своё событие не доходит
    }
    engine_.ProcessEvent(event,&fired);
    WindowEntry entry;
    entry.ts = event.ts();
    entry.type = event.type();
    entry.pid = event.pid();
    window_.PushBack(std::move(entry));
    event_cnt_++;
    bool flag = false;
    for (auto& [type, cnt] : all_types_) {
        if (event.type() == type) {
            cnt++;
            flag = true;
            break;
        }
    }
    if (!flag) {
        all_types_.push_back({event.type(), 1});
    }
}

void Agent::EnableResponse(OsHandle* os, bool dry_run,
                           const std::string& quarantine_dir) {
    planner_ = std::make_unique<ResponsePlanner>(&model_, quarantine_dir);
    responder_ = std::make_unique<Responder>(os, dry_run);
    engine_.Subscribe([this](const Detection& detection) {
        responder_->Execute(planner_->Plan(detection));
    });
}


void Agent::Subscribe(AlertSink sink) {
    FanOutSink all;
    all.Add([this](const Detection&) { PrintContextOnce(); });
    all.Add(std::move(sink));
    engine_.Subscribe(DedupSink(std::move(all), dedup_period_ms_, &suppressed_));
}

void Agent::PrintContextOnce() {
    if (print_context){return;}
    print_context=true;
    if (!quiet_) {
        for (std::size_t i = window_.size()- std::min(window_.size(),std::size_t(2));i<window_.size();++i){
            const WindowEntry& entry = window_[i];
            std::print("[CTX] -{}: {} {} pid={}\n",window_.size()-i, entry.ts.ms, entry.type, entry.pid);
        }
    }

}

void Agent::PrintSummary() const {
    if (copies_){
        std::print("[COPY] строк события скопировано {}, перемещено {}\n",copy_stats().event_strings_copied, copy_stats().event_strings_moved);
        std::print("[COPY] записей WindowEntry скопировано {}, перемещено {}\n",copy_stats().window_copied,copy_stats().window_moved);
        std::print("[COPY] событий скопировано в детекты {}\n",copy_stats().evidence_copied);
        std::print("[COPY] копий всего {}\n",copy_stats().total());
    }
    if (quiet_) {
        return;
    }
    std::print("пустых строк или комментариев {}, битых строк{}\n",skipped_line_,broken_line_);
    std::print("всего событий: {}, каждого типа:\n", event_cnt_);
    for (const auto& [type, cnt] : all_types_) {
        std::print("{} : {}\n", type, cnt);
    }
    for (std::size_t i = 0; i < engine_.size(); ++i) {
        std::print("{}  {}\n", engine_.rule(i).id(), engine_.rule(i).hits());
    }
    std::print("[STAT] в окне {} из {} мест\n",window_.size(),window_.capacity());
    std::print("[STAT] в модели процессов {}, файлов {}\n",model_.process_count(),model_.file_count());
    if (suppressed_!=0){
        std::print("[STAT] подавлено дубликатов {} (срок {} мс\n",suppressed_,dedup_period_ms_);
    }
    if (responder_){
        std::print("[STAT] действий запланировано {}, повторов отброшено{}\n",responder_->stats().planned,responder_->stats().skipped_repeat());
        std::print("[STAT] завершено процессов {}, изолировано файлов {}, удалено файлов {}\n",responder_->stats().killed, responder_->stats().quarantined, responder_->stats().deleted);
        std::print("[STAT] своих событий {}\n", responder_->stats().own_events);
    }
}   

void Agent::OnSkipped() {
    ++skipped_line_;
}
void Agent::OnBroken() {
    ++broken_line_;
}
void Agent::OnTick() {
    ++tick_;
}
}  // namespace nano_edr