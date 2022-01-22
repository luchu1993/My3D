//
// Created by luchu on 2022/1/13.
//

#pragma once

#include "IO/Serializer.h"
#include "IO/Deserializer.h"

namespace My3D
{

/// A common root class for objects that implement both Serializer and Deserializer.
class MY3D_API AbstractFile : public Deserializer, public Serializer
{
public:
    /// Construct
    AbstractFile() : Deserializer() { }
    /// Construct.
    explicit AbstractFile(unsigned int size) : Deserializer(size) { }
    /// Destruct.
    ~AbstractFile() override = default;
    /// Change the file name. Used by the resource system.
    /// @property
    virtual void SetName(const String& name) { name_ = name; }
    /// Return the file name.
    const String& GetName() const override { return name_; }

protected:
    /// File name.
    String name_;
};

}
