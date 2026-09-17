#pragma once
#include "CompressibleFluidSim/fluid/utils/types.h"
#include <cuda_runtime.h>


__device__ void atomic_max_f32(f32* current, f32 val)
{
    f32 min;
    f32 current_val;
    do
    {
        current_val = *current;
        min = fmaxf(current_val, val);

    } while ( __float_as_int(current_val) != atomicCAS((i32*)current, __float_as_int(current_val), __float_as_int(min)));
}

