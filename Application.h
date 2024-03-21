#pragma once

#include "HomeUI.h"
#include "Queue.h"

class Application {
    Queue* _queue;
    HomeUI _home;

public:
    Application(Queue& queue) : _queue(&queue) {}

    void begin();
};
