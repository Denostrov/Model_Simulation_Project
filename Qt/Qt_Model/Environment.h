#pragma once
#include <vector>
#include <array>
#include "auxiliary.h"
#include <cmath>
#include "Missile.h"

class AirObject{
public:
    AirObject(std::vector<Position> const& trajectory_targets, double max_G, double speed);

    void update();

    bool trajectory_created = false;
    std::pair<Position, Position> circle_trajectory;
    Position current_position;
    uint64_t current_target;
    std::vector<Position> trajectory_targets;
    double max_G;
    double speed;
    double current_radius;
    Vector3D direction;
    bool destroyed = false;

private:
    std::pair<Position, double> flatness_by_3_points(Position const& p1, Position const& p2, Position const& p3);
    Position vector_product(Position const& p1, Position const & p2);
    Position gaus_solve(std::array <std::array <double,4>,3> a);
    std::pair<Position, Position> calculate_trajectory (Position const& t, Position const& a, Position const& c);
    Position round_position(Position const& pos);
    std::pair<double, double> intersect(double a1, double a2, double b1, double b2, double c1, double c2);
};

class Environment{
  public:
    Environment();
    explicit Environment(std::vector<AirObject> const & air_objects);
    void update();

    std::vector<AirObject> air_objects;
    std::vector<Missile> missiles;

    std::vector<std::vector<Position>> log_targets;
    std::vector<std::vector<Position>> log_missiles;

private:
    void checkExplosion(Position epicenter);
};
