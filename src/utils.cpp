#include <SFML/Graphics.hpp>
#include <string>
#include <cctype>
#include <unicode/uchar.h>
#include <unicode/unistr.h>
#include <unicode/utypes.h>

#include "include/data.hpp"
#include "include/components.hpp"
#include "include/api_handler.hpp"
#include "include/signals.hpp"

#ifndef _WIN32
  #define STB_IMAGE_IMPLEMENTATION
  #define STB_IMAGE_WRITE_IMPLEMENTATION
#endif
#define STB_IMAGE_RESIZE_IMPLEMENTATION

// Disable warnings produced by external libs
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-but-set-variable"
#pragma GCC diagnostic ignored "-Wmissing-field-initializers"

#include "../external/lib/SFC/Svg.hpp"

#pragma GCC diagnostic pop // Enable all warnings

#ifdef _WIN32
  #define POPEN _popen
  #define PCLOSE _pclose
#else
  #define POPEN popen
  #define PCLOSE pclose
#endif

using icu::UnicodeString;

void debug_draw_bounds(sf::RenderTexture& window, sf::FloatRect bounds) {
  sf::RectangleShape rect;
  if (bounds.size == sf::Vector2f(0, 0))
    std::cout << "[WARN] debug_draw_bounds: Size is 0, 0\n";
  rect.setSize(bounds.size);
  if (bounds.position == sf::Vector2f(0, 0))
    std::cout << "[WARN] debug_draw_bounds: Position is 0, 0\n";
  rect.setPosition(bounds.position);
  rect.setFillColor(sf::Color(255, 0, 0, 128));

  window.draw(rect);
}


void set_window(sf::State state) {
  is_fullscreen = state == sf::State::Fullscreen;
  render_window.create(sf::VideoMode(window_base_size), "Melodia", sf::Style::Default, state, window_settings);
}

void draw_window(sf::RenderWindow& render_window, sf::RenderTexture& window) {
  window.display();
  sf::Sprite scene(window.getTexture());

  render_window.draw(scene);

  render_window.display();
}

bool resize_image(std::string path, std::string output, sf::Vector2u target_size) {
  int w, h, channels;
  unsigned char* data = stbi_load(path.c_str(), &w, &h, &channels, 0);

  if (!data) {
    throw std::runtime_error("Failed to decode image from " + path + ": " + stbi_failure_reason());
    return false;
  }

  std::vector<unsigned char> resized(target_size.x * target_size.y * channels);

  stbir_pixel_layout layout;
  switch (channels) {
    case 1: layout = STBIR_1CHANNEL; break;
    case 2: layout = STBIR_2CHANNEL; break;
    case 3: layout = STBIR_RGB; break;
    case 4: layout = STBIR_RGBA; break;
    default:
      throw std::runtime_error("Unsupported channel count for cover art image.");
      stbi_image_free(data);
      return false;
  }

  stbir_resize(
    data, w, h, 0,
    resized.data(), target_size.x, target_size.y, 0,
    layout,
    STBIR_TYPE_UINT8,
    STBIR_EDGE_CLAMP,
    STBIR_FILTER_DEFAULT
  );

  if (!stbi_write_png(output.c_str(), target_size.x, target_size.y, channels, resized.data(), target_size.x * channels)) {
    throw std::runtime_error("Failed to write resized image.");
    stbi_image_free(data);
    return false;
  }

  stbi_image_free(data);

  return true;
}

bool rasterize_texture(std::string name) {
  std::cout << "[INFO] Rasterizing '" << name << "'\n";

  sfc::SVGImage svg;
  if (svg.loadFromFile(base_path_misc + name + ".svg")) {
    if (std::filesystem::exists(base_path_misc_rasters + name + ".png")) {
      std::filesystem::remove(base_path_misc_rasters + name + ".png");
    }
    auto png_path = base_path_misc_rasters + name + ".png";
    if (!svg.rasterize(2.f).saveToFile(png_path)) {
      return false;
    }

    sf::Vector2u size;
    try {
      std::string icon_id = name;
      if (name.find(inverted_image_suffix) != std::string::npos) {
        icon_id = name.erase(name.find(inverted_image_suffix), inverted_image_suffix.size());
      }
      size = icon_sizes.at(icon_id);
      resize_image(png_path, png_path, size);
    } catch (const std::out_of_range&) {
      // do nothing
    }

    return true;
  }

  return false;
}

void rasterize_textures() {
  for (const auto& entry : std::filesystem::directory_iterator{base_path_misc}) {
    if (!std::filesystem::exists(base_path_misc_rasters + entry.path().stem().string() + ".png") && entry.path().extension().string() == ".svg") {
      rasterize_texture(entry.path().stem().string());
    }
  }
}

