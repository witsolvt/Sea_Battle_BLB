#include <iostream>
#include <vector>
#include <string>
#include <unistd.h>
#include <ncurses.h>

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


class FIELD {
public:
    FIELD ()
    {
        for (auto & i : map)
            for (int & j : i)
                j = NOT_CHECKED;
        alive = 20;
        for (int i = 0; i < 4 ; i++)
            ships_left[i] = 4-i;
    }

    bool size_remains (int size)
    {
        if (ships_left[size])
            return true;
        return false;
    }
    int cell_state(int x, int y) const
    {
        return map[y][x];
    }
    bool add_ship (const int one_x, const int one_y, const int two_x, const int two_y)
    {
        //check if there is ship too close to a new one
        for (int i = limit(min (one_x, two_x) - 1); i <= limit(max (one_x, two_x) + 1); i++)
            for (int j = limit(min (one_y, two_y) - 1); j <= limit(max (one_y, two_y) + 1); j++)
                if (map[j][i] == NOT_HIT)
                    return false;

        //add ship to the map
        for (int i = min (one_x, two_x); i <= max (one_x, two_x); i++)
            for (int j = min (one_y, two_y); j <= max (one_y, two_y); j++)
                map[j][i] = NOT_HIT;

        int size = max (max(one_x, two_x) - min(one_x, two_x), max(one_y, two_y) - min(one_y, two_y));
        ships_left[size]--;
        return true;
    }
    bool check_loss () const
    {
        return alive;
    }
    bool fire (const int x, const int y) // false - done, true - continue fire
    {
        if (map[y][x] == NOT_CHECKED)
        {
            map[y][x] = MISSED;
            return false;
        }
        if (map[y][x] == NOT_HIT)
        {
            alive--;
            map[y][x] = HIT;
            return true;
        }
        if (map[y][x] == HIT  || map[y][x] == MISSED)
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
        for (int i = 0; i < 4 ; i++)
            ships_left[i] = 4-i;
    }
private:
    int ships_left [4]; // index 0 represents 1-decker ship, 1 represents 2-decker, etc.
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
            int option = 2;
            int selection = 1;
            int c;

            // draw menu
            int startx = (COLS - MENU_WIDTH) / 2;
            int starty = (LINES - MENU_HEIGHT) / 2;
            WINDOW *menu_win = newwin(MENU_HEIGHT, MENU_WIDTH, starty, startx);
            keypad(menu_win, TRUE);
            box(menu_win, 0, 0);
            print_menu(menu_win, option, menu_options, hints, one_score, two_score);

