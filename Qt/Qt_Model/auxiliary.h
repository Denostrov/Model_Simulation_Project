#pragma once

#include <cmath>
#include <vector>
#include <algorithm>
#include <fstream>
#include <string>
#include <sstream>
#include <deque>
#include <iostream>

class Random
{
public:
    static uint64_t randInt();
    static uint64_t randDouble();
    static uint64_t state;
};

enum class RadarStationOrderType
{
    ROTATE_SECTOR, MAINTAIN_TARGET
};

struct RadarStationOrder
{
    uint64_t radar_id;
    RadarStationOrderType type;
    double value;
};

constexpr double CHART_RANGE = 100.0;
constexpr double PI = 3.14159265;

struct SimParameters
{
    static uint64_t simulation_tick;
    static double TIME_STEP;
    static double TIME_MULTIPLIER;
    static double SIMULATION_TIME;
};

double rangeToPieSize(double range);
double positionToPiePosition(double position);
double degToRad(double deg);
double radToDeg(double rad);
double spriteSizeToPosition(double size);

struct Position
{
    Position();
    Position(double x, double y, double z);

    Position normalize();
    double length();

    double x;
    double y;
    double z;

    Position& operator-=(Position const& rhs);
    Position& operator+=(Position const& rhs);
    Position& operator*=(double rhs);
};
Position operator-(Position lhs, Position const& rhs);
Position operator+(Position lhs, Position const& rhs);
Position operator*(Position lhs, double rhs);
bool operator==(Position const& lhs, Position const& rhs);
double dot(Position const& lhs, Position const& rhs);
using Vector3D = Position;


//Fuzzy target will be handled on radar's side
struct FuzzyTarget
{
    Position position;
    Vector3D speed;
    std::vector<double> position_errors;
    std::vector<double> speed_errors;
};

struct Target
{
    Position position;
    Vector3D direction;
    double speed;
};
using IdTargetPair = std::pair<uint64_t, Target>;

struct TargetIdentifier
{
    //which radar tracks
    uint64_t radar_id;
    //id of target in radar
    uint64_t target_id;
};

enum class MissileType
{
    FAST = 0
};

struct LaunchSiloEvent{
    MissileType missileType;
    Position goalPosition;
    TargetIdentifier target_id;
};


struct LaunchSiloOrder
{
    uint64_t silo_id;
    LaunchSiloEvent event;
};
