#include <iostream>
#include <vector>
#include <string>
#include <unistd.h>
#include <ncurses.h>
#include <cstdlib>
#include <ctime>

#define min(a, b) (( a < b ) ? a : b)
#define max(a, b) (( a < b ) ? b : a)
#define MENU_HEIGHT 12
#define MENU_WIDTH 40
#define GAME_HEIGHT 40
#define GAME_WIDTH 150
#define FIELD_WIDTH 41
#define BETWEEN_FIELDS ((GAME_WIDTH - FIELD_WIDTH * 2) / 3)

int limit (const int a)
{
    if (a < 0)
        return 0;
    if (a > 9)
        return 9;
    return a;
}
enum cell_state
{
    NOT_CHECKED,
    MISSED,
    HIT,
    NOT_HIT
};
const std::vector<std::string> menu_options = {
        "**X Welcome to sea battle! X**", "Start", "Hints", "Reset score", "Quit"
};
struct coordinates {
    coordinates (int x_, int y_)
    : x(x_), y(y_) {}
    int x, y;
};


class FIELD {
public:
    FIELD ()
    : alive (20), last_fire(0, 0)
    {
        for (auto & i : map)
            for (int & j : i)
                j = NOT_CHECKED;
        for (int i = 0; i < 4 ; i++)
            ships_not_placed[i] = 4 - i;
    }
    bool size_remains (int size)
    {
        if (ships_not_placed[size])
            return true;
        return false;
    }
    int cell_state(coordinates check) const
    {
        return map[check.y][check.x];
    }
    bool add_ship (coordinates from, coordinates to)
    {
        //check if there is ship too close to a new one
        for (int i = limit(min (from.x, to.x) - 1); i <= limit(max (from.x, to.x) + 1); i++)
            for (int j = limit(min (from.y, to.y) - 1); j <= limit(max (from.y, to.y) + 1); j++)
                if (map[j][i] == NOT_HIT)
                    return false;

        //add ship to the map
        for (int i = min (from.x, to.x); i <= max (from.x, to.x); i++)
            for (int j = min (from.y, to.y); j <= max (from.y, to.y); j++)
                map[j][i] = NOT_HIT;

        int size = max (max(from.x, to.x) - min(from.x, to.x), max(from.y, to.y) - min(from.y, to.y));
        ships_not_placed[size]--;
        return true;
    }
    bool check_loss () const
    {
        return alive;
    }
    bool fire (coordinates fire) // false - done, true - continue fire
    {
        if (map[fire.y][fire.x] == NOT_CHECKED)
        {
            map[fire.y][fire.x] = MISSED;
            return false;
        }
        if (map[fire.y][fire.x] == NOT_HIT)
        {
            alive--;
            map[fire.y][fire.x] = HIT;
            //check if ship is dead
            if (!is_ship_alive (fire, fire))
                update_surrounding (fire, fire);
            return true;
        }
        if (map[fire.y][fire.x] == HIT  || map[fire.y][fire.x] == MISSED)
        {
            return true;
        }
        return false;
    }
    void reset ()
    {
        for (auto & i : map)
            for (int & j : i)
                j = NOT_CHECKED;
        alive = 20;
        last_fire.x = 0;
        last_fire.y = 0;
        for (int i = 0; i < 4 ; i++)
            ships_not_placed[i] = 4 - i;
    }
    coordinates last_fire;
private:
    bool is_ship_alive (coordinates check, coordinates ignore)
    {
        for (int i = limit (check.x - 1); i <= limit (check.x+1); i++)
        {
            for (int j = limit (check.y - 1); j <= limit (check.y+1); j++)
            {
                if ((j == ignore.y && i == ignore.x) || (j == check.y && i == check.x))
                    continue;
                switch (map[j][i])
                {
                    case NOT_HIT:
                        return true;
                    case HIT:
                        if (is_ship_alive ({i, j}, check))
                            return true;
                    default:
                        continue;
                }
            }
        }
           return false;
    }
    void update_surrounding (coordinates update, coordinates ignore)
    {
        for (int i = limit (update.x - 1); i <= limit (update.x+1); i++)
        {
            for (int j = limit (update.y - 1); j <= limit (update.y+1); j++)
            {
                if ((j == ignore.y && i == ignore.x) || (j == update.y && i == update.x))
                    continue;
                switch (map[j][i])
                {
                    case MISSED:
                        continue;
                    case NOT_CHECKED:
                        map[j][i] = MISSED;
                        break;
                    case HIT:
                        update_surrounding ({i, j}, update);
                    default:
                        continue;
                }
            }
        }
    }
    int ships_not_placed [4]; // index 0 represents 1-decker ship, 1 represents 2-decker, etc.
    int alive;
    int map [10][10];
};

