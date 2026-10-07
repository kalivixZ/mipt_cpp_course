#include "event_list.h"

namespace nano_edr {

void ListPushBack(EventList* list, const Event* event) {
    if (list == nullptr || event == nullptr) {
        return;
    }

    if (list->capacity != 0 && list->size >= list->capacity) {
        ListPopFront(list);
    }

    EventNode* node = new EventNode{*event, nullptr};
    if (list->tail == nullptr) {
        list->head = node;
        list->tail = node;
    } else {
        list->tail->next = node;
        list->tail = node;
    }
    ++list->size;
}

void ListPopFront(EventList* list) {
    if (list == nullptr || list->head == nullptr) {
        return;
    }

    EventNode* old_head = list->head;
    list->head = old_head->next;
    delete old_head;
    --list->size;

    if (list->head == nullptr) {
        list->tail = nullptr;
        list->size = 0;
    }
}

void ListClear(EventList* list) {
    if (list == nullptr) {
        return;
    }

    while (list->head != nullptr) {
        ListPopFront(list);
    }

    list->tail = nullptr;
    list->size = 0;
}

EventList::~EventList() {
    ListClear(this);
}

}  // namespace nano_edr
