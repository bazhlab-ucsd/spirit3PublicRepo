#include "coord.h"

Coord::Coord()
{
    x = 0;
    y = 0;
    z = 0;
}
Coord::Coord(int x_, int y_)
{
    x = x_;
    y = y_;
    z = 0;
}
Coord::Coord(int x_, int y_, int z_)
{
    x = x_;
    y = y_;
    z = z_;
}
