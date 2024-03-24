#pragma once

#include "HomeUI.h"
#include "Queue.h"

class Application {
    Queue _queue;
    Tasks _tasks;
    HomeUI* _home;
    TrelloApi *_api;

public:
    Application() : _home(nullptr), _api(nullptr) {}

    void begin();
    void process();
};
