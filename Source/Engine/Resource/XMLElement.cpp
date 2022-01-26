//
// Created by luchu on 2022/1/25.
//

#include "pugixml.hpp"

#include "Core/Context.h"
#include "IO/Log.h"
#include "Resource/XMLFile.h"


namespace My3D
{
    XMLElement::XMLElement()
        : node_(nullptr)
        , xpathResultSet_(nullptr)
        , xpathNode_(nullptr)
        , xpathResultIndex_(0)
    {
    }

    XMLElement::XMLElement(XMLFile *file, pugi::xml_node_struct *node)
        : file_(file)
        , node_(node)
        , xpathResultSet_(nullptr)
        , xpathNode_(nullptr)
        , xpathResultIndex_(0)
    {
    }

    XMLElement::XMLElement(XMLFile *file, const XPathResultSet *resultSet, const pugi::xpath_node *xpathNode, unsigned int xpathResultIndex)
        : file_(file)
        , node_(nullptr)
        , xpathResultSet_(resultSet)
        , xpathNode_(resultSet ? xpathNode : (xpathNode ? new pugi::xpath_node(*xpathNode) : nullptr))
        , xpathResultIndex_(xpathResultIndex)
    {
    }

    XMLElement::XMLElement(const XMLElement &rhs)
        : file_(rhs.file_)
        , node_(rhs.node_)
        , xpathResultSet_(rhs.xpathResultSet_)
        , xpathNode_(rhs.xpathResultSet_ ? rhs.xpathNode_ : (rhs.xpathNode_ ? new pugi::xpath_node(*rhs.xpathNode_) : nullptr))
        , xpathResultIndex_(rhs.xpathResultIndex_)
    {
    }

    XMLElement::~XMLElement()
    {
        // XMLElement class takes the ownership of a single xpath_node object, so destruct it now
        if (!xpathResultSet_ && xpathNode_)
        {
            delete xpathNode_;
            xpathNode_ = nullptr;
        }
    }

    XMLElement& XMLElement::operator =(const XMLElement &rhs)
    {
        if (&rhs == this)
            return *this;

        XMLElement copy(rhs);
        Swap(copy);

        return *this;
    }

    XMLElement XMLElement::CreateChild(const String &name)
    {
        return XMLElement::CreateChild(name.CString());
    }

    XMLElement XMLElement::CreateChild(const char *name)
    {
        if (!file_ || (!node_ && !xpathNode_))
            return XMLElement();

        const pugi::xml_node& node = xpathNode_ ? xpathNode_->node() : pugi::xml_node(node_);
        pugi::xml_node child = const_cast<pugi::xml_node&>(node).append_child(name);
        return XMLElement(file_, child.internal_object());
    }

    XMLElement XMLElement::GetOrCreateChild(const String &name)
    {
        return XMLElement::GetOrCreateChild(name.CString());
    }

    XMLElement XMLElement::GetOrCreateChild(const char *name)
    {
        XMLElement child = GetChild(name);
        if (child.NotNull())
            return child;
        else
            return CreateChild(name);
    }

    bool XMLElement::AppendChild(XMLElement element, bool asCopy)
    {
        if (!element.file_ || (!element.node_ && !element.xpathNode_) || !file_ || (!node_ && !xpathNode_))
            return false;

        pugi::xml_node node = xpathNode_ ? xpathNode_->node() : pugi::xml_node(node_);
        const pugi::xml_node& child = element.xpathNode_ ? element.xpathNode_->node() : pugi::xml_node(element.node_);

        if (asCopy)
            node.append_copy(child);
        else
            node.append_move(child);

        return true;
    }

    bool XMLElement::Remove()
    {
        return GetParent().RemoveChild(*this);
    }

    bool XMLElement::RemoveChild(const XMLElement &element)
    {
        if (!element.file_ || (!element.node_ && !element.xpathNode_) || !file_ || (!node_ && !xpathNode_))
            return false;

        const pugi::xml_node& node = xpathNode_ ? xpathNode_->node() : pugi::xml_node(node_);
        const pugi::xml_node& child = element.xpathNode_ ? element.xpathNode_->node() : pugi::xml_node(element.node_);
        return const_cast<pugi::xml_node&>(node).remove_child(child);
    }

    bool XMLElement::RemoveChild(const String &name)
    {
        return RemoveChild(name.CString());
    }

