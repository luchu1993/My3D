//
// Created by luchu on 2022/1/31.
//

#include "Scene/Serializable.h"
#include "IO/Log.h"
#include "Core/Context.h"
#include "IO/Deserializer.h"
#include "IO/Serializer.h"
#include "Resource/XMLElement.h"
#include "Scene/SceneEvents.h"


namespace My3D
{
    Serializable::Serializable(Context *context)
        : Object(context)
        , setInstanceDefault_(false)
        , temporary_(false)
    {
    }

    Serializable::~Serializable() = default;

    void Serializable::OnSetAttribute(const AttributeInfo &attr, const Variant &src)
    {
        if (setInstanceDefault_)
            SetInstanceDefault(attr.name_, src);

        // Check for accessor function mode
        if (attr.accessor_)
        {
            attr.accessor_->Set(this, src);
            return;
        }

        // Get the destination address
        assert(attr.ptr_);
        void* dest = attr.ptr_;

        switch (attr.type_)
        {
        case VAR_INT:
            // If enum type, use the low 8 bits only
            if (attr.enumNames_)
                *(reinterpret_cast<unsigned char*>(dest)) = src.GetInt();
            else
                *(reinterpret_cast<int*>(dest)) = src.GetInt();
        break;

        case VAR_INT64:
            *(reinterpret_cast<long long*>(dest)) = src.GetInt64();
            break;

        case VAR_BOOL:
            *(reinterpret_cast<bool*>(dest)) = src.GetBool();
            break;

        case VAR_FLOAT:
            *(reinterpret_cast<float*>(dest)) = src.GetFloat();
            break;

        case VAR_VECTOR2:
            *(reinterpret_cast<Vector2*>(dest)) = src.GetVector2();
            break;

        case VAR_VECTOR3:
            *(reinterpret_cast<Vector3*>(dest)) = src.GetVector3();
            break;

        case VAR_VECTOR4:
            *(reinterpret_cast<Vector4*>(dest)) = src.GetVector4();
            break;

        case VAR_QUATERNION:
            *(reinterpret_cast<Quaternion*>(dest)) = src.GetQuaternion();
            break;

        case VAR_COLOR:
            *(reinterpret_cast<Color*>(dest)) = src.GetColor();
            break;

        case VAR_STRING:
            *(reinterpret_cast<String*>(dest)) = src.GetString();
            break;

        case VAR_BUFFER:
            *(reinterpret_cast<PODVector<unsigned char>*>(dest)) = src.GetBuffer();
            break;

        case VAR_RESOURCEREF:
            *(reinterpret_cast<ResourceRef*>(dest)) = src.GetResourceRef();
            break;

        case VAR_RESOURCEREFLIST:
            *(reinterpret_cast<ResourceRefList*>(dest)) = src.GetResourceRefList();
            break;

        case VAR_VARIANTVECTOR:
            *(reinterpret_cast<VariantVector*>(dest)) = src.GetVariantVector();
            break;

        case VAR_STRINGVECTOR:
            *(reinterpret_cast<StringVector*>(dest)) = src.GetStringVector();
            break;

        case VAR_VARIANTMAP:
            *(reinterpret_cast<VariantMap*>(dest)) = src.GetVariantMap();
            break;

        case VAR_INTRECT:
            *(reinterpret_cast<IntRect*>(dest)) = src.GetIntRect();
            break;

        case VAR_INTVECTOR2:
            *(reinterpret_cast<IntVector2*>(dest)) = src.GetIntVector2();
            break;

        case VAR_INTVECTOR3:
            *(reinterpret_cast<IntVector3*>(dest)) = src.GetIntVector3();
            break;

        case VAR_DOUBLE:
            *(reinterpret_cast<double*>(dest)) = src.GetDouble();
            break;

        default:
            MY3D_LOGERROR("Unsupported attribute type for OnSetAttribute()");
            return;
        }

        // If it is a network attribute then mark it for next network update
        if (attr.mode_ & AM_NET)
            MarkNetworkUpdate();
    }

