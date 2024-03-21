#pragma once

template <typename Result>
static Result with_mutex(mutex& mutex, function<Result(void)> func) {
    auto guard = lock_guard<std::mutex>(mutex);

    return func();
}

static void with_mutex(mutex& mutex, function<void(void)> func) {
    auto guard = lock_guard<std::mutex>(mutex);

    func();
}
