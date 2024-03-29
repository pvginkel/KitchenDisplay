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
    Callback<void> _back_clicked;

public:
    CardUI(Tasks* tasks, Queue* queue, TrelloApi* api) : _tasks(tasks), _queue(queue), _api(api) {}

    void begin(TrelloCard card);
    void on_back_clicked(function<void()> func) { _back_clicked.add(func); }

protected:
    void do_render(lv_obj_t* parent) override;
};
