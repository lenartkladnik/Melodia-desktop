#include <print>

#include "include/utils.hpp"

// Disable warnings produced by external libs
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"

#include "../../external/lib/httplib.h"
#include "../../external/lib/nlohmann/json.hpp"

#pragma GCC diagnostic pop // Enable all warnings

const std::string api_uri = "http://localhost:5400";
httplib::Client api_cli(api_uri);

bool is_api_up() {
  std::println("[INFO] Checking if API ({}) is online", api_uri);
  auto res = api_cli.Get("/");
  if (!res) {
    std::println("[ERROR] API server is not online");
    return false;
  }

  if (res->status != 200) {
    std::println("[ERROR] API server is returning status {}", res->status);
    return false;
  }

  return true;
}

void remove_playlist(std::string playlist) {
  if (!is_api_up()) return;

  api_cli.Get("/delete-playlist", httplib::Params{
    {"playlist_name", playlist}
  });
};

void remove_song(int id) {
  if (!is_api_up()) return;

  api_cli.Get("/delete-song", httplib::Params{
    {"song_id", std::to_string(id)}
  });
};

std::string create_new_playlist(int song_id) {
  if (!is_api_up()) return "";

  auto res = api_cli.Get("/create-playlist", httplib::Params{
    {"song_id", std::to_string(song_id)}
  });

  if (res && res->status == 200) {
    return res->body;
  }

  return "";
}

std::string rename_playlist(std::string old_playlist, std::string new_playlist) {
  if (!is_api_up()) return "";

  auto res = api_cli.Get("/rename-playlist", httplib::Params{
    {"old_name", old_playlist},
    {"new_name", new_playlist}
  });

  if (res && res->status == 200) {
    return res->body;
  }

  return "";
}

void add_to_playlist(std::string playlist, int song_id) {
  if (!is_api_up()) return;

  api_cli.Get("/playlist-add-item", httplib::Params{
    {"playlist_name", playlist},
    {"song_id", std::to_string(song_id)}
  });
}

void remove_from_playlist(std::string playlist, int song_id) {
  if (!is_api_up()) return;

  api_cli.Get("/playlist-remove-item", httplib::Params{
    {"playlist_name", playlist},
    {"song_id", std::to_string(song_id)}
  });
}

std::u32string get_song_title(int id) {
  if (!is_api_up()) return U"";

  auto res = api_cli.Get("/get-song", httplib::Params{
    {"song_id", std::to_string(id)}
  });

  if (res && res->status == 200) {
    return nlohmann::json::parse(res->body)["title"];
  }

  return U"";
}

std::u32string get_song_artist(int id) {
  if (!is_api_up()) return U"";

  auto res = api_cli.Get("/get-song", httplib::Params{
    {"song_id", std::to_string(id)}
  });

  if (res && res->status == 200) {
    return nlohmann::json::parse(res->body)["artist"];
  }

  return U"";
}

std::string get_song_path(int id) {
  if (!is_api_up()) return "";

  auto res = api_cli.Get("/get-song", httplib::Params{
    {"song_id", std::to_string(id)}
  });

  if (res && res->status == 200) {
    return nlohmann::json::parse(res->body)["base"];
  }

  return "";
}

std::vector<std::string> get_all_playlists() {
  if (!is_api_up()) return {};

  auto res = api_cli.Get("/get-all-playlists");

  if (res && res->status == 200) {
    return json_parse_string_list(res->body);;
  }

  return {};
}

std::vector<int> get_playlist(const std::string& name) {
  if (!is_api_up()) return {};

  auto res = api_cli.Get("/get-playlist", httplib::Params{
    {"playlist_name", name}
  });

  if (res && res->status == 200) {
    return json_parse_int_list(res->body);
  }

  return {};
}

std::vector<int> search_all_songs(const std::u32string& query) {
  if (!is_api_up()) return {};

  auto res = api_cli.Get("/search", httplib::Params{
    {"query", u32_to_utf8(query)}
  });

  if (res && res->status == 200) {
    return json_parse_int_list(res->body);
  }

  return {};
}

MultistateFuture<std::vector<std::string>> get_autocomplete(const std::u32string& query) {
  MultistateFuture<std::vector<std::string>> mf;

  if (!is_api_up()) return mf;

  mf.launch([&](){return json_parse_string_list(api_cli.Get("/fast-autocomplete", httplib::Params{
    {"query", u32_to_utf8(query)}
  })->body);}, 1);

  mf.launch([&](){return json_parse_string_list(api_cli.Get("/slow-autocomplete", httplib::Params{
    {"query", u32_to_utf8(query)}
  })->body);}, 2);

  return mf;
}