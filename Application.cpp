#include "includes.h"

#include "Application.h"

void Application::begin() {
    auto api_key = string(getenv("TRELLO_API_KEY"));
    auto user_token = string(getenv("TRELLO_USER_TOKEN"));

    _api = new TrelloApi(api_key, user_token);

    _home = new HomeUI(&_tasks, &_queue, _api);
    _home->begin();
    _home->render();
}

void Application::process() { _queue.process(); }
