//
// Created by luchu on 2022/1/25.
//

#pragma once

#include "Resource/Resource.h"
#include "Resource/XMLElement.h"


namespace pugi
{
    class xml_document;
    class xml_node;
    class xpath_node;
}


namespace My3D
{

    /// XML document resource
    class MY3D_API XMLFile : public Resource
    {
        MY3D_OBJECT(XMLFile, Resource)

    public:
        /// Construct
        explicit XMLFile(Context* context);
        /// Destruct
        ~XMLFile() override;
        /// Register object factory
        static void RegisterObject(Context* context);

        /// Load resource from stream. May be called from a worker thread. Return true if successful.
        bool BeginLoad(Deserializer& source) override;

        /// Deserialize from a string. Return true if successful.
        bool FromString(const String& source);
        /// Clear the document and create a root element.
        XMLElement CreateRoot(const String& name);
        /// Get the root element if it has matching name, otherwise create it and clear the document.
        XMLElement GetOrCreateRoot(const String& name);
        /// Return the root element, with optionally specified name. Return null element if not found.
        XMLElement GetRoot(const String& name = String::EMPTY);
        /// Return the pugixml document.
        pugi::xml_document* GetDocument() const { return document_.Get(); }
        /// Serialize the XML content to a string.
        String ToString(const String& indentation = "\t") const;

        /// Patch the XMLFile with another XMLFile. Based on RFC 5261.
        void Patch(XMLFile* patchFile);
        /// Patch the XMLFile with another XMLElement. Based on RFC 5261.
        void Patch(const XMLElement& patchElement);

    private:
        /// Pugixml document
        UniquePtr<pugi::xml_document> document_;
    };

}
