//
// Created by luchu on 2022/1/1.
//

#pragma once

#include "My3D.h"


namespace My3D
{

class HashBase;
class ListBase;
class String;
class VectorBase;

template <typename T>
void Swap(T& first, T& second)
{
    T temp = first;
    first = second;
    second = temp;
}

template <> MY3D_API void Swap<String>(String& first, String& second);
template <> MY3D_API void Swap<VectorBase>(VectorBase& first, VectorBase& second);
template <> MY3D_API void Swap<ListBase>(ListBase& first, ListBase& second);
template <> MY3D_API void Swap<HashBase>(HashBase& first, HashBase& second);

}
