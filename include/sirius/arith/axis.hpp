#pragma once

namespace acma {
    enum class axis { 
        x, y, z, w,
        pitch = x, yaw = y, roll = z 
    };
}

namespace acma {
    enum class axis_direction : bool{
        negative,
        positive
    };
}