    bool XMLElement::RemoveChild(const char *name)
    {
        if (!file_ || (!node_ && !xpathNode_))
            return false;

        const pugi::xml_node& node = xpathNode_ ? xpathNode_->node() : pugi::xml_node(node_);
        return const_cast<pugi::xml_node&>(node).remove_child(name);
    }

    XMLElement XMLElement::GetChild(const String &name) const
    {
        return GetChild(name.CString());
    }

    bool XMLElement::RemoveChildren(const String& name)
    {
        return RemoveChildren(name.CString());
    }

    bool XMLElement::RemoveChildren(const char *name)
    {
        if ((!file_ || !node_) && !xpathNode_)
            return false;

        const pugi::xml_node& node = xpathNode_ ? xpathNode_->node() : pugi::xml_node(node_);
        if (!String::CStringLength(name))
        {
            for (;;)
            {
                pugi::xml_node child = node.last_child();
                if (child.empty())
                    break;
                const_cast<pugi::xml_node&>(node).remove_child(child);
            }
        }
        else
        {
            for (;;)
            {
                pugi::xml_node child = node.child(name);
                if (child.empty())
                    break;
                const_cast<pugi::xml_node&>(node).remove_child(child);
            }
        }

        return true;
    }

    bool XMLElement::RemoveAttribute(const String &name)
    {
        return RemoveAttribute(name.CString());
    }

    bool XMLElement::RemoveAttribute(const char *name)
    {
        if (!file_ || (!node_ && !xpathNode_))
            return false;

        // If xpath_node contains just attribute, remove it regardless of the specified name
        if (xpathNode_ && xpathNode_->attribute())
            return xpathNode_->parent().remove_attribute(
                    xpathNode_->attribute());  // In attribute context, xpath_node's parent is the parent node of the attribute itself

        const pugi::xml_node& node = xpathNode_ ? xpathNode_->node() : pugi::xml_node(node_);
        return const_cast<pugi::xml_node&>(node).remove_attribute(node.attribute(name));
    }

    XMLElement XMLElement::SelectSingle(const String& query, pugi::xpath_variable_set* variables) const
    {
        if (!file_ || (!node_ && !xpathNode_))
            return XMLElement();

        const pugi::xml_node& node = xpathNode_ ? xpathNode_->node() : pugi::xml_node(node_);
        pugi::xpath_node result = node.select_single_node(query.CString(), variables);
        return XMLElement(file_, nullptr, &result, 0);
    }

    XMLElement XMLElement::SelectSinglePrepared(const XPathQuery& query) const
    {
        if (!file_ || (!node_ && !xpathNode_ && !query.GetXPathQuery()))
            return XMLElement();

        const pugi::xml_node& node = xpathNode_ ? xpathNode_->node() : pugi::xml_node(node_);
        pugi::xpath_node result = node.select_single_node(*query.GetXPathQuery());
        return XMLElement(file_, nullptr, &result, 0);
    }

    XPathResultSet XMLElement::Select(const String& query, pugi::xpath_variable_set* variables) const
    {
        if (!file_ || (!node_ && !xpathNode_))
            return XPathResultSet();

        const pugi::xml_node& node = xpathNode_ ? xpathNode_->node() : pugi::xml_node(node_);
        pugi::xpath_node_set result = node.select_nodes(query.CString(), variables);
        return XPathResultSet(file_, &result);
    }

    XPathResultSet XMLElement::SelectPrepared(const XPathQuery& query) const
    {
        if (!file_ || (!node_ && !xpathNode_ && query.GetXPathQuery()))
            return XPathResultSet();

        const pugi::xml_node& node = xpathNode_ ? xpathNode_->node() : pugi::xml_node(node_);
        pugi::xpath_node_set result = node.select_nodes(*query.GetXPathQuery());
        return XPathResultSet(file_, &result);
    }

    bool XMLElement::SetValue(const String& value)
    {
        return SetValue(value.CString());
    }

    bool XMLElement::SetValue(const char *value)
    {
        if (!file_ || (!node_ && !xpathNode_))
            return false;

        const pugi::xml_node& node = xpathNode_ ? xpathNode_->node() : pugi::xml_node(node_);

        // Search for existing value first
        for (pugi::xml_node child = node.first_child(); child; child = child.next_sibling())
        {
            if (child.type() == pugi::node_pcdata)
                return const_cast<pugi::xml_node&>(child).set_value(value);
        }

        // If no previous value found, append new
        return const_cast<pugi::xml_node&>(node).append_child(pugi::node_pcdata).set_value(value);
    }

