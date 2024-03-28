#include "includes.h"

#include "Application.h"

#include "MarkdownTestUI.h"

void Application::begin() {
    auto api_key = string(getenv("TRELLO_API_KEY"));
    auto user_token = string(getenv("TRELLO_USER_TOKEN"));

    _api = new TrelloApi(api_key, user_token);

    _home = new HomeUI(&_tasks, &_queue, _api);
    _card = new CardUI(&_tasks, &_queue, _api);

    _home->on_card_opened([this](auto card) { open_card(card); });
    _card->on_back_clicked([this] { open_home(); });

    _home->begin();
    _home->render();
}

void Application::process() { _queue.process(); }

void Application::open_card(const TrelloCard& card) {
    _card->begin(card);
    _card->render();
}

void Application::open_home() {
    _home->begin();
    _home->render();
}
