#include "event_source.h"
// Пропускает к следующему обработчику только события одного процесса.
namespace nano_edr{
class PidFilter : public IEventHandler {
 public:
    PidFilter(IEventHandler* next, const std::string& pid);

    void HandleEvent(const Event& event) override;
    void OnSkipped() override;
    void OnTick() override;
    void OnBroken() override;
    std::size_t filtered();  // сколько событий не пропущено

 private:
    IEventHandler* agent;
    const std::string pid;
    std::size_t filtered_ev=0;
};
}