    bool XMLElement::SetAttribute(const String& name, const String& value)
    {
        return SetAttribute(name.CString(), value.CString());
    }

    bool XMLElement::SetAttribute(const char* name, const char* value)
    {
        if (!file_ || (!node_ && !xpathNode_))
            return false;

        // If xpath_node contains just attribute, set its value regardless of the specified name
        if (xpathNode_ && xpathNode_->attribute())
            return xpathNode_->attribute().set_value(value);

        const pugi::xml_node& node = xpathNode_ ? xpathNode_->node() : pugi::xml_node(node_);
        pugi::xml_attribute attr = node.attribute(name);
        if (attr.empty())
            attr = const_cast<pugi::xml_node&>(node).append_attribute(name);
        return attr.set_value(value);
    }

    bool XMLElement::SetAttribute(const String& value)
    {
        return SetAttribute(value.CString());
    }

    bool XMLElement::SetAttribute(const char* value)
    {
        // If xpath_node contains just attribute, set its value
        return xpathNode_ && xpathNode_->attribute() && xpathNode_->attribute().set_value(value);
    }

    bool XMLElement::SetBool(const String& name, bool value)
    {
        return SetAttribute(name, String(value));
    }

    bool XMLElement::SetBoundingBox(const BoundingBox& value)
    {
        if (!SetVector3("min", value.min_))
            return false;
        return SetVector3("max", value.max_);
    }

    bool XMLElement::SetColor(const String& name, const Color& value)
    {
        return SetAttribute(name, value.ToString());
    }

    bool XMLElement::SetFloat(const String& name, float value)
    {
        return SetAttribute(name, String(value));
    }

    bool XMLElement::SetDouble(const String& name, double value)
    {
        return SetAttribute(name, String(value));
    }

    bool XMLElement::SetUInt(const String& name, unsigned value)
    {
        return SetAttribute(name, String(value));
    }

    bool XMLElement::SetInt(const String& name, int value)
    {
        return SetAttribute(name, String(value));
    }

    bool XMLElement::SetUInt64(const String& name, unsigned long long value)
    {
        return SetAttribute(name, String(value));
    }

    bool XMLElement::SetInt64(const String& name, long long value)
    {
        return SetAttribute(name, String(value));
    }

    bool XMLElement::SetIntRect(const String& name, const IntRect& value)
    {
        return SetAttribute(name, value.ToString());
    }

    bool XMLElement::SetIntVector2(const String& name, const IntVector2& value)
    {
        return SetAttribute(name, value.ToString());
    }

    bool XMLElement::SetIntVector3(const String& name, const IntVector3& value)
    {
        return SetAttribute(name, value.ToString());
    }

    bool XMLElement::SetRect(const String& name, const Rect& value)
    {
        return SetAttribute(name, value.ToString());
    }

    bool XMLElement::SetQuaternion(const String& name, const Quaternion& value)
    {
        return SetAttribute(name, value.ToString());
    }

    bool XMLElement::SetString(const String& name, const String& value)
    {
        return SetAttribute(name, value);
    }

    bool XMLElement::SetVector2(const String& name, const Vector2& value)
    {
        return SetAttribute(name, value.ToString());
    }

    bool XMLElement::SetVector3(const String& name, const Vector3& value)
    {
        return SetAttribute(name, value.ToString());
    }

    bool XMLElement::SetVector4(const String& name, const Vector4& value)
    {
        return SetAttribute(name, value.ToString());
    }

    String XMLElement::GetName() const
    {
        if ((!file_ || !node_) && !xpathNode_)
            return String();

        // If xpath_node contains just attribute, return its name instead
        if (xpathNode_ && xpathNode_->attribute())
            return String(xpathNode_->attribute().name());

        const pugi::xml_node& node = xpathNode_ ? xpathNode_->node() : pugi::xml_node(node_);
        return String(node.name());
    }

    bool XMLElement::HasChild(const String& name) const
    {
        return HasChild(name.CString());
    }

    bool XMLElement::HasChild(const char* name) const
    {
        if (!file_ || (!node_ && !xpathNode_))
            return false;

        const pugi::xml_node& node = xpathNode_ ? xpathNode_->node() : pugi::xml_node(node_);
        return !node.child(name).empty();
    }

