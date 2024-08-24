#include "game.h"
#include <iostream>
#include <vector>
#include <string>
#include <unistd.h>
#include <ncurses.h>
#include <deque>
#include <algorithm>
#include <random>
#include <thread>
#include <atomic>
#include <mutex>


std::mutex GAME::m_screen_mutex;

const std::vector<std::string> GAME::m_menu_options = {
        "Single player", "Multi player", "Hints", "Reset score", "Quit"
};
const std::vector<std::string> GAME::m_banner = {
        "  ____    _____      _            ____       _      _____   _____   _       _____",
        " / ___|  | ____|    / \\          | __ )     / \\    |_   _| |_   _| | |     | ____|",
        " \\___ \\  |  _|     / _ \\         |  _ \\    / _ \\     | |     | |   | |     |  _|",
        "  ___) | | |___   / ___ \\        | |_) |  / ___ \\    | |     | |   | |___  | |___",
        " |____/  |_____| /_/   \\_\\       |____/  /_/   \\_\\   |_|     |_|   |_____| |_____|"
};
const std::vector<std::string> GAME::m_ship = {
        "         _    _",
        "      __|_|__|_|_",
        "    _|___________|__",
        "   |o o o o o o o o/",
        " ~'`~'`~'`~'`~'`~'`~"
};

    GAME::GAME()
            : m_one_score(0), m_two_score(0), m_hints(false) {}
    int GAME::manage_menu(WINDOW* menu_win) {
        //ship animation
        std::atomic<bool> running(true);
        std::thread animationThread(GAME::menu_ship_animation, std::ref(menu_win), std::ref(running));

        while (true) {
            size_t option = 0;
            size_t selection = -1;
            int c;

            draw_menu(menu_win, option);

            while (true) // arrow movement
            {
                c = wgetch(menu_win);
                switch (c) {
                    case KEY_UP:
                    case 'w':
                    case 'W':
                        if (option == 0)
                            option = 4;
                        else
                            option--;
                        break;
                    case KEY_DOWN:
                    case 's':
                    case 'S':
                        if (option == 4)
                            option = 0;
                        else
                            option++;
                        break;
                    case 10: // Enter
                        selection = option;
                        break;
                    default:
                        break;
                }
                draw_menu(menu_win, option);
                if (selection != -1) // player made a choice
                    break;
            }
            switch (selection) // chosen point in menu
            {
                case 0:
                    running = false;
                    animationThread.join();
                    return 1;
                case 1:
                    running = false;
                    animationThread.join();
                    return 2;
                case 2:
                    if (!m_hints) {
                        m_hints = true;
                        break;
                    }
                    m_hints = false;
                    break;
                case 3:
                    m_one_score = 0;
                    m_two_score = 0;
                    break;
                case 4:
                    running = false;
                    animationThread.join();
                    std::cout << "See you later, capitan!" << std::endl;
                    return 0;
                default:
                    continue;
            }
        }
    }
    void GAME::manage_singleplayer_ship_placement(WINDOW *game_win) {

        //place bots ships
        GAME::ships_auto_place (m_two);

        //place players ships
        GAME::place_player_ships (game_win, m_one);
    }
    void GAME::manage_singleplayer_fight (WINDOW *game_win)
    {
        GAME::draw_singleplayer_fight_interface(game_win, m_one);

        //create all possible fire options for bot
        std::deque <coordinates> bot_future_moves;
        for (int i = 0; i < 10; i++)
            for (int j = 0; j < 10; j++)
                bot_future_moves.emplace_back(i,j);
        //shuffle them
        std::random_device rd;
        std::mt19937 g(rd());
        std::shuffle(bot_future_moves.begin(), bot_future_moves.end(), g);

        for (int i = 0; ; i = (i+1)%2 )
        {
            i ? bot_fires(game_win, bot_future_moves) : player_fires (game_win, {BETWEEN_FIELDS * 2 + 3 + FIELD_WIDTH, 12}, m_two);
            if (!continues())
            {
                if (i)
                {
                    wattron(game_win, COLOR_PAIR(1));
                    mvwprintw(game_win, 5, (WINDOW_WIDTH - 41) / 2, "You lost :( Wish you more luck next time!");
                    wattroff(game_win, COLOR_PAIR(1));
                    draw_player_ships (game_win, coordinates (BETWEEN_FIELDS * 2 + 3 + FIELD_WIDTH, 12), m_two);
                    m_two_score++;
                }
                else
                {
                    wattron(game_win, COLOR_PAIR(3));
                    mvwprintw(game_win, 5, (WINDOW_WIDTH - 22) / 2, "You won! Great battle!");
                    wattroff(game_win, COLOR_PAIR(3));
                    m_one_score++;
                }

                mvwprintw(game_win, 7, (WINDOW_WIDTH - 29) / 2, "Press [SPACE] key to continue");

                int c = 0;
                while (c != ' ')
                    c = wgetch(game_win);

                delwin(game_win);
                refresh();
                reset_fields();
                break;
            }
        }
    }
    void GAME::manage_multiplayer_ship_placement (WINDOW *game_win)
    {
        GAME::place_player_ships (game_win, m_one, 3);

        //waiting screen
        wclear(game_win);
        wrefresh(game_win);

        box(game_win, 0, 0);
        wattron(game_win, COLOR_PAIR(3));
        mvwprintw(game_win, WINDOW_HEIGHT/2 - 1, (WINDOW_WIDTH - 23) / 2, "BLUE");
        wattroff(game_win, COLOR_PAIR(3));
        mvwprintw(game_win, WINDOW_HEIGHT/2 - 1, (WINDOW_WIDTH - 23) / 2 + 5, "player, turn away!");

        mvwprintw(game_win, WINDOW_HEIGHT/2 + 1, (WINDOW_WIDTH - 51) / 2, "Press [SPACE] to start placing ships for");
        wattron(game_win, COLOR_PAIR(1));
        mvwprintw(game_win, WINDOW_HEIGHT/2 + 1, (WINDOW_WIDTH - 51) / 2 + 41, "RED");
        wattroff(game_win, COLOR_PAIR(1));
        mvwprintw(game_win, WINDOW_HEIGHT/2 + 1, (WINDOW_WIDTH - 51) / 2 + 45, "player");

        int c = 0;
        while (c != ' ')
            c = wgetch(game_win);

        wclear(game_win);
        wrefresh(game_win);

        GAME::place_player_ships (game_win, m_two, 1);
    }
    void GAME::manage_multiplayer_fight (WINDOW *game_win)
    {
        GAME::draw_multiplayer_fight_interface(game_win);

        for (int i = 0; ; i = (i+1)%2 )
        {
            i ? player_fires(game_win, {BETWEEN_FIELDS + 3, 12}, m_one) : player_fires (game_win, {BETWEEN_FIELDS * 2 + 3 + FIELD_WIDTH, 12}, m_two);
            if (!continues())
            {
                if (i)
                {
                    wattron(game_win, COLOR_PAIR(1));
                    mvwprintw(game_win, 5, (WINDOW_WIDTH - 29) / 2, "RED player won! Great battle!");
                    wattroff(game_win, COLOR_PAIR(1));
                    draw_player_ships (game_win, coordinates (BETWEEN_FIELDS * 2 + 3 + FIELD_WIDTH, 12), m_two);
                    m_two_score++;
                }
                else
                {
                    wattron(game_win, COLOR_PAIR(3));
                    mvwprintw(game_win, 5, (WINDOW_WIDTH - 30) / 2, "BLUE player won! Great battle!");
                    wattroff(game_win, COLOR_PAIR(3));
                    draw_player_ships (game_win, coordinates (BETWEEN_FIELDS + 3, 12), m_one);
                    m_one_score++;
                }

                mvwprintw(game_win, 7, (WINDOW_WIDTH - 29) / 2, "Press [SPACE] key to continue");

                int c = 0;
                while (c != ' ')
                    c = wgetch(game_win);

                delwin(game_win);
                refresh();
                reset_fields();
                break;
            }
        }
    }
    void GAME::place_player_ships (WINDOW *game_win, FIELD& field, int color)
    {
        draw_placing_interface (game_win, color);
        int ship_size = 3; // actually represents the size of 4
        coordinates from (3, 4), to (from.x + ship_size, 4);
        draw_placing_ship(game_win, {BETWEEN_FIELDS + 3, 12}, from, to);
        print_ships_left (game_win, {BETWEEN_FIELDS * 2 + 1 + FIELD_WIDTH, 29}, field);
        wrefresh(game_win);

        int c = 0;
        while (c != 1) // arrow movement
        {
            c = wgetch(game_win);
            switch (c) {
                case KEY_DOWN:
                case 's':
                case 'S':
                    if (from.y == 9 || to.y == 9)
                        break;
                    from.y++;
                    to.y++;
                    break;
                case KEY_UP:
                case 'w':
                case 'W':
                    if (from.y == 0 || to.y == 0)
                        break;
                    from.y--;
                    to.y--;
                    break;
                case KEY_LEFT:
                case 'a':
                case 'A':
                    if (from.x == 0 || to.x == 0)
                        break;
                    from.x--;
                    to.x--;
                    break;
                case KEY_RIGHT:
                case 'd':
                case 'D':
                    if (from.x == 9 || to.x == 9)
                        break;
                    from.x++;
                    to.x++;
                    break;
                case ' ': // Space
                    if (from.y == to.y) {
                        to.x = from.x;
                        to.y += ship_size;

                        if (to.y > 9) {
                            from.y = 9 - ship_size;
                            to.y = 9;
                        }
                        break;
                    }
                    to.x += ship_size;
                    to.y = from.y;
                    if (to.x > 9) {
                        from.x = 9 - ship_size;
                        to.x = 9;
                    }
                    break;
                case 'Q':
                case 'q':
                    field.reset();
                    ship_size = 3;
                    from.x = 3, from.y = 4, to.x = from.x + ship_size, to.y = 4;
                    break;
                case 'E':
                case 'e':
                    if (!field.size_remains(0))
                    {
                        field.reset();
                        ship_size = 3;
                    }
                    GAME::ships_auto_place(field, ship_size);
                    ship_size = 0;
                    break;
                case 10: // Enter
                    field.add_ship(from, to);
                    if (!field.size_remains(ship_size))
                    {
                        ship_size--;
                        from.x == to.x ? to.y-- : to.x--;
                    }
                    if (!field.size_remains(0)) // end ships placement
                        c = 1;
                    break;
                default:
                    continue;
            }
            print_ships_left (game_win, coordinates (BETWEEN_FIELDS * 2 + 1 + FIELD_WIDTH, 29), field);
            draw_player_ships(game_win, {BETWEEN_FIELDS + 3, 12}, field);
            if (field.size_remains(0))
                draw_placing_ship(game_win, {BETWEEN_FIELDS + 3, 12}, from, to);
            wrefresh(game_win);
        }
    }
    void GAME::draw_singleplayer_fight_interface(WINDOW *game_win, FIELD& player) {
        werase(game_win);
        box(game_win, 0, 0);
        mvwprintw(game_win, 0, (WINDOW_WIDTH - 24) / 2, "**X  Battle ongoing  X**");
        mvwprintw(game_win, 10, BETWEEN_FIELDS + 3, "~-----------~    You    ~-----------~");
        mvwprintw(game_win, 10, BETWEEN_FIELDS * 2 + 3 + FIELD_WIDTH, "~----------~    Enemy    ~----------~");
        draw_grid(game_win, {BETWEEN_FIELDS + 1, 11}); //player
        draw_grid(game_win, {BETWEEN_FIELDS * 2 + 1 + FIELD_WIDTH, 11}); //opponent
        GAME::draw_player_ships(game_win, {BETWEEN_FIELDS + 3, 12}, player);
    }
    void GAME::draw_multiplayer_fight_interface(WINDOW *game_win) {
    werase(game_win);
    box(game_win, 0, 0);
    mvwprintw(game_win, 0, (WINDOW_WIDTH - 24) / 2, "**X  Battle ongoing  X**");
    mvwprintw(game_win, 10, BETWEEN_FIELDS + 3, "~-----------~           ~-----------~");
    wattron(game_win, COLOR_PAIR(3));
    mvwprintw(game_win, 10, BETWEEN_FIELDS + 3 + 16, "BLUE");
    wattroff(game_win, COLOR_PAIR(3));
    mvwprintw(game_win, 10, BETWEEN_FIELDS * 2 + 3 + FIELD_WIDTH, "~----------~           ~----------~");
    wattron(game_win, COLOR_PAIR(1));
    mvwprintw(game_win, 10, BETWEEN_FIELDS * 2 + 3 + FIELD_WIDTH + 16, "RED");
    wattroff(game_win, COLOR_PAIR(1));
    draw_grid(game_win, {BETWEEN_FIELDS + 1, 11});
    draw_grid(game_win, {BETWEEN_FIELDS * 2 + 1 + FIELD_WIDTH, 11});
}
    void GAME::draw_placing_interface(WINDOW *game_win, int color) {
        wclear(game_win);
        box(game_win, 0, 0);
        mvwprintw(game_win, 0, (WINDOW_WIDTH - 25) / 2, "**@  Placing ongoing  @**");
        mvwprintw(game_win, 10, BETWEEN_FIELDS + 3, "~-----------~           ~-----------~");
        if (color)
            wattron(game_win, COLOR_PAIR(color));
        mvwprintw(game_win, 10, BETWEEN_FIELDS + 3 + 18, "You");
        if (color)
            wattroff(game_win, COLOR_PAIR(color));
        draw_grid(game_win, {BETWEEN_FIELDS + 1, 11});
        mvwprintw(game_win, 6, 45, "Here is your field, chose carefully where to hide your ships");
        if (color)
            mvwprintw(game_win, 4, (WINDOW_WIDTH - 43 ) /2 , "Do not show the screen to the other player!");
        mvwprintw(game_win, 13, BETWEEN_FIELDS * 2 + 1 + FIELD_WIDTH, "[Enter] Place/Start");
        mvwprintw(game_win, 15, BETWEEN_FIELDS * 2 + 1 + FIELD_WIDTH, "[Space] Rotate");
        mvwprintw(game_win, 17, BETWEEN_FIELDS * 2 + 1 + FIELD_WIDTH, "[E] Auto-place");
        mvwprintw(game_win, 19, BETWEEN_FIELDS * 2 + 1 + FIELD_WIDTH, "[Q] Reset");
    }
    void GAME::ships_auto_place (FIELD& field, int ship_size)
    {
        while (field.size_remains(0))
        {
            std::random_device rd;
            std::mt19937 g(rd());
            std::uniform_int_distribution<> dist(0, 9);

            coordinates from (dist(g), dist(g));
            coordinates to = from;
            dist(g) % 2 ? to.x += ship_size : to.y += ship_size;
            if (to.x > 9 || to.y > 9)
                continue;
            field.add_ship(from, to);
            if (!field.size_remains(ship_size))
                ship_size--;
        }
    }
    void GAME::draw_menu(WINDOW *menu_win, size_t highlight) const
    {
        std::lock_guard<std::mutex> lock(m_screen_mutex);
        box(menu_win, 0, 0);

        //print the banner
        for (size_t i = 0; i < m_banner.size(); i++)
            mvwprintw(menu_win, i + 4, (WINDOW_WIDTH - 84) / 2, "%s", m_banner[i].c_str());

        int x = WINDOW_WIDTH / 5;
        int y = (WINDOW_HEIGHT - 11) / 2;
        for (int i = 0; i < m_menu_options.size(); i++, y+=2)
        {
            if (highlight == i)  //Turn on the highlight of current choice
            {
                wattron(menu_win, A_REVERSE);
            }

            highlight == i ? mvwprintw(menu_win, y, x - 2, ">") : mvwprintw(menu_win, y, x - 2, " ");
            mvwprintw(menu_win, y, x, "%s", m_menu_options[i].c_str());

            if (i == 2)
                m_hints ? mvwprintw(menu_win, y, x + 8 , "ON ") : mvwprintw(menu_win, y, x + 8, "OFF");

            if (highlight == i) //Turn off the highlight of current choice
                wattroff(menu_win, A_REVERSE);
        }

        y += 2;
        mvwprintw(menu_win, y, x, "SCORE");
        x += 7;
        wattron(menu_win, COLOR_PAIR(3));
        mvwprintw(menu_win, y, x, "%d", m_one_score);
        wattroff(menu_win, COLOR_PAIR(3));

        mvwprintw(menu_win, y, 2 + x, ":");

        wattron(menu_win, COLOR_PAIR(1));
        mvwprintw(menu_win, y, 4 + x, "%d", m_two_score);
        wattroff(menu_win, COLOR_PAIR(1));

        mvwprintw(menu_win, WINDOW_HEIGHT/2-2, WINDOW_WIDTH/4*3 - 48/2, "This game is made by @witsolvt (Maksym Humeniuk)");
        mvwprintw(menu_win, WINDOW_HEIGHT/2, WINDOW_WIDTH/4*3 - 14/2, "More on GitHub");
        mvwprintw(menu_win, WINDOW_HEIGHT/2+2, WINDOW_WIDTH/4*3 - 28/2, "Enjoy the fight, captain! ;)");

        wrefresh(menu_win);
    }
    void GAME::draw_grid(WINDOW *game_win, coordinates window) {
        // Draw horizontal lines with intersections
        for (int i = 0; i <= 10; i++)
        {
            int y = window.y + i * 2;
            for (int j = 0; j <= 10; j++)
            {
                int x = window.x + j * 4;
                if (i == 0) //  top line
                    mvwaddch(game_win, y, x, (j == 0) ? ACS_ULCORNER : (j == 10) ? ACS_URCORNER : ACS_TTEE);
                if (i == 10) // bottom line
                    mvwaddch(game_win, y, x, (j == 0) ? ACS_LLCORNER : (j == 10) ? ACS_LRCORNER : ACS_BTEE);
                if (i > 0 && i < 10) // in between
                    mvwaddch(game_win, y, x, (j == 0) ? ACS_LTEE : (j == 10) ? ACS_RTEE : ACS_PLUS);

                if (j < 10) {
                    mvwhline (game_win, y, x + 1, ACS_HLINE, 3);
                }
            }
        }

        // Draw vertical lines with intersections
        for (int j = 0; j <= 10; j++) {
            int x = window.x + j * 4;
            for (int i = 0; i <= 10; i++) {
                int y = window.y + i * 2;
                if (i < 10) {
                    mvwhline(game_win, y + 1, x, ACS_VLINE, 1);
                }
            }
        }
    }
    void GAME::draw_player_ships (WINDOW *game_win, coordinates window, const FIELD & field)
    {
        for (int i = 0; i < 10 ; i++)
        {
            for (int j = 0; j < 10 ; j++)
            {
                switch (field.cell_state({i, j}))
                {
                    case NOT_CHECKED:
                        mvwprintw(game_win, window.y+j*2, window.x+i*4, " ");
                        break;
                    case MISSED:
                        mvwprintw(game_win, window.y+j*2, window.x+i*4, ".");
                        break;
                    case HIT:
                        wattron(game_win, COLOR_PAIR(1));
                        mvwprintw(game_win, window.y+j*2, window.x+i*4, "X");
                        wattroff(game_win, COLOR_PAIR(1));
                        break;
                    case NOT_HIT:
                        mvwprintw(game_win, window.y+j*2, window.x+i*4, "#");
                        break;
                }
            }
        }
    }
    void GAME::draw_enemy_ships (WINDOW *game_win, coordinates window, const FIELD & field)
    {
        for (int i = 0; i < 10 ; i++)
        {
            for (int j = 0; j < 10 ; j++)
            {
                switch (field.cell_state({i, j}))
                {
                    case NOT_CHECKED:
                        mvwprintw(game_win, window.y+j*2, window.x+i*4, " ");
                        break;
                    case MISSED:
                        mvwprintw(game_win, window.y+j*2, window.x+i*4, ".");
                        break;
                    case HIT:
                        wattron(game_win, COLOR_PAIR(1));
                        mvwprintw(game_win, window.y+j*2, window.x+i*4, "X");
                        wattroff(game_win, COLOR_PAIR(1));
                        break;
                    case NOT_HIT:
                        mvwprintw(game_win, window.y+j*2, window.x+i*4, " ");
                        break;
                }
            }
        }
    }
    void GAME::draw_placing_ship (WINDOW *game_win, coordinates window, coordinates from, coordinates to)
    {
        wattron(game_win, COLOR_PAIR(2));
        for (int i = from.x; i <= to.x ; i++)
        {
            for (int j = from.y; j <= to.y ; j++)
            {
                mvwprintw(game_win, window.y+j*2, window.x+i*4, "@");
            }
        }
        wattroff(game_win, COLOR_PAIR(2));
    }
    void GAME::draw_aim (WINDOW *game_win, coordinates window, coordinates aim)
    {
        wattron(game_win, COLOR_PAIR(2));
        mvwprintw(game_win, window.y+aim.y*2, window.x+aim.x*4, "+");
        wattroff(game_win, COLOR_PAIR(2));
    }
    void GAME::player_fires(WINDOW* game_win, coordinates window, FIELD& field)
    {
        do {
            draw_enemy_ships(game_win, window, field);
            draw_aim(game_win, window, field.last_fire);
            wrefresh(game_win);
            int c;
            while (true) // arrow movement
            {
                c = wgetch(game_win);
                switch (c)
                {
                    case KEY_UP:
                    case 'w':
                    case 'W':
                        if (field.last_fire.y > 0)
                            field.last_fire.y--;
                        break;
                    case KEY_DOWN:
                    case 's':
                    case 'S':
                        if (field.last_fire.y < 9)
                            field.last_fire.y++;
                        break;
                    case KEY_LEFT:
                    case 'a':
                    case 'A':
                        if (field.last_fire.x > 0)
                            field.last_fire.x--;
                        break;
                    case KEY_RIGHT:
                    case 'd':
                    case 'D':
                        if (field.last_fire.x < 9)
                            field.last_fire.x++;
                        break;
                    case 10: // Enter
                        break;
                    default:
                        continue;
                }
                draw_enemy_ships(game_win, window, field);
                draw_aim(game_win, window, field.last_fire);
                wrefresh(game_win);
                if (c == 10)
                {
                    break;
                }
            }
        } while (field.fire(field.last_fire));
    }
    void GAME::print_ships_left (WINDOW* game_win, coordinates window, FIELD& field)
    {
        for (int i = 3; i >= 0; i--)
        {
            if (!field.size_remains(i))
            {
                mvwprintw(game_win, window.y-i*2, window.x, "            ");
                continue;
            }
            mvwprintw(game_win, window.y-i*2, window.x, "%d x ", field.size_remains(i));
            for (int j = 0; j <= i; j++)
                mvwprintw(game_win, window.y-i*2, window.x+4+2*j, "[]");
        }
    }
    void GAME::menu_ship_animation (WINDOW *menu_win, std::atomic<bool>& running)
    {
        int x = 1;
        while (running)
        {
            for (size_t i = 0; i < m_ship.size(); i++)
            {
                std::lock_guard<std::mutex> lock(m_screen_mutex);
                mvwprintw(menu_win, 30+i, x, "%s", m_ship[i].c_str());
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(500));
            x++;
            if (x == 129)
            {
                std::lock_guard<std::mutex> lock(m_screen_mutex);
                for (int j = 0; j < 5; j++)
                    mvwprintw(menu_win, 30+j, x, "                    ");
                x = 1;
            }
            wrefresh(menu_win);
        }
    }
    bool GAME::continues() const
    {
        return m_one.check_loss() && m_two.check_loss();
    }
    void GAME::bot_fires (WINDOW* game_win, std::deque <coordinates> & bot_future_moves)
    {
        while (m_one.fire (bot_future_moves.front()))
        {
            if (m_one.cell_state(bot_future_moves.front()) == HIT)
                std::this_thread::sleep_for(std::chrono::milliseconds(200));
            coordinates hit = bot_future_moves.front();
            bot_future_moves.pop_front();

            int check = m_one.cell_state(coordinates(hit.x, limit(hit.y - 1)));
            if(check == NOT_CHECKED || check ==  NOT_HIT)
                bot_future_moves.emplace_front(hit.x, hit.y-1);

            check = m_one.cell_state(coordinates(hit.x, limit(hit.y + 1)));
            if(check == NOT_CHECKED || check ==  NOT_HIT)
                bot_future_moves.emplace_front(hit.x, hit.y+1);

            check = m_one.cell_state(coordinates(limit(hit.x + 1), hit.y));
            if(check == NOT_CHECKED || check ==  NOT_HIT)
                bot_future_moves.emplace_front(hit.x+1, hit.y);

            check = m_one.cell_state(coordinates(limit(hit.x - 1), hit.y));
            if(check == NOT_CHECKED || check ==  NOT_HIT)
                bot_future_moves.emplace_front(hit.x-1, hit.y);

            draw_player_ships(game_win, {BETWEEN_FIELDS + 3, 12}, m_one);
            wrefresh(game_win);
        }
        bot_future_moves.pop_front();
        draw_player_ships(game_win, {BETWEEN_FIELDS + 3, 12}, m_one);
        wrefresh(game_win);
    }
    void GAME::reset_fields ()
    {
        m_one.reset();
        m_two.reset();
    }