    void Serializable::OnGetAttribute(const AttributeInfo &attr, Variant &dest) const
    {
        // Check for accessor function mode
        if (attr.accessor_)
        {
            attr.accessor_->Get(this, dest);
            return;
        }

        // Get the source address
        assert(attr.ptr_);
        const void* src = attr.ptr_;

        switch (attr.type_)
        {
        case VAR_INT:
            // If enum type, use the low 8 bits only
            if (attr.enumNames_)
                dest = *(reinterpret_cast<const unsigned char*>(src));
            else
                dest = *(reinterpret_cast<const int*>(src));
            break;
        case VAR_INT64:
            dest = *(reinterpret_cast<const long long*>(src));
            break;

        case VAR_BOOL:
            dest = *(reinterpret_cast<const bool*>(src));
            break;

        case VAR_FLOAT:
            dest = *(reinterpret_cast<const float*>(src));
            break;

        case VAR_VECTOR2:
            dest = *(reinterpret_cast<const Vector2*>(src));
            break;

        case VAR_VECTOR3:
            dest = *(reinterpret_cast<const Vector3*>(src));
            break;

        case VAR_VECTOR4:
            dest = *(reinterpret_cast<const Vector4*>(src));
            break;

        case VAR_QUATERNION:
            dest = *(reinterpret_cast<const Quaternion*>(src));
            break;

        case VAR_COLOR:
            dest = *(reinterpret_cast<const Color*>(src));
            break;

        case VAR_STRING:
            dest = *(reinterpret_cast<const String*>(src));
            break;

        case VAR_BUFFER:
            dest = *(reinterpret_cast<const PODVector<unsigned char>*>(src));
            break;

        case VAR_RESOURCEREF:
            dest = *(reinterpret_cast<const ResourceRef*>(src));
            break;

        case VAR_RESOURCEREFLIST:
            dest = *(reinterpret_cast<const ResourceRefList*>(src));
            break;

        case VAR_VARIANTVECTOR:
            dest = *(reinterpret_cast<const VariantVector*>(src));
            break;

        case VAR_STRINGVECTOR:
            dest = *(reinterpret_cast<const StringVector*>(src));
            break;

        case VAR_VARIANTMAP:
            dest = *(reinterpret_cast<const VariantMap*>(src));
            break;

        case VAR_INTRECT:
            dest = *(reinterpret_cast<const IntRect*>(src));
            break;

        case VAR_INTVECTOR2:
            dest = *(reinterpret_cast<const IntVector2*>(src));
            break;

        case VAR_INTVECTOR3:
            dest = *(reinterpret_cast<const IntVector3*>(src));
            break;

        case VAR_DOUBLE:
            dest = *(reinterpret_cast<const double*>(src));
            break;

        default:
            MY3D_LOGERROR("Unsupported attribute type for OnGetAttribute()");
            return;
        }
    }

    const Vector<AttributeInfo>* Serializable::GetAttributes() const
    {
        return context_->GetAttributes(GetType());
    }

    const Vector<AttributeInfo>* Serializable::GetNetworkAttributes() const
    {
        return context_->GetNetworkAttributes(GetType());
    }

    bool Serializable::Load(Deserializer& source)
    {
        const Vector<AttributeInfo>* attributes = GetAttributes();
        if (!attributes)
            return true;

        for (unsigned i = 0; i < attributes->Size(); ++i)
        {
            const AttributeInfo& attr = attributes->At(i);
            if (!(attr.mode_ & AM_FILE))
                continue;

            if (source.IsEof())
            {
                MY3D_LOGERROR("Could not load " + GetTypeName() + ", stream not open or at end");
                return false;
            }

            Variant varValue = source.ReadVariant(attr.type_);
            OnSetAttribute(attr, varValue);
        }

        return true;
    }

    bool Serializable::Save(Serializer& dest) const
    {
        const Vector<AttributeInfo>* attributes = GetAttributes();
        if (!attributes)
            return true;

        Variant value;
        for (unsigned i = 0; i < attributes->Size(); ++i)
        {
            const AttributeInfo& attr = attributes->At(i);
            if (!(attr.mode_ & AM_FILE) || (attr.mode_ & AM_FILEREADONLY) == AM_FILEREADONLY)
                continue;

            OnGetAttribute(attr, value);

            if (!dest.WriteVariantData(value))
            {
                MY3D_LOGERROR("Could not save " + GetTypeName() + ", writing to stream failed");
                return false;
            }
        }

        return true;
    }