    XMLElement XMLElement::GetChild(const char *name) const
    {
        if (!file_ || (!node_ && !xpathNode_))
            return XMLElement();

        const pugi::xml_node& node = xpathNode_ ? xpathNode_->node() : pugi::xml_node(node_);
        if (!String::CStringLength(name))
            return XMLElement(file_, node.first_child().internal_object());
        else
            return XMLElement(file_, node.child(name).internal_object());
    }

    XMLElement XMLElement::GetNext(const String& name) const
    {
        return GetNext(name.CString());
    }

    XMLElement XMLElement::GetNext(const char* name) const
    {
        if (!file_ || (!node_ && !xpathNode_))
            return XMLElement();

        const pugi::xml_node& node = xpathNode_ ? xpathNode_->node() : pugi::xml_node(node_);
        if (!String::CStringLength(name))
            return XMLElement(file_, node.next_sibling().internal_object());
        else
            return XMLElement(file_, node.next_sibling(name).internal_object());
    }

    XMLElement XMLElement::GetParent() const
    {
        if (!file_ || (!node_ && !xpathNode_))
            return XMLElement();

        const pugi::xml_node& node = xpathNode_ ? xpathNode_->node() : pugi::xml_node(node_);
        return XMLElement(file_, node.parent().internal_object());
    }

    unsigned XMLElement::GetNumAttributes() const
    {
        if (!file_ || (!node_ && !xpathNode_))
            return 0;

        const pugi::xml_node& node = xpathNode_ ? xpathNode_->node() : pugi::xml_node(node_);
        unsigned ret = 0;

        pugi::xml_attribute attr = node.first_attribute();
        while (!attr.empty())
        {
            ++ret;
            attr = attr.next_attribute();
        }

        return ret;
    }

    bool XMLElement::HasAttribute(const String& name) const
    {
        return HasAttribute(name.CString());
    }

    bool XMLElement::HasAttribute(const char* name) const
    {
        if (!file_ || (!node_ && !xpathNode_))
            return false;

        // If xpath_node contains just attribute, check against it
        if (xpathNode_ && xpathNode_->attribute())
            return String(xpathNode_->attribute().name()) == name;

        const pugi::xml_node& node = xpathNode_ ? xpathNode_->node() : pugi::xml_node(node_);
        return !node.attribute(name).empty();
    }

    String XMLElement::GetValue() const
    {
        if (!file_ || (!node_ && !xpathNode_))
            return String::EMPTY;

        const pugi::xml_node& node = xpathNode_ ? xpathNode_->node() : pugi::xml_node(node_);
        return String(node.child_value());
    }

    bool XMLElement::GetBool(const String& name) const
    {
        return ToBool(GetAttribute(name));
    }

    BoundingBox XMLElement::GetBoundingBox() const
    {
        BoundingBox ret;

        ret.min_ = GetVector3("min");
        ret.max_ = GetVector3("max");
        return ret;
    }

    Color XMLElement::GetColor(const String& name) const
    {
        return ToColor(GetAttribute(name));
    }

    float XMLElement::GetFloat(const String& name) const
    {
        return ToFloat(GetAttribute(name));
    }

    double XMLElement::GetDouble(const String& name) const
    {
        return ToDouble(GetAttribute(name));
    }

    unsigned XMLElement::GetUInt(const String& name) const
    {
        return ToUInt(GetAttribute(name));
    }

    int XMLElement::GetInt(const String& name) const
    {
        return ToInt(GetAttribute(name));
    }

    unsigned long long XMLElement::GetUInt64(const String& name) const
    {
        return ToUInt64(GetAttribute(name));
    }

    long long XMLElement::GetInt64(const String& name) const
    {
        return ToInt64(GetAttribute(name));
    }

    IntRect XMLElement::GetIntRect(const String& name) const
    {
        return ToIntRect(GetAttribute(name));
    }

    IntVector2 XMLElement::GetIntVector2(const String& name) const
    {
        return ToIntVector2(GetAttribute(name));
    }

    IntVector3 XMLElement::GetIntVector3(const String& name) const
    {
        return ToIntVector3(GetAttribute(name));
    }

    Quaternion XMLElement::GetQuaternion(const String& name) const
    {
        return ToQuaternion(GetAttribute(name));
    }

    Vector2 XMLElement::GetVector2(const String& name) const
    {
        return ToVector2(GetAttribute(name));
    }

    Vector3 XMLElement::GetVector3(const String& name) const
    {
        return ToVector3(GetAttribute(name));
    }

    Vector4 XMLElement::GetVector4(const String& name) const
    {
        return ToVector4(GetAttribute(name));
    }

