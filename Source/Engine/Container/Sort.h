//
// Created by luchu on 2022/1/4.
//

#pragma once

#include "Container/Swap.h"
#include "Container/Iter.h"
#include "Container/VectorBase.h"


namespace My3D
{

    static const int QUICKSORT_THRESHOLD = 16;

    template <typename T> void InsertionSort(RandomAccessIterator<T> begin, RandomAccessIterator<T> end)
    {
        for (auto i = begin + 1; i < end; ++i)
        {
            T temp = *i;
            auto j = i;
            while (j > begin && temp < *(j - 1))
            {
                *j = *(j - 1);
                --j;
            }
            *j = temp;
        }
    }

}
