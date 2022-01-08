//
// Created by luchu on 2022/1/3.
//

#include "Container/HashBase.h"


namespace My3D
{
    void HashBase::AllocateBuckets(unsigned int size, unsigned int numBuckets)
    {
        delete [] ptrs_;

        auto ptrs = new HashNodeBase* [numBuckets + 2];
        auto* data = reinterpret_cast<unsigned*>(ptrs);
        data[0] = size;
        data[1] = numBuckets;
        ptrs_ = ptrs;

        ResetPtrs();
    }

    void HashBase::ResetPtrs()
    {
        // Reset bucket pointers
        if (!ptrs_)
            return;

        unsigned numBuckets = NumBuckets();
        HashNodeBase** ptrs = Ptrs();
        for (unsigned i = 0; i < numBuckets; ++i)
            ptrs[i] = nullptr;
    }
}