    Rect XMLElement::GetRect(const String& name) const
    {
        return ToRect(GetAttribute(name));
    }

    String XMLElement::GetAttribute(const String& name) const
    {
        return String(GetAttributeCString(name.CString()));
    }

    String XMLElement::GetAttribute(const char* name) const
    {
        return String(GetAttributeCString(name));
    }

    const char* XMLElement::GetAttributeCString(const char* name) const
    {
        if (!file_ || (!node_ && !xpathNode_))
            return nullptr;

        // If xpath_node contains just attribute, return it regardless of the specified name
        if (xpathNode_ && xpathNode_->attribute())
            return xpathNode_->attribute().value();

        const pugi::xml_node& node = xpathNode_ ? xpathNode_->node() : pugi::xml_node(node_);
        return node.attribute(name).value();
    }

    String XMLElement::GetAttributeLower(const String& name) const
    {
        return GetAttribute(name).ToLower();
    }

    String XMLElement::GetAttributeLower(const char* name) const
    {
        return String(GetAttribute(name)).ToLower();
    }

    String XMLElement::GetAttributeUpper(const String& name) const
    {
        return GetAttribute(name).ToUpper();
    }

    String XMLElement::GetAttributeUpper(const char* name) const
    {
        return String(GetAttribute(name)).ToUpper();
    }

    Vector<String> XMLElement::GetAttributeNames() const
    {
        if (!file_ || (!node_ && !xpathNode_))
            return Vector<String>();

        const pugi::xml_node& node = xpathNode_ ? xpathNode_->node() : pugi::xml_node(node_);
        Vector<String> ret;

        pugi::xml_attribute attr = node.first_attribute();
        while (!attr.empty())
        {
            ret.Push(String(attr.name()));
            attr = attr.next_attribute();
        }

        return ret;
    }

    XMLFile* XMLElement::GetFile() const
    {
        return file_;
    }

    bool XMLElement::IsNull() const
    {
        return !NotNull();
    }

    bool XMLElement::NotNull() const
    {
        return node_ || (xpathNode_ && !xpathNode_->operator !());
    }

    XMLElement::operator bool() const
    {
        return NotNull();
    }

    XMLElement XMLElement::NextResult() const
    {
        if (!xpathResultSet_ || !xpathNode_)
            return XMLElement();

        return xpathResultSet_->operator [](++xpathResultIndex_);
    }

    void XMLElement::Swap(XMLElement& rhs)
    {
        file_.Swap(rhs.file_);
        My3D::Swap(node_, rhs.node_);
        My3D::Swap(xpathResultSet_, rhs.xpathResultSet_);
        My3D::Swap(xpathNode_, rhs.xpathNode_);
        My3D::Swap(xpathResultIndex_, rhs.xpathResultIndex_);
    }

    XPathResultSet::XPathResultSet()
        : resultSet_(nullptr)
    {
    }

    XPathResultSet::XPathResultSet(XMLFile* file, pugi::xpath_node_set* resultSet)
        : file_(file)
        , resultSet_(resultSet ? new pugi::xpath_node_set(resultSet->begin(), resultSet->end()) : nullptr)
    {
        // Sort the node set in forward document order
        if (resultSet_)
            resultSet_->sort();
    }

    XPathResultSet::XPathResultSet(const XPathResultSet& rhs) :
            file_(rhs.file_),
            resultSet_(rhs.resultSet_ ? new pugi::xpath_node_set(rhs.resultSet_->begin(), rhs.resultSet_->end()) : nullptr)
    {
    }

    XPathResultSet::~XPathResultSet()
    {
        delete resultSet_;
        resultSet_ = nullptr;
    }

    XMLElement XPathResultSet::operator [](unsigned index) const
    {
        if (!resultSet_)
            MY3D_LOGERRORF("Could not return result at index: %u. Most probably this is caused by the XPathResultSet not being stored in a lhs variable.", index);

        return resultSet_ && index < Size() ? XMLElement(file_, this, &resultSet_->operator [](index), index) : XMLElement();
    }

    unsigned XPathResultSet::Size() const
    {
        return resultSet_ ? (unsigned)resultSet_->size() : 0;
    }

    bool XPathResultSet::Empty() const
    {
        return resultSet_ ? resultSet_->empty() : true;
    }

    XPathQuery::XPathQuery() = default;

    XPathQuery::~XPathQuery() = default;
}