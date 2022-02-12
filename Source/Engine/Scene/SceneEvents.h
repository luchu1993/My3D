//
// Created by luchu on 2022/2/1.
//

#pragma once

#include "Core/Object.h"

namespace My3D
{
    /// Variable timestep scene update.
    MY3D_EVENT(E_SCENEUPDATE, SceneUpdate)
    {
        MY3D_PARAM(P_SCENE, Scene);                  // Scene pointer
        MY3D_PARAM(P_TIMESTEP, TimeStep);            // float
    }

    /// Variable timestep scene post-update.
    MY3D_EVENT(E_SCENEPOSTUPDATE, ScenePostUpdate)
    {
        MY3D_PARAM(P_SCENE, Scene);                  // Scene pointer
        MY3D_PARAM(P_TIMESTEP, TimeStep);            // float
    }

    /// Scene subsystem update.
    MY3D_EVENT(E_SCENESUBSYSTEMUPDATE, SceneSubsystemUpdate)
    {
        MY3D_PARAM(P_SCENE, Scene);                  // Scene pointer
        MY3D_PARAM(P_TIMESTEP, TimeStep);            // float
    }

    /// A serializable's temporary state has changed.
    MY3D_EVENT(E_TEMPORARYCHANGED, TemporaryChanged)
    {
        MY3D_PARAM(P_SERIALIZABLE, Serializable);    // Serializable pointer
    }

    /// Scene attribute animation update.
    MY3D_EVENT(E_ATTRIBUTEANIMATIONUPDATE, AttributeAnimationUpdate)
    {
        MY3D_PARAM(P_SCENE, Scene);                  // Scene pointer
        MY3D_PARAM(P_TIMESTEP, TimeStep);            // float
    }

    /// Attribute animation added to object animation.
    MY3D_EVENT(E_ATTRIBUTEANIMATIONADDED, AttributeAnimationAdded)
    {
        MY3D_PARAM(P_OBJECTANIMATION, ObjectAnimation);               // Object animation pointer
        MY3D_PARAM(P_ATTRIBUTEANIMATIONNAME, AttributeAnimationName); // String
    }

    /// Attribute animation removed from object animation.
    MY3D_EVENT(E_ATTRIBUTEANIMATIONREMOVED, AttributeAnimationRemoved)
    {
        MY3D_PARAM(P_OBJECTANIMATION, ObjectAnimation);               // Object animation pointer
        MY3D_PARAM(P_ATTRIBUTEANIMATIONNAME, AttributeAnimationName); // String
    }

    /// A node's name has changed.
    MY3D_EVENT(E_NODENAMECHANGED, NodeNameChanged)
    {
        MY3D_PARAM(P_SCENE, Scene);                  // Scene pointer
        MY3D_PARAM(P_NODE, Node);                    // Node pointer
    }

    /// A component's enabled state has changed.
    MY3D_EVENT(E_COMPONENTENABLEDCHANGED, ComponentEnabledChanged)
    {
        MY3D_PARAM(P_SCENE, Scene);                  // Scene pointer
        MY3D_PARAM(P_NODE, Node);                    // Node pointer
        MY3D_PARAM(P_COMPONENT, Component);          // Component pointer
    }
    /// A child node has been added to a parent node.
    MY3D_EVENT(E_NODEADDED, NodeAdded)
    {
        MY3D_PARAM(P_SCENE, Scene);                  // Scene pointer
        MY3D_PARAM(P_PARENT, Parent);                // Node pointer
        MY3D_PARAM(P_NODE, Node);                    // Node pointer
    }

    /// A child node is about to be removed from a parent node. Note that individual component removal events will not be sent.
    MY3D_EVENT(E_NODEREMOVED, NodeRemoved)
    {
        MY3D_PARAM(P_SCENE, Scene);                  // Scene pointer
        MY3D_PARAM(P_PARENT, Parent);                // Node pointer
        MY3D_PARAM(P_NODE, Node);                    // Node pointer
    }

    /// A component has been created to a node.
    MY3D_EVENT(E_COMPONENTADDED, ComponentAdded)
    {
        MY3D_PARAM(P_SCENE, Scene);                  // Scene pointer
        MY3D_PARAM(P_NODE, Node);                    // Node pointer
        MY3D_PARAM(P_COMPONENT, Component);          // Component pointer
    }

    /// A component is about to be removed from a node.
    MY3D_EVENT(E_COMPONENTREMOVED, ComponentRemoved)
    {
        MY3D_PARAM(P_SCENE, Scene);                  // Scene pointer
        MY3D_PARAM(P_NODE, Node);                    // Node pointer
        MY3D_PARAM(P_COMPONENT, Component);          // Component pointer
    }

    /// A node's enabled state has changed.
    MY3D_EVENT(E_NODEENABLEDCHANGED, NodeEnabledChanged)
    {
        MY3D_PARAM(P_SCENE, Scene);                  // Scene pointer
        MY3D_PARAM(P_NODE, Node);                    // Node pointer
    }

    /// A node's tag has been added.
    MY3D_EVENT(E_NODETAGADDED, NodeTagAdded)
    {
        MY3D_PARAM(P_SCENE, Scene);                  // Scene pointer
        MY3D_PARAM(P_NODE, Node);                    // Node pointer
        MY3D_PARAM(P_TAG, Tag);                      // String tag
    }

    /// A node's tag has been removed.
    MY3D_EVENT(E_NODETAGREMOVED, NodeTagRemoved)
    {
        MY3D_PARAM(P_SCENE, Scene);                  // Scene pointer
        MY3D_PARAM(P_NODE, Node);                    // Node pointer
        MY3D_PARAM(P_TAG, Tag);                      // String tag
    }

    /// A node (and its children and components) has been cloned.
    MY3D_EVENT(E_NODECLONED, NodeCloned)
    {
        MY3D_PARAM(P_SCENE, Scene);                  // Scene pointer
        MY3D_PARAM(P_NODE, Node);                    // Node pointer
        MY3D_PARAM(P_CLONENODE, CloneNode);          // Node pointer
    }

    /// A component has been cloned.
    MY3D_EVENT(E_COMPONENTCLONED, ComponentCloned)
    {
        MY3D_PARAM(P_SCENE, Scene);                   // Scene pointer
        MY3D_PARAM(P_COMPONENT, Component);           // Component pointer
        MY3D_PARAM(P_CLONECOMPONENT, CloneComponent); // Component pointer
    }
}