std::shared_ptr<sf::Texture> load_texture(std::string name, bool no_invert) {
  if (dark_mode && !no_invert) {
    // First try to load an inverted variant if there is one present
    try {
      return load_texture(name + inverted_image_suffix, true);
    } catch (const std::runtime_error&) {
      // Load the normal texture and invert manually
    }
  }

  if (!std::filesystem::exists(base_path_misc_rasters + name + ".png")) {
    throw std::runtime_error("Failed to load texture, invalid name '" + name + "'.\n");
  }

  sf::Image im;
  if (!im.loadFromFile(base_path_misc_rasters + name + ".png")) {
    throw std::runtime_error("Failed to load image for '" + name + "'.");
  }
  // Invert the colors of the texture if dark mode is on
  if (dark_mode && !no_invert) {
    auto size = im.getSize();
    for (unsigned int y = 0; y < size.y; y++) {
      for (unsigned int x = 0; x < size.x; x++) {
        auto pixel = im.getPixel({x, y});
        // Only invert mostly black pixels
        if (is_color_black(pixel)) {
          pixel.r = soft_white - pixel.r;
          pixel.g = soft_white - pixel.g;
          pixel.b = soft_white - pixel.b;
          im.setPixel({x, y}, pixel);
        }
      }
    }
  }
  auto tex = std::make_shared<sf::Texture>();
  if (!tex->loadFromImage(im)) {
    throw std::runtime_error("Failed to load texture for '" + name + "'.");
  }
  tex->setSmooth(true);
  return tex;
}

bool is_string_valid_name(const std::string& s) {
  if (s.empty()) return false;

  // Invalid character list from: https://stackoverflow.com/questions/4814040/allowed-characters-in-filename
  return std::count_if(s.begin(), s.end(), [](unsigned char c){ return (
    c == '\0' ||
    c == '\\' ||
    c == '/'  ||
    c == ':'  ||
    c == '*'  ||
    c == '?'  ||
    c == '"'  ||
    c == '<'  ||
    c == '>'  ||
    c == '|'  ||
    c == '.'
  );}) == 0;
}

// Adapted from https://www.sfml-dev.org/tutorials/3.1/migration/sfml-3.0/#character-positions-in-text
sf::Vector2f find_character_pos(const sf::Text& text, size_t index) {
  const auto& glyphs = text.getShapedGlyphs();
  if (glyphs.empty())
    return text.getTransform().transformPoint({});

  for (const auto& g : glyphs) {
    if (g.cluster == index)
      return text.getTransform().transformPoint(g.position);
  }

  return {text.getPosition().x + text.getGlobalBounds().size.x, text.getPosition().y + text.getGlobalBounds().size.y}; // Very end of the string
}

sf::Vector2f find_character_size(const sf::Text& text, size_t index) {
  const auto& glyphs = text.getShapedGlyphs();
  if (glyphs.empty())
    return {0.f, 0.f};

  float width = 0.f;
  float height = 0.f;
  for (const auto& g : glyphs) {
    if (g.cluster == index) {
      width += g.glyph.bounds.size.x;
      if (g.glyph.bounds.size.y > height)
        height = g.glyph.bounds.size.y;
    }
  }

  return {width, height};
}

size_t find_character_at_pos_x(const std::u32string& string, const sf::Text& text, float pos_x) {
  size_t char_pos = string.size(); // This will be returned if all of the glyphs are behind pos_x
  for (size_t i = 0; i < string.size(); i++) {
    if (find_character_pos(text, i).x + (find_character_size(text, i).x / 2) > pos_x) {
      char_pos = i;
      break;
    }
  }
  return char_pos;
}

std::u32string utf8_to_u32(const std::string& utf8) {
    UnicodeString ustr = UnicodeString::fromUTF8(utf8); // UTF-8 -> UTF-16 internally

    UErrorCode err = U_ZERO_ERROR;
    int32_t capacity = ustr.length() + 1; // upper bound; UTF-32 length <= UTF-16 length
    std::u32string result(capacity, 0);

    int32_t written = ustr.toUTF32(
        reinterpret_cast<UChar32*>(&result[0]), capacity, err);

    if (U_FAILURE(err)) {
        throw std::runtime_error("UTF-8 to UTF-32 conversion failed");
    }
    result.resize(written);
    return result;
}

std::string u32_to_utf8(const std::u32string& u32) {
    UnicodeString ustr = UnicodeString::fromUTF32(
        reinterpret_cast<const UChar32*>(u32.data()),
        static_cast<int32_t>(u32.length()));

    std::string result;
    ustr.toUTF8String(result);
    return result;
}

char32_t to_lower_u32(char32_t c) {
    return static_cast<char32_t>(u_tolower(static_cast<UChar32>(c)));
}

std::u32string lower_u32(std::u32string s) {
    std::transform(s.begin(), s.end(), s.begin(), to_lower_u32);
    return s;
}

