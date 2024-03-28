#include "includes.h"

#include "Application.h"

#include "MarkdownTestUI.h"

static const char* TAG = "Application";

void Application::begin() {
    auto api_key = getenv("TRELLO_API_KEY");
    auto user_token = getenv("TRELLO_USER_TOKEN");

    if (!api_key) {
        LOGE(TAG, "TRELLO_API_KEY environment variable not set");
        exit(1);
    }
    if (!user_token) {
        LOGE(TAG, "TRELLO_USER_TOKEN environment variable not set");
        exit(1);
    }

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
