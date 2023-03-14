#include "Missile.h"

Missile::Missile(Position position, Position target_position, TargetIdentifier target_id, double max_G, double speed)
    :current_position{position}, max_G{max_G}, speed{speed}, target_id{target_id}, target_position{target_position}
{
    direction = (target_position - position).normalize();
    log_position.resize(SimParameters::simulation_tick + 1, position);
}

void Missile::update()
{
    if ((target_position - current_position).length() <= 0.0001)
    {
        exploded = true;
    }
    direction = (target_position - current_position).normalize();
    current_position += direction * speed * SimParameters::TIME_STEP;
    if (dot(direction, target_position - current_position) < 0.0)
    {
        current_position = target_position;
        exploded = true;
    }
    log_position.push_back(current_position);
}
