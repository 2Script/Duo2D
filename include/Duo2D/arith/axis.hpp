#pragma once

namespace d2d {
    enum class axis { 
        x, y, z, w,
        pitch = x, yaw = y, roll = z 
    };
}

namespace d2d {
    enum class axis_direction : bool{
        negative,
        positive
    };
}