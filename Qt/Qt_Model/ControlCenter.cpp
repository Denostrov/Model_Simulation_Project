# include "ControlCenter.h"

ControlCenter::ControlCenter(Position position, std::string const& name)
    :position{position}, name{name}
{}

//struct Match
//{
//    uint64_t record_idx;
//    uint64_t target_idx;
//    double distance; // sqaured actually
//    bool is_target_confirmed;

//    static bool cmp(Match const & a, Match const & b)
//    {
//        // comparison function object which returns ​true if the first argument is less than (i.e. is ordered before) the second
//        if (a.is_target_confirmed == true && b.is_target_confirmed == false)
//        {
//            return true;
//        }
//        if (a.is_target_confirmed == false && b.is_target_confirmed == true)
//        {
//            return false;
//        }
//        return a.distance <= b.distance;
//    }
//};

//bool is_intersect(FuzzyTarget const & t1, FuzzyTarget const & t2)
//{
//   bool is_intersect = true;
//   is_intersect = is_intersect && (t1.position.x + t1.position_errors[0] > t2.position.x - t2.position_errors[0]);
//   is_intersect = is_intersect && (t1.position.x - t1.position_errors[0] < t2.position.x + t2.position_errors[0]);
//   is_intersect = is_intersect && (t1.position.y + t1.position_errors[1] > t2.position.y - t2.position_errors[1]);
//   is_intersect = is_intersect && (t1.position.y - t1.position_errors[1] < t2.position.y + t2.position_errors[1]);
//   is_intersect = is_intersect && (t1.position.z + t1.position_errors[2] > t2.position.z - t2.position_errors[2]);
//   is_intersect = is_intersect && (t1.position.z - t1.position_errors[2] < t2.position.z + t2.position_errors[2]);
//   return is_intersect;
//}

void ControlCenter::update()
{
    process_radar_data();
    issue_silo_commands();
    issue_radar_commands();

    for (auto& radar_channels : target_channels)
    {
        for (auto& channel : radar_channels)
        {
            if (channel.phase != ChannelPhase::FREE && !isTargetVisible(channel.identifier))
            {
                channel.phase = ChannelPhase::FREE;
            }
        }
    }
}

bool ControlCenter::isTargetVisible(TargetIdentifier id)
{
    auto result = std::find_if(radar_targets[id.radar_id].begin(), radar_targets[id.radar_id].end(), [&id](IdTargetPair pair){return pair.first == id.target_id;});
    if (result != radar_targets[id.radar_id].end())
    {
        return true;
    }
    return false;
}

IdTargetPair ControlCenter::findTargetWithID(TargetIdentifier id)
{
    auto result = std::find_if(radar_targets[id.radar_id].begin(), radar_targets[id.radar_id].end(), [&id](IdTargetPair pair){return pair.first == id.target_id;});
    return *result;
}

bool ControlCenter::isTargetTracked(TargetIdentifier id)
{
    bool is_tracked = false;
    for (auto& radar_channels : target_channels)
    {
        auto result = std::find_if(radar_channels.begin(), radar_channels.end(), [&id](TargetChannel channel){return channel.identifier == id;});
        if (result != radar_channels.end() && result->phase != ChannelPhase::FREE)
        {
            is_tracked = true;
        }
    }

    return is_tracked;
}

void ControlCenter::process_radar_data()
{
    auto target_priority_sort = [this](TargetIdentifier const& left, TargetIdentifier const& right)
    {
        auto left_target = findTargetWithID(left).second;
        auto right_target = findTargetWithID(right).second;
        return (left_target.position - this->position).length()/left_target.speed < (right_target.position - this->position).length()/right_target.speed;
    };

    tracked_targets.clear();
    for (uint64_t i = 0; i < radar_targets.size(); i++)
    {
        for (auto const& target : radar_targets[i])
        {
            tracked_targets.push_back({i, target.first});
        }
    }
    std::sort(tracked_targets.begin(), tracked_targets.end(), target_priority_sort);

    target_channels.resize(radar_targets.size());
    for (uint64_t i = 0; i < radar_channels.size(); i++)
    {
        target_channels[i].resize(radar_channels[i]);
    }
}

void ControlCenter::issue_radar_commands()
{
    auto assign_free_target_to_channel = [this](TargetChannel& channel, uint64_t radar_id)
    {
        for (auto const& target : tracked_targets)
        {
            if (target.radar_id == radar_id && !isTargetTracked(target))
            {
                channel.phase = ChannelPhase::MAINTAINING;
                channel.identifier = target;
                std::cout << "set channel to maintain target " << target.radar_id << " " << target.target_id << std::endl;
            }
        }
    };

    for (uint64_t i = 0; i < target_channels.size(); i++)
    {
        for (auto& channel : target_channels[i])
        {
            if (channel.phase == ChannelPhase::FREE)
            {
                assign_free_target_to_channel(channel, i);
            }
        }
    }

    radar_orders.clear();
    for (auto const& radar_channels : target_channels)
    {
        for (auto const& channel : radar_channels)
        {
            if (channel.phase == ChannelPhase::MAINTAINING)
            {
                radar_orders.push_back({channel.identifier.radar_id, RadarStationOrderType::MAINTAIN_TARGET, (double)channel.identifier.target_id});
            }
        }
    }
}

