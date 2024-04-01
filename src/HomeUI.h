#pragma once
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
    map<string, lv_obj_t*> _card_attachment_cover_images;
    map<string, string> _card_attachment_cover_files;
    lv_obj_t* _keyboard;
    string _search;
    Callback<const TrelloCard&> _card_opened;

public:
    HomeUI(Tasks* tasks, Queue* queue, TrelloApi* api)
        : _tasks(tasks), _queue(queue), _api(api), _cards(TrelloError::None), _keyboard(nullptr) {}

    void on_card_opened(function<void(const TrelloCard&)> func) { _card_opened.add(func); }

protected:
    void do_render(lv_obj_t* parent) override;

private:
    void refresh_cards();
    void cards_loaded(const result<vector<TrelloCard>, TrelloError>& cards);
    void load_attachment_covers(vector<TrelloCard> cards);
    void delete_keyboard();
    void open_card(const string& string);
    void on_quit();
    void quit();
};
