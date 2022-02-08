//
// Created by luchu on 2022/2/8.
//

#include "Scene/Component.h"
#include "Core/Context.h"
#include "Scene/Scene.h"
#include "IO/Serializer.h"
#include "Resource/XMLElement.h"
#include "Scene/SceneEvents.h"


namespace My3D
{
    const char* autoRemoveModeNames[] = {
        "Disabled",
        "Component",
        "Node",
        nullptr
    };

    Component::Component(Context* context)
        : Animatable(context)
        , node_(nullptr)
        , id_(0)
        , networkUpdate_(false)
        , enabled_(true)
    {
    }

    Component::~Component() = default;

    bool Component::Save(Serializer& dest) const
    {
        // Write type and ID
        if (!dest.WriteStringHash(GetType()))
            return false;
        if (!dest.WriteUInt(id_))
            return false;

        // Write attributes
        return Animatable::Save(dest);
    }

    bool Component::SaveXML(XMLElement& dest) const
    {
        // Write type and ID
        if (!dest.SetString("type", GetTypeName()))
            return false;
        if (!dest.SetUInt("id", id_))
            return false;

        // Write attributes
        return Animatable::SaveXML(dest);
    }

    void Component::MarkNetworkUpdate()
    {
        if (!networkUpdate_ && IsReplicated())
        {
            Scene* scene = GetScene();
            if (scene)
            {
                scene->MarkNetworkUpdate(this);
                networkUpdate_ = true;
            }
        }
    }

    void Component::GetDependencyNodes(PODVector<Node*>& dest)
    {
    }

    void Component::DrawDebugGeometry(DebugRenderer* debug, bool depthTest)
    {
    }

    void Component::SetEnabled(bool enable)
    {
        if (enable != enabled_)
        {
            enabled_ = enable;
            OnSetEnabled();
            MarkNetworkUpdate();

            // Send change event for the component
            Scene* scene = GetScene();
            if (scene)
            {
                using namespace ComponentEnabledChanged;

                VariantMap& eventData = GetEventDataMap();
                eventData[P_SCENE] = scene;
                eventData[P_NODE] = node_;
                eventData[P_COMPONENT] = this;

                scene->SendEvent(E_COMPONENTENABLEDCHANGED, eventData);
            }
        }
    }

    void Component::Remove()
    {
        if (node_)
            node_->RemoveComponent(this);
    }

    bool Component::IsReplicated() const
    {
        return Scene::IsReplicatedID(id_);
    }

    Scene* Component::GetScene() const
    {
        return node_ ? node_->GetScene() : nullptr;
    }

    void Component::OnNodeSet(Node* node)
    {
    }

    void Component::OnSceneSet(Scene* scene)
    {
    }

    void Component::OnMarkedDirty(Node* node)
    {
    }

    void Component::OnNodeSetEnabled(Node* node)
    {
    }

    void Component::SetID(unsigned id)
    {
        id_ = id;
    }

    void Component::SetNode(Node* node)
    {
        node_ = node;
        OnNodeSet(node_);
    }

    Component* Component::GetComponent(StringHash type) const
    {
        return node_ ? node_->GetComponent(type) : nullptr;
    }

    bool Component::IsEnabledEffective() const
    {
        return enabled_ && node_ && node_->IsEnabled();
    }

    void Component::GetComponents(PODVector<Component*>& dest, StringHash type) const
    {
        if (node_)
            node_->GetComponents(dest, type);
        else
            dest.Clear();
    }

    void Component::OnAttributeAnimationAdded()
    {
        if (attributeAnimationInfos_.Size() == 1)
            SubscribeToEvent(GetScene(), E_ATTRIBUTEANIMATIONUPDATE, MY3D_HANDLER(Component, HandleAttributeAnimationUpdate));
    }

    void Component::OnAttributeAnimationRemoved()
    {
        if (attributeAnimationInfos_.Empty())
            UnsubscribeFromEvent(GetScene(), E_ATTRIBUTEANIMATIONUPDATE);
    }

    void Component::HandleAttributeAnimationUpdate(StringHash eventType, VariantMap& eventData)
    {
        using namespace AttributeAnimationUpdate;

        UpdateAttributeAnimations(eventData[P_TIMESTEP].GetFloat());
    }

}