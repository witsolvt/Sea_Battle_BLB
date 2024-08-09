#ifndef SEABATTLE_GAME_H
#define SEABATTLE_GAME_H

#include <ncurses.h>
#include <atomic>
#include <deque>
#include <vector>
#include <mutex>
#include <string>
#include "field.h"

#define WINDOW_HEIGHT 40
#define WINDOW_WIDTH 150
#define FIELD_WIDTH 41
#define BETWEEN_FIELDS ((WINDOW_WIDTH - FIELD_WIDTH * 2) / 3)

class GAME {
public:
    GAME();
    int manage_menu(WINDOW* menu_win); // 0 - quit, 1 - start single player game, 2 - two player game
    void manage_singleplayer_ship_placement(WINDOW *game_win);
    void manage_singleplayer_fight (WINDOW *game_win);
    void manage_multiplayer_ship_placement (WINDOW *game_win);
    void manage_multiplayer_fight (WINDOW *game_win);
private:
    static void draw_singleplayer_fight_interface(WINDOW *game_win, FIELD& player);
    static void draw_multiplayer_fight_interface(WINDOW *game_win);
    static void draw_placing_interface(WINDOW *game_win, int color = 0);
    static void ships_auto_place (FIELD& field, int ship_size = 3);
    static void place_player_ships (WINDOW *game_win, FIELD& field, int color = 0);
    void print_menu(WINDOW *menu_win, size_t highlight) const;
    static void draw_grid(WINDOW *game_win, coordinates window);
    static void draw_player_ships (WINDOW *game_win, coordinates window, const FIELD & field);
    static void draw_enemy_ships (WINDOW *game_win, coordinates window, const FIELD & field);
    static void draw_placing_ship (WINDOW *game_win, coordinates window, coordinates from, coordinates to);
    static void draw_aim (WINDOW *game_win, coordinates window, coordinates aim);
    static void player_fires(WINDOW* game_win, coordinates window, FIELD& field);
    static void print_ships_left (WINDOW* game_win, coordinates window, FIELD& field);
    static void menu_ship_animation (WINDOW *menu_win, std::atomic<bool>& running);
    bool continues() const;
    void bot_fires (WINDOW* game_win, std::deque <coordinates> & bot_future_moves);
    void reset_fields ();

    bool m_hints;
    int m_one_score, m_two_score;
    FIELD m_one, m_two;
    static std::mutex m_screen_mutex;

    static const std::vector<std::string> m_menu_options;
    static const std::vector<std::string> m_banner;
    static const std::vector<std::string> m_ship;
};


#endif //SEABATTLE_GAME_H
