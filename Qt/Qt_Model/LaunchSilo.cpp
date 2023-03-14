#include "LaunchSilo.h"

LaunchSilo::LaunchSilo(Position position, std::string const& name, std::vector<uint64_t>const& missiles)
    :position{position}, name{name}, missiles{missiles}
{
     timer = 0.;
     status = LaunchSiloStatus::FREE;
}

LaunchSilo::~LaunchSilo()
{
}


void LaunchSilo::update()
{
    launched_missiles.clear();

    for (auto const& order : orders)
    {
        cached_orders.push_back(order);
    }

    if (timer > 0.){
        timer -= SimParameters::TIME_STEP;
        if (timer <= 0.){
            timer = 0.;
            status = LaunchSiloStatus::FREE;
        }
        return;
    }

    if (!cached_orders.empty() && status == LaunchSiloStatus::FREE){
        launchMissile(cached_orders[0].event.goalPosition, cached_orders[0].event.missileType, cached_orders[0].event.target_id);
        cached_orders.erase(cached_orders.begin());
        timer = 0.5;
        status = LaunchSiloStatus::BUSY;
    }
}

void LaunchSilo::launchMissile(Position goal, MissileType type, TargetIdentifier target_id)
{
    if (missiles[0] > 0)
    {
        missiles[0]--;
        launched_missiles.push_back(Missile(position, goal, target_id, 10.0, 40.0));
    }
}

