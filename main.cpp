#include <cstdlib>
#include <iostream>
#include <iomanip>
#include <vector>
#include <set>
#include <list>
#include <map>
#include <array>
#include <deque>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <compare>
#include <algorithm>
#include <cassert>
#include <memory>
#include <iterator>
#include <functional>
#include <stdexcept>
#include <unistd.h>
#include <ncurses.h>

#define min(a, b) (( a < b ) ? a : b)
#define max(a, b) (( a < b ) ? b : a)
#define HEIGHT 12
#define WIGHT 40
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
    void draw_players_field()
    {
        std::cout << "    A B C D E F G H I J" << std::endl;
        for (int i = 0; i < 10; i++)
        {
            draw_player_field_line (i);
            std::cout << std::endl;
        }
    }
    void draw_opponents_field ()
    {
        std::cout << "    A B C D E F G H I J" << std::endl;
        for (int i = 0; i < 10; i++)
        {
            draw_opponents_field_line (i);
            std::cout << std::endl;
        }
    }
    void draw ()
    {
        std::cout << "    A B C D E F G H I J\t\t\t\t    A B C D E F G H I J" << std::endl;
        for (int i = 0; i < 10; i++)
        {
            draw_player_field_line (i);
            std::cout << "\t\t\t";
            draw_opponents_field_line (i);
            std::cout << std::endl;
        }

    }
    bool add_ship (const int one_x, const int one_y, const int two_x, const int two_y)
    {
        //check if coordinates are correct
        if (one_x != two_x && one_y != two_y)
            return false;
        if (one_x < 0 || one_x > 9 || two_x < 0 || two_x > 9 || one_y < 0 || one_y > 9 || two_y < 0 || two_y > 9)
            return false;
        if (max(one_x, two_x) - min(one_x, two_x) > 3 || max(one_y, two_y) - min(one_y, two_y) > 3)
            return false;

        //check if there is ship too close to a new one
        for (int i = limit(min (one_x, two_x) - 1); i <= limit(max (one_x, two_x) + 1); i++)
            for (int j = limit(min (one_y, two_y) - 1); j <= limit(max (one_y, two_y) + 1); j++)
                if (map[j][i] == NOT_HIT)
                    return false;

        //check if there are enough ships of this size
        int size = max (max(one_x, two_x) - min(one_x, two_x), max(one_y, two_y) - min(one_y, two_y));
        if (!ships_left[size])
            return false;

        //add ship to the map
        for (int i = min (one_x, two_x); i <= max (one_x, two_x); i++)
            for (int j = min (one_y, two_y); j <= max (one_y, two_y); j++)
                map[j][i] = NOT_HIT;
        ships_left[size]--;
        return true;
    }
    bool check_loss () const
    {
        return alive;
    }
    bool fire (const int x, const int y)
    {
        if (map[y][x] == HIT || map[y][x] == MISSED)
        {
            std::cout << "\033[H\033[J";
            std::cout << "You have already tried this one! Chose other place to fire" << std::endl;
            return true;
        }
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
        return false;
    }
    void print_amount_left ()
    {
        std::cout << "\nAmount of ships left:" << std::endl;
        if (ships_left[3])
            std::cout << "Four-decker - " << ships_left[3] << "\n";
        if (ships_left[2])
            std::cout << "Three-decker - " << ships_left[2] << '\n';
        if (ships_left[1])
            std::cout << "Two-decker - " << ships_left[1] << '\n';
        if (ships_left[0])
            std::cout << "One-decker - " << ships_left[0] << std::endl;
    }
    bool placing_ongoing () {
        int left = 0;
        for (int i : ships_left)
            left += i;
        return left-9;
    }
