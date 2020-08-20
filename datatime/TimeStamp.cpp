#include "TimeStamp.h"
#include <ctime>
using namespace std::chrono;

namespace
{
template <typename TimeType>
constexpr auto ToTimePoint(int64_t time)
{
    using __from = time_point<system_clock, TimeType>;
    return time_point_cast<system_clock::duration>(__from(TimeType(time)));
}

constexpr auto MicroSecondToTimePoint(int64_t ms_epoch)
{
    return ToTimePoint<microseconds>(ms_epoch);
}

constexpr auto SecondToTimePoint(int64_t second_epoch)
{
    return ToTimePoint<seconds>(second_epoch);
}
} // namespace

TimeStamp::TimeStamp(int64_t ms_epoch)
    : tp(MicroSecondToTimePoint(ms_epoch))
{
}

TimeStamp::TimeStamp(TimePoint time_point)
    : tp(std::move(time_point))
{
}

int64_t TimeStamp::MicroSecondsSinceEpoch() const
{
    return duration_cast<microseconds>(tp.time_since_epoch()).count();
}

Duration TimeStamp::DurationSinceNow(TimeStamp when) const
{
    return when.tp - system_clock::now();
}

std::string TimeStamp::ToString() const
{
    const auto time = system_clock::to_time_t(tp);
    return std::string(std::ctime(&time));
}

void TimeStamp::Swap(TimeStamp& other)
{
    std::swap(tp, other.tp);
}

TimeStamp TimeStamp::Now()
{
    return TimeStamp(system_clock::now());
}

TimeStamp TimeStamp::Invaild()
{
    return TimeStamp(0);
}

bool TimeStamp::operator<(const TimeStamp& other) const
{
    return tp < other.tp;
}

bool TimeStamp::operator==(const TimeStamp& other) const
{
    return tp == other.tp;
}

// friend
TimeStamp AddTime(TimeStamp origin, const Duration& duration_)
{
    return TimeStamp(origin.tp + duration_);
}