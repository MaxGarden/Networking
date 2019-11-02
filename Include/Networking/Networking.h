#pragma once

#include <string>
#include <vector>
#include <memory>
#include <functional>
#include <assert.h>

namespace Networking
{
    using byte = unsigned char;

#define NETWORKING_ASSERT(expression) assert(expression)

#define DECLARE_POINTERS(x)                                 \
    class x;                                                \
    using x##SharedPtr = std::shared_ptr<x>;                \
    using x##ConstSharedPtr = std::shared_ptr<const x>;     \
    using x##WeakPtr = std::weak_ptr<x>;                    \
    using x##ConstWeakPtr = std::weak_ptr<const x>;         \
    using x##UniquePtr = std::unique_ptr<x>;                \
    using x##ConstUniquePtr = std::unique_ptr<const x>;     \


    DECLARE_POINTERS(INetwork);
    DECLARE_POINTERS(IConnection);
}