class GAME {
public:
    GAME()
            : one_score(0), two_score(0), hints(false) {}
    bool manage_menu() // true - start game, false - quit
    {
        while (true) {
            size_t option = 2;
            size_t selection = 1;
            int c;

            // draw menu
            int start_x = (COLS - MENU_WIDTH) / 2;
            int start_y = (LINES - MENU_HEIGHT) / 2;
            WINDOW *menu_win = newwin(MENU_HEIGHT, MENU_WIDTH, start_y, start_x);
            keypad(menu_win, TRUE);
            print_menu(menu_win, option, menu_options, hints, one_score, two_score);

            while (true) // arrow movement
            {
                c = wgetch(menu_win);
                switch (c) {
                    case KEY_UP:
                    case 'w':
                    case 'W':
                        if (option == 2)
                            option = menu_options.size();
                        else
                            option--;
                        break;
                    case KEY_DOWN:
                    case 's':
                    case 'S':
                        if (option == menu_options.size())
                            option = 2;
                        else
                            option++;
                        break;
                    case 10: // Enter
                        selection = option;
                        break;
                    default:
                        break;
                }
                print_menu(menu_win, option, menu_options, hints, one_score, two_score);
                if (selection != 1) // player made a choice
                    break;
            }
            switch (selection) // chosen point in menu
            {
                case 2:
                    werase(menu_win);
                    wrefresh(menu_win);
                    delwin(menu_win);
                    clear();
                    return true;
                case 3:
                    if (!hints) {
                        hints = true;
                        break;
                    }
                    hints = false;
                    break;
                case 4:
                    one_score = 0;
                    two_score = 0;
                    break;
                case 5:
                    delwin(menu_win);
                    endwin();
                    std::cout << "See you later, capitan!" << std::endl;
                    return false;
                default:
                    continue;
            }
        }
    }
    static void draw_fight_interface(WINDOW *game_win, FIELD& player) {
        werase(game_win);
        wrefresh(game_win);
        box(game_win, 0, 0);
        mvwprintw(game_win, 10, BETWEEN_FIELDS + 3, "~-----------~    You    ~-----------~");
        mvwprintw(game_win, 10, BETWEEN_FIELDS * 2 + 3 + FIELD_WIDTH, "~----------~    Enemy    ~----------~");
        draw_grid(game_win, {BETWEEN_FIELDS + 1, 11}); //player
        draw_grid(game_win, {BETWEEN_FIELDS * 2 + 1 + FIELD_WIDTH, 11}); //opponent
        GAME::draw_player_ships(game_win, {BETWEEN_FIELDS + 3, 12}, player);
    }
    static void draw_placing_interface(WINDOW *game_win) {
        werase(game_win);
        wrefresh(game_win);
        box(game_win, 0, 0);
        mvwprintw(game_win, 10, BETWEEN_FIELDS + 3, "~-----------~    You    ~-----------~");
        draw_grid(game_win, {BETWEEN_FIELDS + 1, 11});
        mvwprintw(game_win, 3, 45, "Here is your field, chose carefully where to hide your fleet");
        mvwprintw(game_win, 19, BETWEEN_FIELDS * 2 + 1 + FIELD_WIDTH, "[Q] Reset");
        mvwprintw(game_win, 21, BETWEEN_FIELDS * 2 + 1 + FIELD_WIDTH, "[E] Auto-place");
    }
    void manage_ship_placement(WINDOW *game_win, FIELD &field) {
        draw_placing_interface (game_win);
        int ship_size = 3; // actually represents the size of 4
        coordinates from (3, 4), to (from.x + ship_size, 4);
        draw_placing_ship(game_win, {BETWEEN_FIELDS + 3, 12}, from, to);
        wrefresh(game_win);

        int c = 0;
        while (c != 10 || one.size_remains(0)) // arrow movement
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
                    if (!one.size_remains(0))
                    {
                        field.reset();
                    }
                    GAME::ships_auto_place(field);
                    ship_size = 0;
                    break;
                case 10: // Enter
                    field.add_ship(from, to);
                    if (!one.size_remains(ship_size))
                    {
                        ship_size--;
                        from.x == to.x ? to.y-- : to.x--;
                    }
                    break;
                default:
                    continue;
            }
            draw_player_ships(game_win, {BETWEEN_FIELDS + 3, 12}, one);
            draw_placing_ship(game_win, {BETWEEN_FIELDS + 3, 12}, from, to);
            wrefresh(game_win);
        }
    }
    bool continues() const
    {
        return one.check_loss() && two.check_loss();
    }
    void player_fires(WINDOW* game_win, coordinates window, FIELD& field)
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
    void bot_fires (WINDOW* game_win)
    {
        while (one.fire ( one.last_fire))
        {
            if (one.cell_state(one.last_fire) == HIT)
                usleep(500000);
            one.last_fire.x = rand() % 10;
            one.last_fire.y = rand() % 10;
            draw_player_ships(game_win, {BETWEEN_FIELDS + 3, 12}, one);
            wrefresh(game_win);
        }
        draw_player_ships(game_win, {BETWEEN_FIELDS + 3, 12}, one);
        wrefresh(game_win);
    }
    void increase_one_score ()
    {
        one_score++;
    }
    void increase_two_score ()
    {
    two_score++;
    }
    void reset_fields ()
    {
    one.reset();
    two.reset();
    }
    static void ships_auto_place (FIELD& field)
    {
        int ship_size = 3;
        srand (time(nullptr));

        while (field.size_remains(0))
        {
            coordinates from (rand() % 10, rand() % 10);
            coordinates to = from;
            rand() % 2 ? to.x += ship_size : to.y += ship_size;
            if (to.x > 9 || to.y > 9)
                continue;
            field.add_ship(from, to);
            if (!field.size_remains(ship_size))
                ship_size--;
        }
    }
    FIELD one, two;