            while (true) // arrow movement
            {
                c = wgetch(menu_win);
                switch (c) {
                    case KEY_UP:
                        if (option == 2)
                            option = menu_options.size();
                        else
                            option--;
                        break;
                    case KEY_DOWN:
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
            }
        }
    }

    void draw_interface(WINDOW *game_win) {
        box(game_win, 0, 0);
        mvwprintw(game_win, 10, BETWEEN_FIELDS + 3, "~-----------~    You    ~-----------~");
        mvwprintw(game_win, 10, BETWEEN_FIELDS * 2 + 3 + FIELD_WIDTH, "~----------~    Enemy    ~----------~");
        draw_grid(game_win, 11, BETWEEN_FIELDS + 1); //player
        draw_grid(game_win, 11, BETWEEN_FIELDS * 2 + 1 + FIELD_WIDTH); //opponent
    }

    void manage_ship_placement(WINDOW *game_win, FIELD &field) {
        mvwprintw(game_win, 3, 45, "Here is your field, chose carefully where to hide your fleet");
        //draw existing ships
        draw_player_ships(game_win, 12, BETWEEN_FIELDS + 3, one);
        int ship_size = 3; // actually represents the size of 4
        int from_x = 3, from_y = 4, to_x = from_x + ship_size, to_y = 4;
        draw_placing_ship(game_win, 12, BETWEEN_FIELDS + 3, from_x, from_y, to_x, to_y);
        wrefresh(game_win);

        int c;
        while (ship_size != -1) // arrow movement
        {

            c = wgetch(game_win);
            switch (c) {
                case KEY_DOWN:
                    if (from_y == 9 || to_y == 9)
                        break;
                    from_y++;
                    to_y++;
                    break;
                case KEY_UP:
                    if (from_y == 0 || to_y == 0)
                        break;
                    from_y--;
                    to_y--;
                    break;
                case KEY_LEFT:
                    if (from_x == 0 || to_x == 0)
                        break;
                    from_x--;
                    to_x--;
                    break;
                case KEY_RIGHT:
                    if (from_x == 9 || to_x == 9)
                        break;
                    from_x++;
                    to_x++;
                    break;
                case ' ': // Space
                    if (from_y == to_y) {
                        to_x = from_x;
                        to_y += ship_size;

                        if (to_y > 9) {
                            from_y = 9 - ship_size;
                            to_y = 9;
                        }
                        break;
                    }
                    to_x += ship_size;
                    to_y = from_y;
                    if (to_x > 9) {
                        from_x = 9 - ship_size;
                        to_x = 9;
                    }
                    break;
                case 10: // Enter
                    field.add_ship(from_x, from_y, to_x, to_y);
                    if (!one.size_remains(ship_size)) {
                        ship_size--;
                        from_x == to_x ? to_y-- : to_x--;
                    }
                    break;
                default:
                    continue;
            }
            draw_player_ships(game_win, 12, BETWEEN_FIELDS + 3, one);
            draw_placing_ship(game_win, 12, BETWEEN_FIELDS + 3, from_x, from_y, to_x, to_y);
            wrefresh(game_win);
        }
    }

    bool continues() const {
        return one.check_loss() && two.check_loss();
    }

    void player_fires(WINDOW* game_win, int start_x, int start_y, FIELD& field, int* x, int* y)
    {
        do {
            draw_enemy_ships(game_win, start_y, start_x, field);
            draw_aim(game_win, start_y, start_x, *x, *y);
            wrefresh(game_win);
            int c = 0;
            while (true) // arrow movement
            {
                c = wgetch(game_win);
                switch (c)
                {
                    case KEY_UP:
                        if (*y > 0)
                            (*y)--;
                        break;
                    case KEY_DOWN:
                        if (*y < 9)
                            (*y)++;
                        break;
                    case KEY_LEFT:
                        if (*x > 0)
                            (*x)--;
                        break;
                    case KEY_RIGHT:
                        if (*x < 9)
                            (*x)++;
                        break;
                    case 10: // Enter
                        break;
                    default:
                        continue;
                }
                draw_enemy_ships(game_win, start_y, start_x, field);
                draw_aim(game_win, start_y, start_x, *x, *y);
                wrefresh(game_win);
                if (c == 10)
                {
                    break;
                }
            }
        } while (field.fire(*x, *y));
    }
    void bot_fires (WINDOW* game_win, int x, int y)
    {
        one.fire (x, y);
        draw_player_ships(game_win, 12, BETWEEN_FIELDS + 3, one);
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
    FIELD one, two;
private:
    static void print_menu(WINDOW *menu_win, int highlight, const std::vector<std::string> &choices, bool hints, const int one_score, const int two_score)
    {

        int x = (MENU_WIDTH - sizeof (choices[0])) / 2 + 1, y = 0;
        mvwprintw(menu_win, y, x, "%s", choices[0].c_str());
        x = 5;
        y = 3;
        for (int i = 1; i < choices.size(); ++i) {
            if (highlight == i + 1) //is selected
            {
                wattron(menu_win, A_REVERSE); // Highlight the current choice
                mvwprintw(menu_win, y, x, "%s", choices[i].c_str());
                if (i == 2)
                {
                    if (hints)
                        mvwprintw(menu_win, y, MENU_WIDTH * 3 / 4 , "ON");
                    if (!hints)
                        mvwprintw(menu_win, y, MENU_WIDTH * 3 / 4, "OFF");
                }
                wattroff(menu_win, A_REVERSE);
            }
            else // not selected
            {
                mvwprintw(menu_win, y, x, "%s", choices[i].c_str());
                if (i == 2)
                {
                    if (hints)
                        mvwprintw(menu_win, y, MENU_WIDTH * 3 / 4, "ON");
                    else
                        mvwprintw(menu_win, y, MENU_WIDTH * 3 / 4, "OFF");
                }
            }
            y++;
        }
        mvwprintw(menu_win, 8, (MENU_WIDTH - 5) / 2, "SCORE");
        mvwprintw(menu_win, 9, (MENU_WIDTH - 5) / 2, "%d : %d", one_score, two_score);
        wrefresh(menu_win);
    }
    static void draw_grid(WINDOW *game_win, int start_y, int start_x) {
        // Draw horizontal lines with intersections
        for (int i = 0; i <= 10; i++)
        {
            int y = start_y + i * 2;
            for (int j = 0; j <= 10; j++)
            {
                int x = start_x + j * 4;
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
            int x = start_x + j * 4;
            for (int i = 0; i <= 10; i++) {
                int y = start_y + i * 2;
                if (i < 10) {
                    mvwhline(game_win, y + 1, x, ACS_VLINE, 1);
                }
            }
        }
    }
    static void draw_player_ships (WINDOW *game_win, int start_y, int start_x, const FIELD & field)
    {
        for (int i = 0; i < 10 ; i++)
        {
            for (int j = 0; j < 10 ; j++)
            {
                switch (field.cell_state(i, j))
                {
                    case NOT_CHECKED:
                        mvwprintw(game_win, start_y+j*2, start_x+i*4, " ");
                        break;
                    case MISSED:
                        mvwprintw(game_win, start_y+j*2, start_x+i*4, "*");
                        break;
                    case HIT:
                        mvwprintw(game_win, start_y+j*2, start_x+i*4, "X");
                        break;
                    case NOT_HIT:
                        mvwprintw(game_win, start_y+j*2, start_x+i*4, "#");
                        break;
                }
            }
        }
    }
    static void draw_enemy_ships (WINDOW *game_win, int start_y, int start_x, const FIELD & field)
    {
        for (int i = 0; i < 10 ; i++)
        {
            for (int j = 0; j < 10 ; j++)
            {
                switch (field.cell_state(i, j))
                {
                    case NOT_CHECKED:
                        mvwprintw(game_win, start_y+j*2, start_x+i*4, " ");
                        break;
                    case MISSED:
                        mvwprintw(game_win, start_y+j*2, start_x+i*4, "*");
                        break;
                    case HIT:
                        mvwprintw(game_win, start_y+j*2, start_x+i*4, "X");
                        break;
                    case NOT_HIT:
                        mvwprintw(game_win, start_y+j*2, start_x+i*4, " ");
                        break;
                }
            }
        }
    }
    static void draw_placing_ship (WINDOW *game_win, int start_y, int start_x, int from_x, int from_y, int to_x, int to_y)
    {
        for (int i = from_x; i <= to_x ; i++)
        {
            for (int j = from_y; j <= to_y ; j++)
            {
                mvwprintw(game_win, start_y+j*2, start_x+i*4, "@");
            }
        }
    }
    static void draw_aim (WINDOW *game_win, int start_y, int start_x, int x, int y)
    {
        mvwprintw(game_win, start_y+y*2, start_x+x*4, "+");
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
        int startx = (COLS - GAME_WIDTH) / 2;
        int starty = (LINES - GAME_HEIGHT) / 2;
        WINDOW* game_win = newwin(GAME_HEIGHT, GAME_WIDTH, starty, startx);
        keypad(game_win, TRUE);

        game.draw_interface (game_win);
        game.manage_ship_placement(game_win, game.one);

        int last_fire_x = 0, last_fire_y = 0;
        int a = 0, b = 0;
        while (true)
        {
            game.player_fires (game_win, BETWEEN_FIELDS * 2 + 3 + FIELD_WIDTH, 12, game.two, &last_fire_x, &last_fire_y);
            if (!game.continues())
            {
                game.increase_one_score();
                mvwprintw(game_win, 5, (GAME_WIDTH - 22) / 2, "You won! Great battle!");
                mvwprintw(game_win, 6, (GAME_WIDTH - 23) / 2, "Press any key to continue");
                wgetch(game_win);
                delwin(game_win);
                refresh();
                game.reset_fields();
                break;
            }
            game.bot_fires(game_win, a, b);
            wrefresh(game_win);
            if (!game.continues())
            {
                game.increase_two_score();
                mvwprintw(game_win, 5, (GAME_WIDTH - 41) / 2, "You lost :( Wish you more luck next time!");
                mvwprintw(game_win, 6, (GAME_WIDTH - 23) / 2, "Press any key to continue");
                wgetch(game_win);
                delwin(game_win);
                refresh();
                game.reset_fields();
                break;
            }
            a++;
            if (a > 9)
            {
                a = 0;
                b++;
            }
        }
    }
}