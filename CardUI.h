#pragma once

#include "LvglUI.h"
#include "Queue.h"
#include "Tasks.h"
#include "TrelloApi.h"

class CardUI : public LvglUI {
    Tasks* _tasks;
    Queue* _queue;
    TrelloApi* _api;
    optional<TrelloCard> _card;

public:
    CardUI(Tasks* tasks, Queue* queue, TrelloApi* api) : _tasks(tasks), _queue(queue), _api(api) {}

    void begin(TrelloCard card);

protected:
    void do_render(lv_obj_t* parent) override;
};
