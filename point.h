#ifndef POINT_H
#define POINT_H

#include "common.h"

typedef struct{
    int x;
    int y;
}Point;


float point_distance(Point p1, Point p2);

#endif