private:
    void draw_player_field_line (int i)
    {
        if (i < 9)
            std::cout << ' ';
        std::cout << i+1;
        std::cout << " |";
        for (int j = 0; j < 10; j++)
        {
            switch (map[i][j])
            {
                case NOT_CHECKED:
                {
                    std::cout << ' ';
                    break;
                }
                case MISSED :
                {
                    std::cout << '*';
                    break;
                }
                case HIT:
                {
                    std::cout << 'X';
                    break;
                }
                case NOT_HIT :
                {
                    std::cout << '0';
                    break;
                }
            }
            std::cout << '|';
        }
    }
    void draw_opponents_field_line (int i)
    {
        if (i < 9)
            std::cout << ' ';
        std::cout << i+1;
        std::cout << " |";
        for (int j = 0; j < 10; j++)
        {
            switch (map[i][j])
            {
                case NOT_CHECKED:
                {
                    std::cout << ' ';
                    break;
                }
                case MISSED :
                {
                    std::cout << '*';
                    break;
                }
                case HIT:
                {
                    std::cout << 'X';
                    break;
                }
                case NOT_HIT :
                {
                    std::cout << ' '; // we shouldn't see opponents ships
                    break;
                }
            }
            std::cout << '|';
        }
    }
    int ships_left [4]; // index 0 represents 1-decker ship, 1 represents 2-decker, etc.
    int alive;
    int map [10][10];
};
class GAME {
public:
    GAME ()
    : one_score (0), two_score(0), hints (false)
    {}
    FIELD one, two;
    bool manage_menu () // true - start, false - quit
    {
        while (1)
        {
            int option = 2;
            int selection = 0;
            int c;

            initscr();
            clear();
            noecho();
            cbreak(); // Line buffering disabled. Pass on everything
            keypad(stdscr, TRUE);

            int startx = (COLS - WIGHT) / 2;
            int starty = (LINES - HEIGHT) / 2;

            WINDOW *menu_win = newwin(HEIGHT, WIGHT, starty, startx);
            keypad(menu_win, TRUE);
            refresh();
            print_menu(menu_win, option, menu_options, hints, one_score, two_score);

            while (1)
            {
                c = wgetch(menu_win);
                switch (c) {
                    case KEY_UP:
                    {
                        if (option == 2)
                            option = menu_options.size();
                        else
                            option--;
                        break;
                    }
                    case KEY_DOWN:
                    {
                        if (option == menu_options.size())
                            option = 2;
                        else
                            option++;
                        break;
                    }

                    case 10: // Enter
                    {
                        selection = option;
                        break;
                    }
                    default:
                        break;
                }
                print_menu(menu_win, option, menu_options, hints, one_score, two_score);
                if (selection != 0) // player made a choice
                    break;
            }
            clrtoeol();
            refresh();
            endwin();

            switch (selection)
            {
                case 2:
                {
                    std::cout << "\033[H\033[J";
                    return true;
                }
                case 3:
                {
                    if (!hints)
                    {
                        hints = true;
                        break;
                    }
                    hints = false;
                    break;
                }
                case 4:
                {
                    one_score = 0;
                    two_score = 0;
                    break;
                }
                case 5:
                {
                    std::cout << "See you later, capitan!" << std::endl;
                    return false;
                }
            }
        }
    }

private:

    static void print_menu(WINDOW *menu_win, int highlight, const std::vector<std::string> &choices, bool hints, const int one_score, const int two_score)
    {

        int x = (WIGHT - sizeof (choices[0])) / 2 + 1, y = 0;
        box(menu_win, 0, 0);
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
                        mvwprintw(menu_win, y, WIGHT * 3 / 4 , "ON");
                    if (!hints)
                        mvwprintw(menu_win, y, WIGHT * 3 / 4, "OFF");
                }
                wattroff(menu_win, A_REVERSE);
            }
            else // not selected
            {
                mvwprintw(menu_win, y, x, "%s", choices[i].c_str());
                if (i == 2)
                {
                    if (hints)
                        mvwprintw(menu_win, y, WIGHT * 3 / 4, "ON");
                    else
                        mvwprintw(menu_win, y, WIGHT * 3 / 4, "OFF");
                }
            }
            y++;
        }
        mvwprintw(menu_win, 8, (WIGHT-5)/2, "SCORE");
        mvwprintw(menu_win, 9, (WIGHT-5)/2, "%d : %d", one_score, two_score);
        wrefresh(menu_win);
    }
    bool hints;
    int one_score = 0, two_score = 0;
};



int main() {
    std::cout << "\033[H\033[J";
    GAME game;
    while (true)
    {
        if (!game.manage_menu())
            return 0;
        game.two.add_ship(0,0,0,3);
        game.two.add_ship(2,0,2,2);
        game.two.add_ship(4,0,4,2);
        game.two.add_ship(6,0,6,2);
        game.two.add_ship(8,0,8,2);
        while (game.one.placing_ongoing()) //place your ships on field
            {
            std::cout << "\033[H\033[J";
            std::cout << "Here is your field, chose carefully where to hide your fleet" << std::endl;

            game.one.draw ();

            std::string coordinates;
            std::cin >> coordinates;
            if (game.one.add_ship(coordinates[0]-'A', coordinates[1]-'1', coordinates[3]-'A', coordinates[4]-'1'))
                std::cout << "Ship was added\n" << std::endl;
            else
                std::cout << "Write coordinates correctly!\n" << std::endl;
        }

        int x_ = 0, y_ = 0;
        while (game.one.check_loss() && game.two.check_loss()) // manage game
        {
            std::cout << "\033[H\033[J";
            std::cout << "\t **X Battle field X**" << std::endl;
            game.one.draw_players_field();
            std::cout << '\n';
            game.two.draw_opponents_field();
            char x, y;
            std::cin >> x >> y;
            if (game.two.fire(x - 'A', y - '1'))
                continue;
            do
            {
                x++;
                if (x > 9)
                {
                    x = 0;
                    y++;
                }
            } while (game.one.fire(x_, y_));
        }
    }
}