//
// Created by luchu on 2022/2/11.
//

#pragma once

#include "Scene/Component.h"


namespace My3D
{
    /// Placeholder for allowing unregistered components to be loaded & saved along with scenes.
    class MY3D_API UnknownComponent : public Component
    {
        MY3D_OBJECT(UnknownComponent, Component)
    public:
        /// Construct.
        explicit UnknownComponent(Context* context);
        /// Initialize the type name. Called by Node when loading.
        void SetTypeName(const String& typeName);
        /// Initialize the type hash only when type name not known. Called by Node when loading.
        void SetType(StringHash typeHash);

    private:
        /// Type of stored component.
        StringHash typeHash_;
        /// Type name of the stored component.
        String typeName_;
        /// XML format attribute infos.
        Vector<AttributeInfo> xmlAttributeInfos_;
        /// XML format attribute data (as strings).
        Vector<String> xmlAttributes_;
        /// Binary attributes.
        PODVector<unsigned char> binaryAttributes_;
        /// Flag of whether was loaded using XML/JSON data.
        bool useXML_;
    };
}
