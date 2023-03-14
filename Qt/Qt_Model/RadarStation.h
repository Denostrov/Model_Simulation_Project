#pragma once

#include <vector>

#include "auxiliary.h"

class AirObject;

struct TrackedTarget
{
    uint64_t env_id;
    IdTargetPair target;
};

class Radar
{
public:
    Radar(double direction, double range, double rotation_speed, double scan_speed, double azimuth_sector_size,
                    double elevation_sector_size, int32_t max_track_targets);

    void update(std::vector<Target> const& environment_targets, Position const& radar_position, std::vector<uint64_t>& maintained_targets);
    //returns which of the targets are visible
    void findTargets(std::vector<Target> const& environment_targets, Position const& radar_position, std::vector<uint64_t>& maintained_targets);

    double direction;
    double range;
    double rotation_speed;
    double scan_speed;
    double azimuth_sector_size;
    double elevation_sector_size;
    double scanbeam_direction;
    int32_t max_track_targets;

    //
    std::vector<TrackedTarget> last_visible_targets;
    std::vector<TrackedTarget> visible_targets;

    bool scan_complete = false;
    uint64_t next_free_target_id = 0;
private:
    void rotateSector();
};

class RadarStation
{
public:
    RadarStation(Radar const& radar, Position position, std::string const& name);

    void update();

    ~RadarStation() = default;

    double position_error_range;
    double speed_error_range;
    Radar radar;
    Position position;
    std::string name;

    std::vector<uint64_t> maintained_targets;

    std::vector<std::vector<Position>> log_targets;
    std::vector<uint64_t> log_load;

    //input variables

    //all targets in Environment
    std::vector<Target> environment_targets;
    //orders from ControlCenter
    std::vector<RadarStationOrder> orders;

    //output variables

    //visible targets with errors and radar id to ControlCenter
    std::vector<IdTargetPair> visible_targets;
private:
    //populates the visible targets vector
    void findTargets();
};
