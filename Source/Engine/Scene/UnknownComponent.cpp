//
// Created by luchu on 2022/2/11.
//

#include "Scene/UnknownComponent.h"

namespace My3D
{
    static HashMap<StringHash, String> unknownTypeToName;
    static String letters("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ");

    static String GenerateNameFromType(StringHash typeHash)
    {
        if (unknownTypeToName.Contains(typeHash))
            return unknownTypeToName[typeHash];

        String test;

        // Begin brute-force search
        unsigned numLetters = letters.Length();
        unsigned combinations = numLetters;
        bool found = false;

        for (unsigned i = 1; i < 6; ++i)
        {
            test.Resize(i);

            for (unsigned j = 0; j < combinations; ++j)
            {
                unsigned current = j;

                for (unsigned k = 0; k < i; ++k)
                {
                    test[k] = letters[current % numLetters];
                    current /= numLetters;
                }

                if (StringHash(test) == typeHash)
                {
                    found = true;
                    break;
                }
            }

            if (found)
                break;

            combinations *= numLetters;
        }

        unknownTypeToName[typeHash] = test;
        return test;
    }

    UnknownComponent::UnknownComponent(Context* context)
        : Component(context)
        , useXML_(false)
    {
    }

    void UnknownComponent::SetTypeName(const String& typeName)
    {
        typeName_ = typeName;
        typeHash_ = typeName;
    }

    void UnknownComponent::SetType(StringHash typeHash)
    {
        typeName_ = GenerateNameFromType(typeHash);
        typeHash_ = typeHash;
    }
}