std::string seconds_to_human_readable(float total_sec_left) {
  int minutes = (int)(total_sec_left / 60); // floor positive value
  std::string seconds = std::to_string((int)((int)total_sec_left % 60));

  if (seconds.size() < 2) {
    seconds = "0" + seconds;
  }

  return std::to_string(minutes) + ":" + seconds;
}

float getFontOffsetPixels(float target_size) {
  // This maps approximately what the results of font.getLineSpacing(*_font_size) for the
  // Inter font at certain font sizes would be. (Because that font looks normal in the UI)

  // This is a linear function fitted for these points:
  // (32.4, 21.5)
  // (36.0, 23.5)
  // (39.6, 26.5)
  // (43.2, 28.5)
  auto normal_line_spacing = 0.666667f * target_size - 0.2;

  return default_font.getLineSpacing(target_size) - normal_line_spacing;
}

void setFontSize(sf::Text& text, float target_size, unsigned int raster_mul) {
  auto raster_size = target_size * raster_mul;
  float scale = 1.0f / raster_mul;

  text.setCharacterSize((unsigned int)raster_size);
  text.setScale({scale, scale});

  auto offset = getFontOffsetPixels(target_size);
  text.setOrigin({0, offset});
}

void reset_globals() {
  // All of the std::vector objects that get cleared here contain
  // some pointers to objects in memory that might have changed

  search_res_click_events.clear();
  search_results.clear();
  click_events.clear();
  popup_components.clear();
  text_events.clear();
  kb_events.clear();
  focus_events.clear();
  scroll_events.clear();

  copy_signal.reset();
  paste_signal.reset();
  select_all_signal.reset();
  play_toggle_signal.reset();
  confirm_signal.reset();
  left_signal.reset();
  right_signal.reset();
  escape_signal.reset();

  // Reset the global z-index since
  // all the objects must be redrawn
  global_z_index = 0;
}

sf::Color sub_colors(sf::Color a, sf::Color b) {
  return sf::Color({(uint8_t)(a.r - b.r), (uint8_t)(a.g - b.g), (uint8_t)(a.b - b.b), (uint8_t)(a.a - b.a)});
}

sf::Color add_colors(sf::Color a, sf::Color b) {
  return sf::Color({(uint8_t)(a.r + b.r), (uint8_t)(a.g + b.g), (uint8_t)(a.b + b.b), (uint8_t)(a.a + b.a)});
}

sf::Color add_int_to_color(sf::Color a, int b) {
  return sf::Color({(uint8_t)(a.r + b), (uint8_t)(a.g + b), (uint8_t)(a.b + b)});
}

float dot_colors(sf::Color a, float wr, float wg, float wb) {
  return (a.r / 255.f) * wr + (a.g / 255.f) * wg + (a.b / 255.f) * wb;
}

sf::Color adjust_if_dark_mode(sf::Color a) {
  if (dark_mode) {
    float gray = dot_colors(a, 0.299f, 0.587f, 0.114f);
    float invf = 1.f - gray;
    uint8_t invu = (uint8_t)(invf * (float)(soft_white) + 0.5f);
    return sf::Color(invu, invu, invu, a.a);
  }
  return a;
}

bool color_less_than_color(sf::Color a, sf::Color b) {
  return a.r < b.r && a.g < b.g && a.b < b.b;
}

bool is_color_black(sf::Color a) {
  return color_less_than_color(a, sf::Color{black_threshold, black_threshold, black_threshold});
}

void start_drag_and_drop() {
  // For was_unintentional_drag_and_drop
  started_dragging_time = std::chrono::high_resolution_clock::now();
}

bool was_unintentional_drag_and_drop() {
  std::chrono::duration<float> dragging_time = std::chrono::high_resolution_clock::now() - started_dragging_time; // How long the item was dragged for

  if (dragging_time.count() < min_drag_and_drop_time) {
    return true;
  }

  return false;
}

std::string title_string(const std::string& s) {
  std::string result;
  bool next_upper = true;

  for (auto c : s) {
    if (next_upper) {
      c = std::toupper(c);
      next_upper = false;
    }

    if (c == ' ') {
      next_upper = true;
    }

    result += c;
  }

  return result;
}

std::vector<std::string> json_parse_string_list(std::string json_s) {
  std::vector<std::string> results;
  for (const std::string field : nlohmann::json::parse(json_s)) {
    results.push_back(field);
  }
  return results;
}

std::vector<int> json_parse_int_list(std::string json_s) {
  std::vector<int> results;
  for (const std::string field : nlohmann::json::parse(json_s)) {
    try {
      results.push_back(std::stoi(field));
    } catch (const std::invalid_argument&) {}
  }
  return results;
}

void cleanup(int code) {
  std::cout << "[INFO] Exiting...\n";
  exit(code);
}
