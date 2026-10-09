#include "event_list.h"
namespace nano_edr{
void EventList::PushBack(const Event& event){
    EventNode* ev = new EventNode(event);
    if (head_==nullptr){
        head_=ev;
        tail_=ev;
        size_++;
        return;
    }
    if (capacity_>0 && size_>=capacity()){
        if (capacity_==1){
            PopFront();
            head_=ev;
            tail_=ev;
            size_++;
            return;
        }
        PopFront();
        tail_->next=ev;
        tail_=ev;
        size_++;
        return;
    }
    tail_->next=ev;
    tail_=ev;
    size_++;
}

void EventList::PopFront(){
    if (head_==nullptr){
        return;
    }
    EventNode* last_head=head_;
    if (size_==1){
        Clear();
        return;

    }
    head_=head_->next;
    delete last_head;
    size_--;
}

void EventList::Clear(){
    while (head_!=nullptr){
        EventNode* last_head=head_;
        head_=head_->next;
        delete last_head;
    }
    size_=0;
    head_=nullptr;
    tail_=nullptr;
}
EventList::~EventList(){
    Clear();
}
