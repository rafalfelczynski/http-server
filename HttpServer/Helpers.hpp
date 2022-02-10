#pragma once

namespace helpers
{
// helper type for the visitor #4
template<class... Ts>
struct compose : Ts...
{
    using Ts::operator()...;
};
// explicit deduction guide (not needed as of C++20)
template<class... Ts>
compose(Ts...) -> compose<Ts...>;
}  // namespace helpers
