#pragma once
#include "CompressibleFluidSim/fluid/utils/types.h"

namespace region_structs
{
    enum class types : u8
    {
        none,
        slip_wall,
        supersonic_inflow,
        subsonic_inflow,
        supersonic_outflow,
        subsonic_outflow,
    };
}