    bool Serializable::LoadXML(const XMLElement& source)
    {
        if (source.IsNull())
        {
            MY3D_LOGERROR("Could not load " + GetTypeName() + ", null source element");
            return false;
        }

        const Vector<AttributeInfo>* attributes = GetAttributes();
        if (!attributes)
            return true;

        XMLElement attrElem = source.GetChild("attribute");
        unsigned startIndex = 0;

        while (attrElem)
        {
            String name = attrElem.GetAttribute("name");
            unsigned i = startIndex;
            unsigned attempts = attributes->Size();

            while (attempts)
            {
                const AttributeInfo& attr = attributes->At(i);
                if ((attr.mode_ & AM_FILE) && !attr.name_.Compare(name, true))
                {
                    Variant varValue;
                    // If enums specified, do enum lookup and int assignment. Otherwise assign the variant directly
                    if (attr.enumNames_)
                    {
                        String value = attrElem.GetAttribute("value");
                        bool enumFound = false;
                        int enumValue = 0;
                        const char** enumPtr = attr.enumNames_;
                        while (*enumPtr)
                        {
                            if (!value.Compare(*enumPtr, false))
                            {
                                enumFound = true;
                                break;
                            }
                            ++enumPtr;
                            ++enumValue;
                        }

                        if (enumFound)
                            varValue = enumValue;
                        else
                            MY3D_LOGWARNING("Unknown enum value " + value + " in attribute " + attr.name_);
                    }
                    else
                        varValue = attrElem.GetVariantValue(attr.type_);

                    if (!varValue.IsEmpty())
                        OnSetAttribute(attr, varValue);

                    startIndex = (i + 1) % attributes->Size();
                    break;
                }
                else
                {
                    i = (i + 1) % attributes->Size();
                    --attempts;
                }
            }

            if (attributes)
                MY3D_LOGWARNING("Unknown attribute " + name + " in XML data");

            attrElem = attrElem.GetNext("attribute");
        }

        return true;
    }

    bool Serializable::SaveXML(XMLElement &dest) const
    {
        if (dest.IsNull())
        {
            MY3D_LOGERROR("Could not save " + GetTypeName() + ", null destination element");
            return false;
        }

        const Vector<AttributeInfo>* attributes = GetAttributes();
        if (!attributes)
            return true;

        Variant value;

        for (unsigned i = 0; i < attributes->Size(); ++i)
        {
            const AttributeInfo& attr =  attributes->At(i);
            if (!(attr.mode_ & AM_FILE) || (attr.mode_ & AM_FILEREADONLY) == AM_FILEREADONLY)
                continue;

            OnGetAttribute(attr, value);
            Variant defaultValue(GetAttributeDefault(i));

            // In XML serialization default values can be skipped. This will make the file easier to read or edit manually
            if (value == defaultValue && !SaveDefaultAttributes())
                continue;

            XMLElement attrElem = dest.CreateChild("attribute");
            attrElem.SetAttribute("name", attr.name_);
            // If enums specified, set as an enum string. Otherwise set directly as a Variant
            if (attr.enumNames_)
            {
                int enumValue = value.GetInt();
                attrElem.SetAttribute("value", attr.enumNames_[enumValue]);
            }
            else
                attrElem.SetVariantValue(value);
        }

        return true;
    }

    bool Serializable::SetAttribute(unsigned int index, const Variant& value)
    {
        const Vector<AttributeInfo>* attributes = GetAttributes();
        if (!attributes)
        {
            MY3D_LOGERROR(GetTypeName() + " has no attributes");
            return false;
        }

        if (index >= attributes->Size())
        {
            MY3D_LOGERROR("Attribute index out of bounds");
            return false;
        }

        const AttributeInfo& attr = attributes->At(index);

        // Check that the new value's type matches the attribute type
        if (value.GetType() == attr.type_)
        {
            OnSetAttribute(attr, value);
            return true;
        }
        else
        {
            MY3D_LOGERROR("Could not set attribute " + attr.name_ + ": expected type " + Variant::GetTypeName(attr.type_) +
                            " but got " + value.GetTypeName());
            return false;
        }
    }

    bool Serializable::SetAttribute(const String& name, const Variant& value)
    {
        const Vector<AttributeInfo>* attributes = GetAttributes();
        if (!attributes)
        {
            MY3D_LOGERROR(GetTypeName() + " has no attributes");
            return false;
        }

        for (Vector<AttributeInfo>::ConstIterator i = attributes->Begin(); i != attributes->End(); ++i)
        {
            if (!i->name_.Compare(name, true))
            {
                // Check that the new value's type matches the attribute type
                if (value.GetType() == i->type_)
                {
                    OnSetAttribute(*i, value);
                    return true;
                }
                else
                {
                    MY3D_LOGERROR("Could not set attribute " + i->name_ + ": expected type " + Variant::GetTypeName(i->type_)
                                    + " but got " + value.GetTypeName());
                    return false;
                }
            }
        }

        MY3D_LOGERROR("Could not find attribute " + name + " in " + GetTypeName());
        return false;
    }

