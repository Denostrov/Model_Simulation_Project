#include "RadarStation.h"

Radar::Radar(double direction, double range, double rotation_speed, double scan_speed, double azimuth_sector_size,
                                 double elevation_sector_size, int32_t max_track_targets)
    :direction{direction}, range{range}, rotation_speed{rotation_speed}, scan_speed{scan_speed}, azimuth_sector_size{azimuth_sector_size},
      elevation_sector_size{elevation_sector_size}, max_track_targets{max_track_targets}
{
    scanbeam_direction = direction + azimuth_sector_size / 2.0;
}

void Radar::update(std::vector<Target> const& environment_targets, Position const& radar_position, std::vector<uint64_t>& maintained_targets)
{
    findTargets(environment_targets, radar_position, maintained_targets);
    rotateSector();
}

void Radar::rotateSector()
{

}

void Radar::findTargets(std::vector<Target> const& environment_targets, Position const& radar_position, std::vector<uint64_t>& maintained_targets)
{
    auto check_target_sector_intersect = [&radar_position](double start_direction, double end_direction, double range, Position target_position)
    {
        double distance = (Position(target_position.x, 0.0, target_position.z) - Position(radar_position.x, 0.0, radar_position.z)).length();
        double angle = radToDeg(std::atan2(target_position.x - radar_position.x, target_position.z - radar_position.z)) - 90.0;
        if (angle < -180.0)
        {
            angle += 360.0;
        }
        return angle > end_direction && angle < start_direction && distance < range;
    };

    double start_scanbeam_direction = scanbeam_direction;
    scanbeam_direction -= scan_speed * SimParameters::TIME_STEP;
    if (scanbeam_direction < direction - azimuth_sector_size / 2.0)
    {
        scanbeam_direction = direction + azimuth_sector_size / 2.0;
        scan_complete = true;
        std::cout << "scan finished" << std::endl;
    }
    double end_scanbeam_direction = scanbeam_direction;

    for (uint64_t i = 0; i < environment_targets.size(); i++)
    {
        bool target_visible = false;
        if (start_scanbeam_direction < end_scanbeam_direction)
        {
            target_visible = check_target_sector_intersect(start_scanbeam_direction, direction - azimuth_sector_size / 2.0, range, environment_targets[i].position)
                    || check_target_sector_intersect(direction + azimuth_sector_size / 2.0, end_scanbeam_direction, range, environment_targets[i].position);
        }
        else
        {
            target_visible = check_target_sector_intersect(start_scanbeam_direction, end_scanbeam_direction, range, environment_targets[i].position);
        }
        if (target_visible)
        {
            std::cout << "Target visible: " << i << std::endl;
            auto find_func = [&i](TrackedTarget targ){ return targ.env_id == i;};
            auto target_it = std::find_if(last_visible_targets.begin(), last_visible_targets.end(), find_func);
            if (std::find_if(visible_targets.begin(), visible_targets.end(), find_func) == visible_targets.end())
            {
                if (target_it != last_visible_targets.end())
                {
                    visible_targets.push_back(TrackedTarget{i,
                                                            {(*target_it).target.first, environment_targets[i]}});
                }
                else
                {
                    visible_targets.push_back(TrackedTarget{i, {next_free_target_id, environment_targets[i]}});
                    next_free_target_id++;
                }
            }
        }
    }

    for (uint64_t i = 0; i < maintained_targets.size(); i++)
    {
        auto find_func = [&maintained_targets, &i](TrackedTarget targ){ return targ.target.first == maintained_targets[i];};
        auto target_it = std::find_if(last_visible_targets.begin(), last_visible_targets.end(), find_func);
        if (target_it != last_visible_targets.end())
        {
            if (check_target_sector_intersect(direction + azimuth_sector_size / 2.0, direction - azimuth_sector_size / 2.0, range, environment_targets[maintained_targets[i]].position))
            {
                (*target_it).target.second = environment_targets[target_it->env_id];
                std::cout << "Maintained target " << target_it->env_id << std::endl;
            }
            else
            {
                maintained_targets.erase(maintained_targets.begin() + i);
                i--;
            }
        }
    }

    if (scan_complete)
    {
        last_visible_targets = visible_targets;
        visible_targets.clear();
        scan_complete = false;
    }

    return;
}

RadarStation::RadarStation(Radar const& radar, Position position, std::string const& name)
    :radar{radar}, position{position}, name{name}
{

}

void RadarStation::update()
{
    for (auto order : orders)
    {
        maintained_targets.push_back(std::round(order.value));
    }
    log_load.push_back(maintained_targets.size());

    radar.update(environment_targets, position, maintained_targets);

    findTargets();
}

void RadarStation::findTargets()
{
    log_targets.resize(environment_targets.size());
    visible_targets.clear();
    for (auto const& target : radar.last_visible_targets)
    {
        visible_targets.emplace_back(target.target);
        log_targets[target.env_id].resize(SimParameters::simulation_tick + 1, {0.0, 0.0, 0.0});
        log_targets[target.env_id][SimParameters::simulation_tick] = target.target.second.position;
    }
}
