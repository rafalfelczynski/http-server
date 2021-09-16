#include "RequestWorker.h"

namespace http
{
RequestWorker::RequestWorker(HttpRequest&& request) : request_(request) {}
}  // namespace http
