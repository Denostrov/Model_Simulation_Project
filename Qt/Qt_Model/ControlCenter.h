#pragma once

#include "RadarStation.h"
#include "LaunchSilo.h"
#include "auxiliary.h"

#include <vector>
#include <string>

enum class ChannelPhase
{
    DISABLED = 0, FREE, SEARCHING, MAINTAINING, EXACT_COORDINATES, ASSIGNED, LAUNCHED, GUIDING_ROCKET, DESTROYED_SUCCESS, DESTROYED_FAIL
};


inline bool operator==(const TargetIdentifier& lhs, const TargetIdentifier& rhs)
{
    return (lhs.radar_id == rhs.radar_id) && (lhs.target_id == rhs.target_id);
}

struct TargetChannel
{
    ChannelPhase phase = ChannelPhase::FREE;
    TargetIdentifier identifier;
};

class ControlCenter
{
public:
    ControlCenter() = default;
    ControlCenter(Position position, std::string const& name);

    void update();

    //unique targets currently being processed
    std::vector<TargetIdentifier> tracked_targets;
    //total tracking channels in the system
    std::vector<std::vector<TargetChannel>> target_channels;

    //input

    //target data from radars
    std::vector<std::vector<IdTargetPair>> radar_targets;
    //number of channels per radar
    std::vector<uint64_t> radar_channels;
    //positions of silos
    std::vector<Position> silo_positions;
    //missile counts of silos
    std::vector<uint64_t> silo_missiles;

    //output

    //radar station orders
    std::vector<RadarStationOrder> radar_orders;
    //launch silo orders
    std::vector<LaunchSiloOrder> launch_silo_orders;

    Position position;
    std::string name;

    bool isTargetTracked(TargetIdentifier id);
    IdTargetPair findTargetWithID(TargetIdentifier id);
private:
    bool isTargetVisible(TargetIdentifier id);

    void process_radar_data();
    void issue_silo_commands();
    void issue_radar_commands();
};
