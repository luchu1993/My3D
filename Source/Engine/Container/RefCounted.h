//
// Created by luchu on 2022/1/1.
//
#pragma once

#include "My3D.h"


namespace My3D
{

struct RefCount
{
    RefCount() : refs_(0), weakRefs_(0) { }
    ~RefCount() { refs_ = -1; weakRefs_ = -1; }

    int refs_;
    int weakRefs_;
};

class MY3D_API RefCounted
{
public:
    RefCounted();
    virtual ~RefCounted();

    RefCounted(const RefCounted& rhs) = delete;
    RefCounted& operator=(const RefCounted& rhs) = delete;

    void AddRef();
    void ReleaseRef();

    int Refs() const;
    int WeakRefs() const;

    RefCount* RefCountPtr() { return refCount_; }

private:
    RefCount* refCount_;
};

}