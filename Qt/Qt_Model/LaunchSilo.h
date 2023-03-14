#pragma once
#include <vector>
#include <deque>
#include "auxiliary.h"
#include "Missile.h"

enum class LaunchSiloStatus
{
    FREE, BUSY
};

class LaunchSilo
{
public:
    LaunchSilo(Position position, std::string const& name, std::vector<uint64_t>const& missiles);
	~LaunchSilo();

    void update();
    void launchMissile(Position goal, MissileType type, TargetIdentifier target_id);

    Position position;
    std::string name;

    double timer;
    LaunchSiloStatus status;

    std::vector<uint64_t> missiles;      // {n_missiles_of_1_type, m_missiles_of_2_type, k_missiles_of_3_type}

    std::vector<LaunchSiloOrder> cached_orders;

    //input

    std::vector<LaunchSiloOrder> orders;

    //output

    std::vector<Missile> launched_missiles;
};

