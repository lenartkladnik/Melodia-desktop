#ifndef STORAGE_HANDLER_HPP
#define STORAGE_HANDLER_HPP

// Disable warnings produced by external libs
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"

#include "../../../external/lib/httplib.h"

#pragma GCC diagnostic pop // Enable all warnings

extern const std::string api_uri;
extern httplib::Client api_cli;

bool is_api_up();
void remove_playlist(std::string playlist);
void remove_song(int id);
size_t get_next_available_song_id();
std::string create_new_playlist(int song_id);
std::string rename_playlist(std::string old_playlist, std::string new_playlist);
void add_to_playlist(std::string playlist, int song_id);
void remove_from_playlist(std::string playlist, int song_id);
std::u32string get_song_title(int id);
std::u32string get_song_artist(int id);
std::string get_song_path(int id);
std::vector<std::string> get_all_playlists();
std::vector<int> get_playlist(const std::string& name);
std::vector<int> search_all_songs(const std::u32string& query);
MultistateFuture<std::vector<std::string>>& get_autocomplete(const std::u32string& query);

#endif