#include <stdexcept>
#include <string>
#include <iostream>
#include "alert_sink.h"
#include "dedup_sink.h"
#include "detection.h"

namespace nano_edr {

// --- ConsoleSink -----------------------------------------------------------

void ConsoleSink::operator()(const Detection& detection) const {
    std::cout<< ToString(detection);
}

// --- FileSink --------------------------------------------------------------

FileSink::FileSink(const std::string& path) : out_(path) {
    if (!out_.is_open()) {
        throw std::runtime_error("не удалось открыть файл: " + path);
    }
}

void FileSink::operator()(const Detection& detection) {
    out_ << ToString(detection);
}

// --- CountingSink ----------------------------------------------------------

void CountingSink::operator()(const Detection&detection) const {
    ++counter_->total;
    counter_->seen.push_back(SeenAlert{detection.rule, detection.severity,
                                   detection.ts, detection.pid,
                                   detection.actions});
}

// --- FanOutSink ------------------------------------------------------------

void FanOutSink::Add(AlertSink sink) {
    sinks_.push_back(std::move(sink));
}

void FanOutSink::operator()(const Detection& detection) const {
    for (auto& el : sinks_){
        el(detection);
    }
}

// --- DedupSink -------------------------------------------------------------

DedupSink::DedupSink(AlertSink inner, uint64_t period_ms, std::size_t* suppressed)
    :inner_(std::move(inner)),
    period_ms_(period_ms),
    suppressed_(suppressed) {}
void DedupSink::operator()(const Detection& detection) {
    if (period_ms_ == 0) {
        inner_(detection);
        return;
    }
    auto key=std::make_pair(detection.rule,detection.pid);
    auto it=notes_.find(key);
    if (it==notes_.end()){
        notes_[key]=detection.ts.ms;
        inner_(detection);
        return;
    }
    if (detection.ts.ms- it->second <=period_ms_){
        if (suppressed_){++(*suppressed_);}
        return;
    }
    it->second = detection.ts.ms;
    inner_(detection);
}

}  // namespace nano_edr
