//
// Created by luchu on 2022/1/22.
//

#pragma once

#include "Container/HashSet.h"
#include "Core/Mutex.h"
#include "Resource/XMLElement.h"
#include "Scene/Node.h"


namespace My3D
{
    class File;
    class PackageFile;

    static const unsigned FIRST_REPLICATED_ID = 0x1;
    static const unsigned LAST_REPLICATED_ID = 0xffffff;
    static const unsigned FIRST_LOCAL_ID = 0x01000000;
    static const unsigned LAST_LOCAL_ID = 0xffffffff;

    /// Asynchronous scene loading mode.
    enum LoadMode
    {
        /// Preload resources used by a scene or object prefab file, but do not load any scene content.
        LOAD_RESOURCES_ONLY = 0,
        /// Load scene content without preloading. Resources will be requested synchronously when encountered.
        LOAD_SCENE,
        /// Default mode: preload resources used by the scene first, then load the scene content.
        LOAD_SCENE_AND_RESOURCES
    };

    /// Asynchronous loading progress of a scene.
    struct AsyncProgress
    {
        /// File for binary mode.
        SharedPtr<File> file_;
        /// XML file for XML mode.
        SharedPtr<XMLFile> xmlFile_;

        /// Current XML element for XML mode.
        XMLElement xmlElement_;

        /// Current JSON child array and for JSON mode.
        unsigned jsonIndex_;

        /// Current load mode.
        LoadMode mode_;
        /// Resource name hashes left to load.
        HashSet<StringHash> resources_;
        /// Loaded resources.
        unsigned loadedResources_;
        /// Total resources.
        unsigned totalResources_;
        /// Loaded root-level nodes.
        unsigned loadedNodes_;
        /// Total root-level nodes.
        unsigned totalNodes_;
    };

    /// Root scene node, represents the whole scene.
    class MY3D_API Scene : public Node
    {
        MY3D_OBJECT(Scene, Node)

    public:
        using Node::GetComponent;
        using Node::SaveXML;

        /// Construct.
        explicit Scene(Context* context);
        /// Destruct.
        ~Scene() override;
        /// Return threaded update flag.
        bool IsThreadedUpdate() const { return threadedUpdate_; }
        /// Get free node ID, either non-local or local.
        unsigned GetFreeNodeID(CreateMode mode);
        /// Get free component ID, either non-local or local.
        unsigned GetFreeComponentID(CreateMode mode);
        /// Return whether the specified id is a replicated id.
        static bool IsReplicatedID(unsigned id) { return id < FIRST_LOCAL_ID; }
        /// Cache node by tag if tag not zero, no checking if already added. Used internaly in Node::AddTag.
        void NodeTagAdded(Node* node, const String& tag);
        /// Cache node by tag if tag not zero.
        void NodeTagRemoved(Node* node, const String& tag);
        /// Node added. Assign scene pointer and add to ID map.
        void NodeAdded(Node* node);
        /// Node removed. Remove from ID map.
        void NodeRemoved(Node* node);
        /// Component added. Add to ID map.
        void ComponentAdded(Component* component);
        /// Component removed. Remove from ID map.
        void ComponentRemoved(Component* component);
        /// Mark a node for attribute check on the next network update.
        void MarkNetworkUpdate(Node* node);
        /// Mark a component for attribute check on the next network update.
        void MarkNetworkUpdate(Component* component);
        /// Mark a node dirty in scene replication states. The node does not need to have own replication state yet.
        void MarkReplicationDirty(Node* node);
        /// Update scene. Called by HandleUpdate.
        void Update(float timeStep);

    private:
        /// Handle the logic update event to update the scene, if active.
        void HandleUpdate(StringHash eventType, VariantMap& eventData);
        /// Update asynchronous loading.
        void UpdateAsyncLoading();
        /// Replicated scene nodes by ID.
        HashMap<unsigned, Node*> replicatedNodes_;
        /// Local scene nodes by ID.
        HashMap<unsigned, Node*> localNodes_;
        /// Replicated components by ID.
        HashMap<unsigned, Component*> replicatedComponents_;
        /// Local components by ID.
        HashMap<unsigned, Component*> localComponents_;
        /// Cached tagged nodes by tag.
        HashMap<StringHash, PODVector<Node*> > taggedNodes_;
        /// Asynchronous loading progress.
        AsyncProgress asyncProgress_;
        /// Nodes to check for attribute changes on the next network update.
        HashSet<unsigned> networkUpdateNodes_;
        /// Components to check for attribute changes on the next network update.
        HashSet<unsigned> networkUpdateComponents_;
        /// Mutex for the delayed dirty notification queue.
        Mutex sceneMutex_;
        /// Next free non-local node ID.
        unsigned replicatedNodeID_;
        /// Next free non-local component ID.
        unsigned replicatedComponentID_;
        /// Next free local node ID.
        unsigned localNodeID_;
        /// Next free local component ID.
        unsigned localComponentID_;
        /// Scene source file checksum.
        mutable unsigned checksum_;
        float timeScale_;
        /// Elapsed time accumulator.
        float elapsedTime_;
        /// Motion smoothing constant.
        float smoothingConstant_;
        /// Motion smoothing snap threshold.
        float snapThreshold_;
        /// Update enabled flag.
        bool updateEnabled_;
        /// Asynchronous loading flag.
        bool asyncLoading_;
        /// Threaded update flag.
        bool threadedUpdate_;
    };
}
