#pragma once

#include <map>
#include <memory>

namespace observer
{
template<typename... DataType>
struct ISubscriber
{
    virtual ~ISubscriber() = default;
    virtual void onPublisherNotification(const DataType&... data) = 0;
};
}  // namespace observer
