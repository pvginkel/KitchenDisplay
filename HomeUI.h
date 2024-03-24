#pragma once

#include "LvglUI.h"

#include "Queue.h"
#include "Tasks.h"
#include "TrelloApi.h"

class HomeUI : public LvglUI {
    Tasks* _tasks;
    Queue* _queue;
    TrelloApi* _api;
    result<vector<TrelloCard>, TrelloError> _cards;
    map<string, string> _card_attachment_cover_files;

public:
    HomeUI(Tasks* tasks, Queue* queue, TrelloApi* api)
        : _tasks(tasks), _queue(queue), _api(api), _cards(TrelloError::None)
    {}

protected:
    void do_render(lv_obj_t* parent) override;

private:
    void refresh_cards();
    void cards_loaded(const result<vector<TrelloCard>, TrelloError>& cards,
                      const map<string, string>& card_attachment_cover_files);
};
