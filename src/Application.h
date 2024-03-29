#pragma once

#include "CardUI.h"
#include "HomeUI.h"
#include "Queue.h"

class Application {
    Queue _queue;
    Tasks _tasks;
    HomeUI* _home;
    CardUI* _card;
    TrelloApi* _api;

public:
    Application() : _home(nullptr), _card(nullptr), _api(nullptr) {}

    void begin();
    void process();

private:
    void open_card(const TrelloCard& card);
    void open_home();
};
