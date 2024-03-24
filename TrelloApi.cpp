#include "includes.h"

#include "TrelloApi.h"

static const char *TAG = "TrelloApi";

typedef size_t curl_write_cb_t(void *contents, size_t size, size_t nmemb, void *userp);

bool TrelloCard::has_label(char const *label) const {
    for (auto card_label : _labels) {
        if (card_label == label) {
            return true;
        }
    }

    return false;
}

bool TrelloCard::is_match(const string& search) { return is_match(_name, search) || is_match(_description, search); }

bool TrelloCard::is_match(const string& field, const string& value) {
    if (value.empty()) {
        return true;
    }

    istringstream value_stream(value);
    string word;

    while (value_stream >> word) {
        if (field.find(word) == std::string::npos) {
            return false;
        }
    }

    return true;
}

TrelloApi::TrelloApi(string api_key, string user_token)
    : _api_key(std::move(api_key)), _user_token(std::move(user_token)) {
    _cache_path = "cache";
    filesystem::create_directory(_cache_path);
}

TrelloResult<vector<TrelloCard>> TrelloApi::get_board_cards(const string &id) {
    string data;
    attempt_res(data, get_cached(format("1/boards/{}/cards", id)));

    cJSON_Data json(cJSON_Parse(data.c_str()));
    if (!*json) {
        LOGE(TAG, "Failed to parse request JSON");
        return TrelloError::RequestInvalid;
    }

    if (!cJSON_IsArray(*json)) {
        return TrelloError::RequestInvalid;
    }

    vector<TrelloCard> cards;

    for (auto i = 0, count = cJSON_GetArraySize(*json); i < count; i++) {
        auto card_obj = cJSON_GetArrayItem(*json, i);

        if (!cJSON_IsObject(card_obj)) {
            return TrelloError::RequestInvalid;
        }

        auto labels = cJSON_GetObjectItemCaseSensitive(card_obj, "labels");

        vector<string> card_labels;

        if (cJSON_IsArray(labels)) {
            for (auto j = 0, labels_count = cJSON_GetArraySize(labels); j < labels_count; j++) {
                auto label_obj = cJSON_GetArrayItem(labels, j);
                auto label_name = cJSON_GetObjectItemCaseSensitive(label_obj, "name");

                if (!cJSON_IsString(label_name)) {
                    return TrelloError::RequestInvalid;
                }

                card_labels.push_back(cJSON_GetStringValue(label_name));
            }
        }

        auto id = cJSON_GetObjectItemCaseSensitive(card_obj, "id");
        auto desc = cJSON_GetObjectItemCaseSensitive(card_obj, "desc");
        auto id_list = cJSON_GetObjectItemCaseSensitive(card_obj, "idList");
        auto name = cJSON_GetObjectItemCaseSensitive(card_obj, "name");
        auto id_attachment_cover = cJSON_GetObjectItemCaseSensitive(card_obj, "idAttachmentCover");

        if (!cJSON_IsString(id) || !cJSON_IsString(desc) || !cJSON_IsString(id_list) || !cJSON_IsString(name)) {
            return TrelloError::RequestInvalid;
        }

        cards.push_back(
            {cJSON_GetStringValue(id), cJSON_GetStringValue(id_list), cJSON_GetStringValue(name),
             cJSON_GetStringValue(desc),
             cJSON_IsString(id_attachment_cover) ? optional(cJSON_GetStringValue(id_attachment_cover)) : nullopt,
             card_labels});
    }

    sort(cards.begin(), cards.end(), [](const TrelloCard &lhs, const TrelloCard &rhs) {
        auto result = mismatch(lhs.name().cbegin(), lhs.name().cend(), rhs.name().cbegin(), rhs.name().cend(),
                               [](const char a_char, const char b_char) { return tolower(a_char) == tolower(b_char); });

        return result.second != rhs.name().cend() &&
               (result.first == lhs.name().cend() || tolower(*result.first) < tolower(*result.second));
    });

    return cards;
}

TrelloResult<TrelloCardAttachment> TrelloApi::get_card_attachment(const string &card_id, const string &attachment_id) {
    string data;
    attempt_res(data, get_cached(format("1/cards/{}/attachments/{}", card_id, attachment_id)));

    cJSON_Data json(cJSON_Parse(data.c_str()));
    if (!*json) {
        LOGE(TAG, "Failed to parse request JSON");
        return TrelloError::RequestInvalid;
    }

    if (!cJSON_IsObject(*json)) {
        return TrelloError::RequestInvalid;
    }

    auto id = cJSON_GetObjectItemCaseSensitive(*json, "id");
    auto name = cJSON_GetObjectItemCaseSensitive(*json, "name");
    auto url = cJSON_GetObjectItemCaseSensitive(*json, "url");

    if (!cJSON_IsString(id) || !cJSON_IsString(name) || !cJSON_IsString(url)) {
        return TrelloError::RequestInvalid;
    }

    return TrelloCardAttachment(cJSON_GetStringValue(id), cJSON_GetStringValue(name), cJSON_GetStringValue(url));
}

