//
// Created by luchu on 2022/1/13.
//

#pragma once

#include "Core/Object.h"
#include "Container/Ptr.h"


namespace My3D
{
    class Deserializer;
    class Serializer;
    class JSONValue;
    class XMLElemenet;


    class MY3D_API Serializable : public Object
    {
    public:
        /// Construct
        explicit Serializable(Context* context);
        /// Destruct
        ~Serializable() override;

        /// Load from binary data. Return true if successful
        virtual bool Load(Deserializer& source);
        /// Save as binary data. Return true if successful
        virtual bool Save(Serializer& dest) const;
        /// Load from XML data. Return true if successful
        virtual bool LoadXML(XMLElemenet& source);
        /// Save as XML data. Return true if successful
        virtual bool SaveXML(XMLElemenet& dest) const;
        /// Load from JSON data. Return true if successful
        virtual bool LoadJSON(const JSONValue& source);
        /// Save as JSON data. Return true if successful
        virtual bool SaveJSON(JSONValue& dest) const;

        /// Apply attribute changes that can not be applied immediately.
        virtual void ApplyAttributes() { }

    private:
        /// Set instance-level default value.
        void SetInstanceDefault(const String& name, const Variant& defaultValue);
        /// Get instance-level default value.
        Variant GetInstanceDefault(const String& name) const;
        /// Attribute default value at each instance level
        UniquePtr<VariantMap> instanceDefaultValues_;
        /// When true, store the attribute value as instance's default value (internal use only).
        bool setInstanceDefault_;
        /// Temporary flag.
        bool temporary_;
    };
}
