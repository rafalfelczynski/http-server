#include "Url.h"

namespace http
{
Url::Url(std::string address) : mainAddress_(address) {}
const std::string& Url::getAddress() const { return mainAddress_; }
}  // namespace http
