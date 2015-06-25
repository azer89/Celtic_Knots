#ifndef TILETYPE_H
#define TILETYPE_H

enum TileType
{
    TILE_CORNER             = 0,
    TILE_CROSS_OVER         = 1,
    TILE_CROSS_UNDER        = 2,
    TILE_STRAIGHT_CROSS     = 3,
    TILE_STRAIGHT           = 4,
    TILE_BACKSLASH          = 5, /*   \   */
    TILE_SLASH              = 6, /*   /   */
    TILE_NONE               = 7,
};

#endif // TILETYPE_H