    void Serializable::ResetToDefault()
    {
        const Vector<AttributeInfo>* attributes = GetAttributes();
        if (!attributes)
            return;

        for (unsigned i = 0; i < attributes->Size(); ++i)
        {
            const AttributeInfo& attr = attributes->At(i);
            if (attr.mode_ & (AM_NOEDIT | AM_NODEID | AM_COMPONENTID | AM_NODEIDVECTOR))
                continue;

            Variant defaultValue = GetInstanceDefault(attr.name_);
            if (defaultValue.IsEmpty())
                defaultValue = attr.defaultValue_;

            OnSetAttribute(attr, defaultValue);
        }
    }

    void Serializable::RemoveInstanceDefault()
    {
        instanceDefaultValues_.Reset();
    }

    void Serializable::SetTemporary(bool enable)
    {
        if (enable != temporary_)
        {
            temporary_ = enable;

            using namespace TemporaryChanged;

            VariantMap& eventData = GetEventDataMap();
            eventData[P_SERIALIZABLE] = this;

            SendEvent(E_TEMPORARYCHANGED, eventData);
        }
    }

    Variant Serializable::GetAttribute(unsigned int index) const
    {
        Variant ret;

        const Vector<AttributeInfo>* attributes = GetAttributes();
        if (!attributes)
        {
            MY3D_LOGERROR(GetTypeName() + " has no attributes");
            return ret;
        }

        if (index >= attributes->Size())
        {
            MY3D_LOGERROR("Attribute index out of bounds");
            return ret;
        }

        OnGetAttribute(attributes->At(index), ret);
        return ret;
    }

    Variant Serializable::GetAttribute(const String &name) const
    {
        Variant ret;

        const Vector<AttributeInfo>* attributes = GetAttributes();
        if (!attributes)
        {
            MY3D_LOGERROR(GetTypeName() + " has no attributes");
            return ret;
        }

        for (auto i = attributes->Begin(); i != attributes->End(); ++i)
        {
            if (!i->name_.Compare(name, true))
            {
                OnGetAttribute(*i, ret);
                return ret;
            }
        }

        MY3D_LOGERROR("Could not find attribute " + name + " in " + GetTypeName());
        return ret;
    }

    Variant Serializable::GetAttributeDefault(unsigned int index) const
    {
        const Vector<AttributeInfo>* attributes = GetAttributes();
        if (!attributes)
        {
            MY3D_LOGERROR(GetTypeName() + " has no attributes");
            return Variant::EMPTY;
        }

        if (index >= attributes->Size())
        {
            MY3D_LOGERROR("Attribute index out of bounds");
            return Variant::EMPTY;
        }

        AttributeInfo attr = attributes->At(index);
        Variant defaultValue = GetInstanceDefault(attr.name_);
        return defaultValue.IsEmpty() ? attr.defaultValue_ : defaultValue;
    }

    Variant Serializable::GetAttributeDefault(const String& name) const
    {
        Variant defaultValue = GetInstanceDefault(name);
        if (!defaultValue.IsEmpty())
            return defaultValue;

        const Vector<AttributeInfo>* attributes = GetAttributes();
        if (!attributes)
        {
            MY3D_LOGERROR(GetTypeName() + " has no attributes");
            return Variant::EMPTY;
        }

        for (Vector<AttributeInfo>::ConstIterator i = attributes->Begin(); i != attributes->End(); ++i)
        {
            if (!i->name_.Compare(name, true))
                return i->defaultValue_;
        }

        MY3D_LOGERROR("Could not find attribute " + name + " in " + GetTypeName());
        return Variant::EMPTY;
    }

    unsigned Serializable::GetNumAttributes() const
    {
        const Vector<AttributeInfo>* attributes = GetAttributes();
        return attributes ? attributes->Size() : 0;
    }

    unsigned Serializable::GetNumNetworkAttributes() const
    {
        const Vector<AttributeInfo>* attributes = context_->GetNetworkAttributes(GetType());
        return attributes ? attributes->Size() : 0;
    }

    void Serializable::SetInstanceDefault(const String& name, const Variant& defaultValue)
    {
        // Allocate the instance level default value
        if (!instanceDefaultValues_)
            instanceDefaultValues_ = new VariantMap();
        instanceDefaultValues_->operator [](name) = defaultValue;
    }

    Variant Serializable::GetInstanceDefault(const String& name) const
    {
        if (instanceDefaultValues_)
        {
            VariantMap::ConstIterator i = instanceDefaultValues_->Find(name);
            if (i != instanceDefaultValues_->End())
                return i->second_;
        }

        return Variant::EMPTY;
    }
}