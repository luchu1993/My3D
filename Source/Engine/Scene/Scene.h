//
// Created by luchu on 2022/1/22.
//

#pragma once

#include "Container/HashSet.h"
#include "Core/Mutex.h"
#include "Resource/XMLElement.h"
#include "Scene/Node.h"
#include "Scene/SceneResolver.h"


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
        /// Register object factory. Node must be registered first.
        static void RegisterObject(Context* context);
        /// Load from binary data. Removes all existing child nodes and components first. Return true if successful.
        bool Load(Deserializer& source) override;
        /// Save to binary data. Return true if successful.
        bool Save(Serializer& dest) const override;
        /// Load from XML data. Removes all existing child nodes and components first. Return true if successful.
        bool LoadXML(const XMLElement& source) override;
        /// Mark for attribute check on the next network update.
        void MarkNetworkUpdate() override;
        /// Load from an XML file. Return true if successful.
        bool LoadXML(Deserializer& source);
        /// Save to an XML file. Return true if successful.
        bool SaveXML(Serializer& dest, const String& indentation = "\t") const;
        /// Load from a binary file asynchronously. Return true if started successfully. The LOAD_RESOURCES_ONLY mode can also be used to preload resources from object prefab files.
        bool LoadAsync(File* file, LoadMode mode = LOAD_SCENE_AND_RESOURCES);
        /// Load from an XML file asynchronously. Return true if started successfully. The LOAD_RESOURCES_ONLY mode can also be used to preload resources from object prefab files.
        bool LoadAsyncXML(File* file, LoadMode mode = LOAD_SCENE_AND_RESOURCES);
        /// Stop asynchronous loading.
        void StopAsyncLoading();
        /// Clear scene completely of either replicated, local or all nodes and components.
        void Clear(bool clearReplicated = true, bool clearLocal = true);

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
        /// Return node from the whole scene by ID, or null if not found.
        Node* GetNode(unsigned id) const;
        /// Get nodes with specific tag from the whole scene, return false if empty.
        bool GetNodesWithTag(PODVector<Node*>& dest, const String& tag)  const;
        /// Return whether updates are enabled.
        bool IsUpdateEnabled() const { return updateEnabled_; }
        /// Return component from the whole scene by ID, or null if not found.
        Component* GetComponent(unsigned id) const;
        /// Update scene. Called by HandleUpdate.
        void Update(float timeStep);
        /// Begin a threaded update. During threaded update components can choose to delay dirty processing.
        void BeginThreadedUpdate();
        /// End a threaded update. Notify components that marked themselves for delayed dirty processing.
        void EndThreadedUpdate();
        /// Add a component to the delayed dirty notify queue. Is thread-safe.
        void DelayedMarkedDirty(Component* component);
        /// Register a node user variable hash reverse mapping (for editing).
        void RegisterVar(const String& name);
        /// Unregister a node user variable hash reverse mapping.
        void UnregisterVar(const String& name);
        /// Clear all registered node user variable hash reverse mappings.
        void UnregisterAllVars();
        /// Return update time scale.
        float GetTimeScale() const { return timeScale_; }
        /// Return elapsed time in seconds.
        float GetElapsedTime() const { return elapsedTime_; }

    private:
        /// Handle the logic update event to update the scene, if active.
        void HandleUpdate(StringHash eventType, VariantMap& eventData);
        /// Update asynchronous loading.
        void UpdateAsyncLoading();
        /// Finish loading. Sets the scene filename and checksum.
        void FinishLoading(Deserializer* source);
        /// Finish saving. Sets the scene filename and checksum.
        void FinishSaving(Serializer* dest) const;
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
        /// Node and component ID resolver for asynchronous loading.
        SceneResolver resolver_;
        /// Source file name.
        mutable String fileName_;
        /// Required package files for networking.
        Vector<SharedPtr<PackageFile> > requiredPackageFiles_;
        /// Registered node user variable reverse mappings.
        HashMap<StringHash, String> varNames_;
        /// Nodes to check for attribute changes on the next network update.
        HashSet<unsigned> networkUpdateNodes_;
        /// Components to check for attribute changes on the next network update.
        HashSet<unsigned> networkUpdateComponents_;
        /// Delayed dirty notification queue for components.
        PODVector<Component*> delayedDirtyComponents_;
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
