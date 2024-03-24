#pragma once

string sha1(const string& input);

template <typename Result>
static Result with_mutex(mutex& mutex, function<Result(void)> func) {
    auto guard = lock_guard<std::mutex>(mutex);

    return func();
}

static void with_mutex(mutex& mutex, function<void(void)> func) {
    auto guard = lock_guard<std::mutex>(mutex);

    func();
}

template <typename T, typename E>
class result {
    optional<T> _value;
    E _error;

public:
    result(T value) : _value(value) {}
    result(E error) : _error(error) {}

    bool is_ok() const { return _value.has_value(); }

    T& value() {
        assert(is_ok());
        return _value.value();
    }

    E error() const {
        assert(!is_ok());
        return _error;
    }

    bool is_error(E error) const { return !is_ok() && _error == error; }
};

template <typename E>
class result<void, E> {
    bool _is_ok;
    E _error;

public:
    result() : _is_ok(true) {}
    result(E error) : _is_ok(false), _error(error) {}

    bool is_ok() const { return _is_ok; }

    void value() const { assert(is_ok()); }

    E error() const {
        assert(!is_ok());
        return _error;
    }

    bool is_error(E error) const { return !is_ok() && _error == error; }
};

#define attempt(e)                     \
    do {                               \
        auto tmp_result = (e);         \
        if (!tmp_result.is_ok()) {     \
            return tmp_result.error(); \
        }                              \
    } while (0)

#define attempt_res(t, e)              \
    do {                               \
        auto tmp_result = (e);         \
        if (!tmp_result.is_ok()) {     \
            return tmp_result.error(); \
        }                              \
        (t) = tmp_result.value();      \
    } while (0)

#define LOGE(tag, format, ...) printf("\033[31mERROR [%s] " format "\033[0m\n", tag, ##__VA_ARGS__)
#define LOGW(tag, format, ...) printf("\033[33mWARN [%s] " format "\033[0m\n", tag, ##__VA_ARGS__)
#define LOGI(tag, format, ...) printf("\033[97mINFO [%s] " format "\033[0m\n", tag, ##__VA_ARGS__)
#define LOGD(tag, format, ...) printf("DEBUG [%s] " format "\n", tag, ##__VA_ARGS__)

class cJSON_Data {
    cJSON* _data;

public:
    cJSON_Data(cJSON* data) : _data(data) {}
    cJSON_Data(const cJSON_Data& other) = delete;
    cJSON_Data(cJSON_Data&& other) noexcept = delete;
    cJSON_Data& operator=(const cJSON_Data& other) = delete;
    cJSON_Data& operator=(cJSON_Data&& other) noexcept = delete;

    ~cJSON_Data() {
        if (_data) {
            cJSON_Delete(_data);
        }
    }

    cJSON* operator*() const { return _data; }
};

class curl_session {
    CURL* _session;

public:
    curl_session(CURL* session) : _session(session) {}
    curl_session(const curl_session& other) = delete;
    curl_session(curl_session&& other) noexcept = delete;
    curl_session& operator=(const curl_session& other) = delete;
    curl_session& operator=(curl_session&& other) noexcept = delete;

    ~curl_session() {
        if (_session) {
            curl_easy_cleanup(_session);
        }
    }

    CURL* operator*() const { return _session; }
};

template <typename T>
class curl_data {
    T _data;

public:
    curl_data(T session) : _data(session) {}
    curl_data(const curl_data& other) = delete;
    curl_data(curl_data&& other) noexcept = delete;
    curl_data& operator=(const curl_data& other) = delete;
    curl_data& operator=(curl_data&& other) noexcept = delete;

    ~curl_data() {
        if (_data) {
            curl_free(_data);
        }
    }

    T operator*() const { return _data; }
};
