#include "field.h"

int limit (const int a)
{
    if (a < 0)
        return 0;
    if (a > 9)
        return 9;
    return a;
}
#define min(a, b) (( a < b ) ? a : b)
#define max(a, b) (( a < b ) ? b : a)


    FIELD::FIELD ()
            : m_alive (20), last_fire(0, 0), m_ships_not_placed {4, 3, 2, 1}
    {
        for (auto & i : m_map)
            for (int & j : i)
                j = NOT_CHECKED;
    }
    int FIELD::size_remains (int size)
    {
        return m_ships_not_placed[size];
    }
    int FIELD::cell_state(coordinates check) const
    {
        return m_map[check.y][check.x];
    }
    bool FIELD::add_ship (coordinates from, coordinates to)
    {
        //check if there is ship too close to a new m_one
        for (int i = limit(min (from.x, to.x) - 1); i <= limit(max (from.x, to.x) + 1); i++)
            for (int j = limit(min (from.y, to.y) - 1); j <= limit(max (from.y, to.y) + 1); j++)
                if (m_map[j][i] == NOT_HIT)
                    return false;

        //add ship to the m_map
        for (int i = min (from.x, to.x); i <= max (from.x, to.x); i++)
            for (int j = min (from.y, to.y); j <= max (from.y, to.y); j++)
                m_map[j][i] = NOT_HIT;

        int size = max (max(from.x, to.x) - min(from.x, to.x), max(from.y, to.y) - min(from.y, to.y));
        m_ships_not_placed[size]--;
        return true;
    }
    bool FIELD::check_loss () const
    {
        return m_alive;
    }
    bool FIELD::fire (coordinates fire) // false - done, true - continue fire
    {
        if (m_map[fire.y][fire.x] == NOT_CHECKED)
        {
            m_map[fire.y][fire.x] = MISSED;
            return false;
        }
        if (m_map[fire.y][fire.x] == NOT_HIT)
        {
            m_alive--;
            m_map[fire.y][fire.x] = HIT;
            //check if ship is dead
            if (!is_ship_alive (fire, fire))
                update_surrounding (fire, fire);
            if (!m_alive)
                return false;
            return true;
        }
        if (m_map[fire.y][fire.x] == HIT || m_map[fire.y][fire.x] == MISSED)
        {
            return true;
        }
        return false;
    }
    void FIELD::reset ()
    {
        for (auto & i : m_map)
            for (int & j : i)
                j = NOT_CHECKED;
        m_alive = 20;
        last_fire.x = 0;
        last_fire.y = 0;
        for (int i = 0; i < 4 ; i++)
            m_ships_not_placed[i] = 4 - i;
    }
    bool FIELD::is_ship_alive (coordinates check, coordinates ignore)
    {
        for (int i = limit (check.x - 1); i <= limit (check.x+1); i++)
        {
            for (int j = limit (check.y - 1); j <= limit (check.y+1); j++)
            {
                if ((j == ignore.y && i == ignore.x) || (j == check.y && i == check.x))
                    continue;
                switch (m_map[j][i])
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
    void FIELD::update_surrounding (coordinates update, coordinates ignore)
    {
        for (int i = limit (update.x - 1); i <= limit (update.x+1); i++)
        {
            for (int j = limit (update.y - 1); j <= limit (update.y+1); j++)
            {
                if ((j == ignore.y && i == ignore.x) || (j == update.y && i == update.x))
                    continue;
                switch (m_map[j][i])
                {
                    case MISSED:
                        continue;
                    case NOT_CHECKED:
                        m_map[j][i] = MISSED;
                        break;
                    case HIT:
                        update_surrounding ({i, j}, update);
                    default:
                        continue;
                }
            }
        }
    }
