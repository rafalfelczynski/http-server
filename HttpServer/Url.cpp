#include "Url.h"

namespace http
{
Url::Url(std::string address) : address_(address) {}
const std::string& Url::getAddress() const { return address_; }
}  // namespace http
