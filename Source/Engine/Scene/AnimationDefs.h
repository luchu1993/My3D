//
// Created by luchu on 2022/2/1.
//

#pragma once

namespace My3D
{
    /// Animation wrap mode
    enum WrapMode
    {
        /// Loop mode
        WM_Loop = 0,
        /// Play once, when animation finished it will be removed
        WM_ONCE,
        /// Clamp mode
        WM_CLAMP,
    };
}
