#pragma once

#include "auxiliary.h"

class Missile
{
public:
    Missile(Position position, Position target_position, TargetIdentifier target_id, double max_G, double speed);

    void update();

    Position current_position;
    double max_G;
    double speed;
    Vector3D direction;
    TargetIdentifier target_id;

    std::vector<Position> log_position;

    bool exploded = false;
    //input

    Position target_position;
};
