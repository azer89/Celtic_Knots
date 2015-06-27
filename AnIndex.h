#ifndef ANINDEX_H
#define ANINDEX_H

struct AnIndex
{
public:
    // x
    float x;

    // y
    float y;

    // Default constructor
    AnIndex()
    {
        this->x = -1;
        this->y = -1;
    }

    AnIndex(int x, int y)
    {
        this->x = x;
        this->y = y;
    }

    bool operator== (const AnIndex& other)
    { return this->x == other.x && this->y == other.y; }
};

#endif // ANINDEX_H
