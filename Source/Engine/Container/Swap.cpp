//
// Created by luchu on 2022/1/1.
//

#include "Container/Swap.h"
#include "Container/VectorBase.h"
#include "Container/String.h"
#include "Container/ListBase.h"
#include "Container/HashBase.h"


namespace My3D
{

template <> void Swap<String>(String& first, String& second)
{
    first.Swap(second);
}

template <> void Swap<VectorBase>(VectorBase& first, VectorBase& second)
{
    first.Swap(second);
}

template <> void Swap<ListBase>(ListBase& first, ListBase& second)
{
    first.Swap(second);
}

template <> void Swap<HashBase>(HashBase& first, HashBase& second)
{
    first.Swap(second);
}

}