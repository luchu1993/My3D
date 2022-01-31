//
// Created by luchu on 2022/1/15.
//

#pragma once

#include "Core/Object.h"
#include "Container/Ptr.h"
#include "Core/Attribute.h"


namespace My3D
{
    class Deserializer;
    class Serializer;
    class JSONValue;
    class XMLElement;


    /// Base class for objects with automatic serialization through attributes.
    class MY3D_API Serializable : public Object
    {
        MY3D_OBJECT(Serializable, Object)
    public:
        /// Construct
        explicit Serializable(Context* context);
        /// Destruct
        ~Serializable() override;
        /// Handle attribute write access. Default implementation writes to the variable at offset, or invokes the set accessor.
        virtual void OnSetAttribute(const AttributeInfo& attr, const Variant& src);
        /// Handle attribute read access. Default implementation reads the variable at offset, or invokes the get accessor.
        virtual void OnGetAttribute(const AttributeInfo& attr, Variant& dest) const;
        /// Return attribute descriptions, or null if none defined.
        virtual const Vector<AttributeInfo>* GetAttributes() const;
        /// Return network replication attribute descriptions, or null if none defined.
        virtual const Vector<AttributeInfo>* GetNetworkAttributes() const;
        /// Load from binary data. Return true if successful
        virtual bool Load(Deserializer& source);
        /// Save as binary data. Return true if successful
        virtual bool Save(Serializer& dest) const;
        /// Load from XML data. Return true if successful
        virtual bool LoadXML(const XMLElement& source);
        /// Save as XML data. Return true if successful
        virtual bool SaveXML(XMLElement& dest) const;
        /// Apply attribute changes that can not be applied immediately.
        virtual void ApplyAttributes() { }
        /// Return whether should save default-valued attributes into XML. Default false.
        virtual bool SaveDefaultAttributes() const { return false; }
        /// Mark for attribute check on the next network update.
        virtual void MarkNetworkUpdate() { }

        /// @property{get_attributes}
        Variant GetAttribute(unsigned index) const;
        /// Return attribute value by name. Return empty if not found.
        Variant GetAttribute(const String& name) const;
        /// Return attribute default value by index. Return empty if illegal index.
        /// @property{get_attributeDefaults}
        Variant GetAttributeDefault(unsigned index) const;
        /// Return attribute default value by name. Return empty if not found.
        Variant GetAttributeDefault(const String& name) const;
        /// Return number of attributes.
        /// @property
        unsigned GetNumAttributes() const;
        /// Return number of network replication attributes.
        unsigned GetNumNetworkAttributes() const;

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