void ControlCenter::issue_silo_commands()
{
    auto find_closest_silo_to_target = [this](Target const& target)
    {
        uint64_t result = 0;
        double min_distance = std::numeric_limits<double>::max();
        for (uint64_t i = 0; i < silo_positions.size(); i++)
        {
            auto distance = (silo_positions[i] - target.position).length();
            if (distance < min_distance && silo_missiles[i] > 0)
            {
                result = i;
                min_distance = distance;
            }
        }
        silo_missiles[result]--;
        return result;
    };

    launch_silo_orders.clear();
    for (auto& radar_channels : target_channels)
    {
        for (auto& channel : radar_channels)
        {
            if (channel.phase == ChannelPhase::MAINTAINING)
            {
                channel.phase = ChannelPhase::LAUNCHED;
                auto target = findTargetWithID(channel.identifier).second;
                launch_silo_orders.push_back({find_closest_silo_to_target(target), {MissileType::FAST, target.position, channel.identifier}});
            }
        }
    }
}

//    enum RecordStatus { unmatched, matched, identified};
//    std::vector<RecordStatus> records_status(records.size(), RecordStatus::unmatched); // was record identified with some target at least once
//    std::vector<Match> matches;
//    for (uint64_t i = 0; i < records.size(); i = i + 1)
//    {
//        for (uint64_t j = 0; j < targets.size(); j = j + 1)
//        {
//            if (!is_intersect(records[i], targets[j].strob))
//            {
//                continue;
//            }
//            records_status[i] = RecordStatus::matched;
//            double distance = (records[i].position.x - targets[j].strob.position.x) * (records[i].position.x - targets[j].strob.position.x) +
//                    + (records[i].position.y - targets[j].strob.position.y) * (records[i].position.y - targets[j].strob.position.y)
//                    + (records[i].position.z - targets[j].strob.position.z) * (records[i].position.z - targets[j].strob.position.z);
//            matches.push_back(Match{i, j, distance, targets[j].target_status == Target::TargetStatus::confirmed});
//        }
//    }
//    std::sort(matches.begin(), matches.end(), Match::cmp);
//    std::vector<bool> is_target_identified(targets.size(), false); // have target been identified with some record already
//    for (uint64_t i = 0; i < matches.size(); i = i + 1)
//    {
//        uint64_t & target_idx = matches[i].target_idx;
//        uint64_t & record_idx = matches[i].record_idx;
//        if (is_target_identified[target_idx] == true || records_status[record_idx] == RecordStatus::identified)
//        {
//            continue;
//        }
//        targets[target_idx].trajectory.push_back(records[matches[i].record_idx]);
//        is_target_identified[target_idx] = true;
//        records_status[record_idx] = RecordStatus::identified;
//    }
//    for (uint64_t i = 0; i < targets.size(); i = i + 1)
//    {
//        std::vector<bool> & was_identified = targets[i].was_identified;
//        was_identified.insert(was_identified.begin(), is_target_identified[i]);
//        if (was_identified.size() > ControlCenter::iter_count)
//        {
//            was_identified.erase(was_identified.begin()+ControlCenter::iter_count, was_identified.end());
//        }
//        uint64_t miss_count = std::count(was_identified.begin(), was_identified.end(), false);
//        if ( (miss_count > ControlCenter::iter_count - ControlCenter::register_threshold) && targets[i].launches.size() == 0)
//        {
//            targets[i].target_status = Target::TargetStatus::drop;
//        }
//        else if (was_identified.size() == ControlCenter::iter_count)
//        {
//            targets[i].target_status = Target::TargetStatus::confirmed;
//        }
//    }
//    targets.erase(std::remove_if(targets.begin(), targets.end(), [](Target const & x){return x.target_status == Target::TargetStatus::drop;}),
//                  targets.end());
//    for (uint64_t i = 0; i < records_status.size(); i = i + 1)
//    {
//        if (records_status[i] == RecordStatus::unmatched)
//        {
//            Target new_target = Target(ControlCenter::target_count);
//            ControlCenter::target_count = ControlCenter::target_count + 1;
//            new_target.trajectory.push_back(records[i]);
//            new_target.was_identified.push_back(true);
//            new_target.target_status = Target::TargetStatus::trial;
//            targets.push_back(new_target);
//        }
//    }
//    for (uint64_t i = 0; i < targets.size(); i = i + 1)
//    {
//        std::vector<bool> & was_identified = targets[i].was_identified;
//        double t = TIME_STEP * std::distance(was_identified.begin(), std::find(was_identified.begin(), was_identified.end(), true)); // how many iterations have passed since last identification
//        FuzzyTarget & strob = targets[i].strob;
//        strob.position.x = strob.position.x + strob.speed.x * t;
//        strob.position.y = strob.position.y + strob.speed.y * t;
//        strob.position.z = strob.position.z + strob.speed.z * t;
//        strob.position_errors = std::vector<double>(3, ControlCenter::max_acceleration * t * t / 2);
//    }
