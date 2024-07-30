#ifndef SEABATTLE_FIELD_H
#define SEABATTLE_FIELD_H

enum cell_state
{
    NOT_CHECKED,
    MISSED,
    HIT,
    NOT_HIT
};
struct coordinates {
    coordinates (int x_, int y_)
            : x(x_), y(y_) {}
    int x, y;
};
int limit (int a);

class FIELD {
public:
    FIELD ();
    int size_remains (int size);
    int cell_state(coordinates check) const;
    bool add_ship (coordinates from, coordinates to);
    bool check_loss () const;
    bool fire (coordinates fire); // false - done, true - continue fire
    void reset ();
    coordinates last_fire;
private:
    bool is_ship_alive (coordinates check, coordinates ignore);
    void update_surrounding (coordinates update, coordinates ignore);
    int m_ships_not_placed [4]; // index 0 represents 1-decker ship, 1 represents 2-decker, etc.
    int m_alive;
    int m_map [10][10];
};

#endif //SEABATTLE_FIELD_H
