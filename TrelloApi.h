#pragma once

enum class TrelloError { None, Unknown, RequestFailed, RequestInvalid };

template <typename T>
using TrelloResult = result<T, TrelloError>;

class TrelloCard {
    string _id;
    string _id_list;
    string _name;
    string _description;
    optional<string> _id_attachment_cover;
    vector<string> _labels;

public:
    TrelloCard(string id, string id_list, string name, string description, optional<string> id_attachment_cover,
               vector<string> labels)
        : _id(std::move(id)),
          _id_list(std::move(id_list)),
          _name(std::move(name)),
          _description(std::move(description)),
          _id_attachment_cover(std::move(id_attachment_cover)),
          _labels(std::move(labels)) {}

    const string& id() const { return _id; }
    const string& id_list() const { return _id_list; }
    const string& name() const { return _name; }
    const string& description() const { return _description; }
    const optional<string>& id_attachment_cover() const { return _id_attachment_cover; }
    const vector<string>& labels() const { return _labels; }
    bool has_label(char const* label) const;
    bool is_match(const string& search);

private:
    bool is_match(const string& field, const string& value);
};

class TrelloCardAttachment {
    string _id;
    string _name;
    string _url;

public:
    TrelloCardAttachment(string id, string name, string url)
        : _id(std::move(id)), _name(std::move(name)), _url(std::move(url)) {}

    const string& id() const { return _id; }
    const string& name() const { return _name; }
    const string& url() const { return _url; }
};

class TrelloApi {
    string _api_key;
    string _user_token;
    string _cache_path;

public:
    TrelloApi(string api_key, string user_token);

    TrelloResult<vector<TrelloCard>> get_board_cards(const string& board_id);
    TrelloResult<TrelloCardAttachment> get_card_attachment(const string& card_id, const string& attachment_id);
    TrelloResult<string> get_file(const string& url, bool force = false);
    TrelloResult<string> get_image_file(const string& url, optional<int> width, optional<int> height, bool force = false);

private:
    TrelloResult<string> get_file(const string& url, const string& extension, bool force = false);
    TrelloResult<string> get_cached(const string& url);
    TrelloResult<string> simplify_unicode(const string& str);
    string get_cache_key(const string& url);
};
