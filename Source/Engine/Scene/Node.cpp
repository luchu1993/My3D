//
// Created by luchu on 2022/1/22.
//

#include "Core/Context.h"
#include "Scene/Node.h"


namespace My3D
{
    Node::Node(Context *context)
        : Animatable(context)
    {

    }

    Node::~Node()
    {

    }

    void Node::RegisterObject(Context *context)
    {
        context->RegisterFactory<Node>();

        MY3D_ACCESSOR_ATTRIBUTE("Is Enabled", IsEnabled, SetEnabled, bool, true, AM_DEFAULT);
    }

    bool Node::Load(Deserializer &source)
    {
        return false;
    }

    bool Node::Save(Serializer &dest) const
    {
        return false;
    }

    bool Node::LoadXML(const XMLElement &source)
    {
        return false;
    }

    bool Node::SaveXML(XMLElement &dest) const
    {
        return false;
    }

    void Node::ApplyAttributes()
    {

    }

    void Node::SetEnabled(bool enable)
    {

    }

    void Node::OnAttributeAnimationAdded()
    {
    }

    void Node::OnAttributeAnimationRemoved()
    {

    }
}