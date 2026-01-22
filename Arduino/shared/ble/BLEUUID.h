#pragma once

#include <ArduinoBLE.h>

using BLEUUID = std::array<char, 37>;

namespace btutil
{
BLEUUID makeUuidWithService(
    const std::string& serviceName,
    const std::string& overrideId);

BLEUUID makeUuidWithProperty(
    const std::string& propertyId,
    const BLEUUID& serviceId);
}