TrelloResult<string> TrelloApi::get_cached(const string &url) {
    string file_name;
    attempt_res(file_name, get_file(url));

    ifstream file(file_name);

    stringstream buffer;
    buffer << file.rdbuf();

    return simplify_unicode(buffer.str());
}

TrelloResult<string> TrelloApi::simplify_unicode(const string& str) {
    UErrorCode status = U_ZERO_ERROR;

    // Create a UnicodeString that contains an 'e' followed by the combining grave accent (U+0300)
    icu::UnicodeString originalStr = icu::UnicodeString::fromUTF8(str.c_str());

    // Normalize the string to NFC form to combine characters
    icu::UnicodeString normalizedStr;
    icu::Normalizer::normalize(originalStr, UNORM_NFC, 0, normalizedStr, status);

    if (U_FAILURE(status)) {
        return TrelloError::RequestInvalid;
    }

    // Convert normalized string back to UTF-8 for display
    std::string utf8Result;
    normalizedStr.toUTF8String(utf8Result);

    //std::cout << "Original: " << originalStr << " | Normalized: " << utf8Result << std::endl;

    return utf8Result;
}

string TrelloApi::get_cache_key(const string &url) { return sha1(url); }

TrelloResult<string> TrelloApi::get_file(const string &url, bool force) { return get_file(url, ".cache", force); }

TrelloResult<string> TrelloApi::get_file(const string &url, const string &extension, bool force) {
    auto file_name = format("{}/{}{}", _cache_path, get_cache_key(url), extension);
    if (filesystem::exists(file_name)) {
        if (force) {
            // Force the cache to be invalidated.
            filesystem::remove(file_name);
        } else {
            return file_name;
        }
    }

    auto curl = curl_easy_init();
    if (!curl) {
        return TrelloError::Unknown;
    }

    auto authorization_header =
        format(R"(Authorization: OAuth oauth_consumer_key="{}", oauth_token="{}")", _api_key, _user_token);

    auto headers = curl_slist_append(nullptr, authorization_header.c_str());
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

    ofstream file(file_name, ios::binary);

    auto full_url = url;
    if (full_url.find("://") == string::npos) {
        full_url = format("https://api.trello.com/{}", full_url);
    }

    curl_easy_setopt(curl, CURLOPT_URL, full_url.c_str());
    curl_easy_setopt(
        curl, CURLOPT_WRITEFUNCTION, (curl_write_cb_t *)[](void *contents, size_t size, size_t nmemb, void *userp) {
            auto file = (ofstream *)userp;
            file->write((char *)contents, size * nmemb);
            return size * nmemb;
        });
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &file);
    curl_easy_setopt(curl, CURLOPT_CAINFO, getenv("CURL_CA_BUNDLE"));

    LOGI(TAG, "Requesting %s", full_url.c_str());

    auto res = curl_easy_perform(curl);

    file.close();

    curl_easy_cleanup(curl);
    curl_slist_free_all(headers);

    long status_code = 0;
    if (res == CURLE_OK) {
        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &status_code);
    }

    if (status_code < 200 || status_code >= 300) {
        filesystem::remove(file_name);

        LOGE(TAG, "Request failed %d status code %d", (int)res, (int)status_code);

        return TrelloError::RequestFailed;
    }

    return file_name;
}

TrelloResult<string> TrelloApi::get_image_file(const string &url, optional<int> width, optional<int> height,
                                               bool force) {
    auto curl = curl_session(curl_easy_init());
    if (!*curl) {
        return TrelloError::Unknown;
    }

    auto url_encoded = curl_data(curl_easy_escape(*curl, url.c_str(), 0));
    if (!*url_encoded) {
        return TrelloError::Unknown;
    }

    auto new_url = format("http://{}/lvgl/get_image.php?url={}&headers=authorization&format=bin_888&cf=true_color",
                          SRVMAIN, *url_encoded);

    if (width.has_value()) {
        new_url += format("&width={}", width.value());
    }
    if (height.has_value()) {
        new_url += format("&height={}", height.value());
    }

    return get_file(new_url, ".bin", force);
}
