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

#define min(a, b) (( a < b ) ? a : b)
#define max(a, b) (( a < b ) ? b : a)
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

class FIELD {
public:
    FIELD ()
    {
        for (int i = 0; i < 10; i++)
        {
            for (int j = 0; j < 10; j++)
                map[i][j] = NOT_CHECKED;
        }
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
    void draw_player_and_opponent ()
    {
        std::cout << "    A B C D E F G H I J\t\t\t    A B C D E F G H I J" << std::endl;
        for (int i = 0; i < 10; i++)
        {
            draw_player_field_line (i);
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
    bool check_loss ()
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
        for (int i = 0; i < 4; i++)
            left += ships_left[i];
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

int main() {
    int menu = 0;
    bool hints;
    int first_score = 0, second_score = 0;
    std::cout << "\033[H\033[J";
    while (true)
    {
        while (menu != 1) // manage menu
            {
            std::cout << "     **X  Welcome to Sea Battle!  X**     " << std::endl;
            std::cout << "Menu: \n1. Start\n2. Turn on hints\n3. Turn off hints\n4. Show score\n5.Leave" << std::endl;
            std::cin >> menu;
            switch (menu)
            {
                case 1:
                {
                    std::cout << "\033[H\033[J";
                    break;
                }
                case 2:
                {
                    std::cout << "\033[H\033[J";
                    hints = true;
                    std::cout << "Hints were turned on" << std::endl;
                    break;
                }
                case 3:
                {
                    std::cout << "\033[H\033[J";
                    hints = false;
                    std::cout << "Hints were turned off" << std::endl;
                    break;
                }
                case 4:
                {
                    std::cout << "\033[H\033[J";
                    std::cout << "Current score is " << first_score << ':' << second_score << std::endl;
                    if (first_score == second_score && first_score != 0)
                        std::cout << "You both are really good!" << std::endl;
                    if (first_score < second_score)
                        std::cout << "First player wins!" << std::endl;
                    if (first_score > second_score)
                        std::cout << "Second player wins!" << std::endl;
                    break;
                }
                case 5:
                {
                    std::cout << "See you later, capitan!" << std::endl;
                    return 0;
                }

                default:
                {
                    std::cout << "\033[H\033[J";
                    std::cout << "Oppss, try again" << std::endl;
                    break;
                }
            }
        }
        FIELD player, bot;
        bot.add_ship(0,0,0,3);
        bot.add_ship(2,0,2,2);
        bot.add_ship(4,0,4,2);
        bot.add_ship(6,0,6,2);
        bot.add_ship(8,0,8,2);
        while (player.placing_ongoing()) //place your ships on field
            {
            std::cout << "\033[H\033[J";
            std::cout << "Here is your field, chose carefully where to hide your fleet" << std::endl;
            std::cout << "Set position of each ship in a format \"A1:A3\", size of ship is a distance between two points\n" << std::endl;

            player.draw_players_field ();
            player.print_amount_left();

            std::string coordinates;
            std::cin >> coordinates;
            if (player.add_ship(coordinates[0]-'A', coordinates[1]-'1', coordinates[3]-'A', coordinates[4]-'1'))
                std::cout << "Ship was added\n" << std::endl;
            else
                std::cout << "Write coordinates correctly!\n" << std::endl;
            ;
        }

        int x_ = 0, y_ = 0;
        while (player.check_loss() && bot.check_loss()) // manage game
        {
            std::cout << "\033[H\033[J";
            std::cout << "\t **X Battle field X**" << std::endl;
            player.draw_players_field();
            std::cout << '\n';
            bot.draw_opponents_field();
            char x, y;
            std::cin >> x >> y;
            if (bot.fire(x - 'A', y - '1'))
                continue;
            do
            {
                x++;
                if (x > 9)
                {
                    x = 0;
                    y++;
                }
            } while (player.fire(x_, y_));
        }
    }
}