private:
    static void print_menu(WINDOW *menu_win, size_t highlight, const std::vector<std::string> &choices, bool hints, const int one_score, const int two_score)
    {
        erase();
        refresh();
        box(menu_win, 0, 0);
        int x = (MENU_WIDTH - 30) / 2 + 1, y = 0;
        mvwprintw(menu_win, y, x, "%s", choices[0].c_str());
        x = 5;
        y = 3;
        for (int i = 1; i < choices.size(); ++i) {
            if (highlight == i + 1) //is selected
            {
                wattron(menu_win, A_REVERSE); // Highlight the current choice
                mvwprintw(menu_win, y, x, "%s", choices[i].c_str());
                if (i == 2)
                    hints ? mvwprintw(menu_win, y, MENU_WIDTH * 3 / 4 , "ON") : mvwprintw(menu_win, y, MENU_WIDTH * 3 / 4, "OFF");
                wattroff(menu_win, A_REVERSE);
            }
            else // not selected
            {
                mvwprintw(menu_win, y, x, "%s", choices[i].c_str());
                if (i == 2)
                    hints ? mvwprintw(menu_win, y, MENU_WIDTH * 3 / 4 , "ON") : mvwprintw(menu_win, y, MENU_WIDTH * 3 / 4, "OFF");
            }
            y++;
        }
        mvwprintw(menu_win, 8, (MENU_WIDTH - 5) / 2, "SCORE");
        mvwprintw(menu_win, 9, (MENU_WIDTH - 5) / 2, "%d : %d", one_score, two_score);
        wrefresh(menu_win);
    }
    static void draw_grid(WINDOW *game_win, coordinates window) {
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
    static void draw_player_ships (WINDOW *game_win, coordinates window, const FIELD & field)
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
                        mvwprintw(game_win, window.y+j*2, window.x+i*4, "X");
                        break;
                    case NOT_HIT:
                        mvwprintw(game_win, window.y+j*2, window.x+i*4, "#");
                        break;
                }
            }
        }
    }
    static void draw_enemy_ships (WINDOW *game_win, coordinates window, const FIELD & field)
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
                        mvwprintw(game_win, window.y+j*2, window.x+i*4, "X");
                        break;
                    case NOT_HIT:
                        mvwprintw(game_win, window.y+j*2, window.x+i*4, " ");
                        break;
                }
            }
        }
    }
    static void draw_placing_ship (WINDOW *game_win, coordinates window, coordinates from, coordinates to)
    {
        for (int i = from.x; i <= to.x ; i++)
        {
            for (int j = from.y; j <= to.y ; j++)
            {
                mvwprintw(game_win, window.y+j*2, window.x+i*4, "@");
            }
        }
    }
    static void draw_aim (WINDOW *game_win, coordinates window, coordinates aim)
    {
        mvwprintw(game_win, window.y+aim.y*2, window.x+aim.x*4, "+");
    }
    bool hints;
    int one_score, two_score;
};



int main() {
    GAME game;
    while (true)
    {
        initscr();
        cbreak();
        noecho();
        keypad(stdscr, TRUE);

        if (!game.manage_menu())
            return 0;

        //create a new window for game
        int start_x = (COLS - GAME_WIDTH) / 2;
        int start_y = (LINES - GAME_HEIGHT) / 2;
        WINDOW* game_win = newwin(GAME_HEIGHT, GAME_WIDTH, start_y, start_x);
        keypad(game_win, TRUE);

        game.manage_ship_placement(game_win, game.one);
        GAME::ships_auto_place (game.two);
        GAME::draw_fight_interface (game_win, game.one);
        for (int i = 0; ; i = (i+1)%2 )
        {
            i ? game.bot_fires(game_win) : game.player_fires (game_win, {BETWEEN_FIELDS * 2 + 3 + FIELD_WIDTH, 12}, game.two);
            if (!game.continues())
            {
                i ? mvwprintw(game_win, 5, (GAME_WIDTH - 41) / 2, "You lost :( Wish you more luck next time!") : mvwprintw(game_win, 5, (GAME_WIDTH - 22) / 2, "You won! Great battle!");
                i ? game.increase_two_score() : game.increase_one_score();

                mvwprintw(game_win, 6, (GAME_WIDTH - 23) / 2, "Press any key to continue");
                wgetch(game_win);
                delwin(game_win);
                refresh();
                game.reset_fields();
                break;
            }
        }
    }
}