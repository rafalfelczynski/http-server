#pragma once

#include <map>
#include <memory>

namespace observer
{
template<typename... DataType>
class IPublisher
{
public:
    virtual ~IPublisher() = default;
    virtual unsigned addObserver(std::function<void(const DataType&...)> obs)
    {
        observers_.emplace(nextKey_, std::move(obs));
        return nextKey_++;
    }

    virtual void removeObserver(const unsigned key)
    {
        auto observer = observers_.find(key);
        if (observer != observers_.end())
        {
            observers_.erase(observer);
        }
    }

    virtual void notifyObservers(const DataType&... data)
    {
        for (const auto& obs : observers_)
        {
            obs.second(data...);
        }
    }

protected:
    std::map<unsigned, std::function<void(const DataType&...)>> observers_;
    unsigned nextKey_ = 0;
};
}  // namespace observer
