#include <unistd.h>
#include <ncurses.h>
#include <algorithm>
#include <mutex>
#include "game.h"


int main() {
    GAME game;

    initscr();
    start_color();
    init_pair(1, COLOR_RED, COLOR_BLACK);
    init_pair(2, COLOR_YELLOW, COLOR_BLACK);
    init_pair(3, COLOR_BLUE, COLOR_BLACK);
    cbreak();
    noecho();
    keypad(stdscr, TRUE);

    while (true)
    {
        //window for menu
        int start_x = (COLS - WINDOW_WIDTH) / 2;
        int start_y = (LINES - WINDOW_HEIGHT) / 2;
        WINDOW *menu_win = newwin(WINDOW_HEIGHT, WINDOW_WIDTH, start_y, start_x);
        wattron(menu_win, A_BOLD);
        keypad(menu_win, TRUE);

        int start_game = game.manage_menu(menu_win);

        delwin(menu_win);
        refresh();

        if(!start_game)
        {
            endwin();
            return 0;
        }
        //window for game
        start_x = (COLS - WINDOW_WIDTH) / 2;
        start_y = (LINES - WINDOW_HEIGHT) / 2;
        WINDOW* game_win = newwin(WINDOW_HEIGHT, WINDOW_WIDTH, start_y, start_x);
        keypad(game_win, TRUE);
        wattron(game_win, A_BOLD);

        //sets ships for both player and bot
        game.manage_ship_placement(game_win);

        //cycle in witch player and bot fire each other until someone wins
        game.manage_ongoing_fight (game_win);
    }
}
