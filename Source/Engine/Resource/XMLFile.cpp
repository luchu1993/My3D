//
// Created by luchu on 2022/1/25.
//

#include "pugixml.hpp"

#include "Resource/XMLFile.h"
#include "Core/Context.h"
#include "IO/MemoryBuffer.h"
#include "IO/VectorBuffer.h"
#include "IO/Deserializer.h"
#include "IO/Log.h"
#include "Container/ArrayPtr.h"


namespace My3D
{
    /// XML writer for pugixml.
    class XMLWriter : public pugi::xml_writer
    {
    public:
        /// Construct.
        explicit XMLWriter(Serializer& dest)
            : dest_(dest)
            , success_(true)
        {
        }

        /// Write bytes to output.
        void write(const void* data, size_t size) override
        {
            if (dest_.Write(data, (unsigned)size) != size)
                success_ = false;
        }

        /// Destination serializer.
        Serializer& dest_;
        /// Success flag.
        bool success_;
    };

    XMLFile::XMLFile(Context *context)
        : Base(context)
        , document_(new pugi::xml_document())
    {
    }

    XMLFile::~XMLFile() = default;

    void XMLFile::RegisterObject(Context *context)
    {
        context->RegisterFactory<XMLFile>();
    }

    bool XMLFile::BeginLoad(Deserializer &source)
    {
        unsigned dataSize = source.GetSize();
        if (!dataSize && !source.GetName().Empty())
        {
            MY3D_LOGERROR("Zero sized XML data in " + source.GetName());
            return false;
        }

        SharedArrayPtr<char> buffer(new char[dataSize]);
        if (source.Read(buffer.Get(), dataSize) != dataSize)
            return false;

        if (!document_->load_buffer(buffer.Get(), dataSize))
        {
            MY3D_LOGERROR("Could not parse XML data from " + source.GetName());
            document_->reset();
            return false;
        }

        XMLElement rootElement = GetRoot();
        return true;
    }

    bool XMLFile::FromString(const String& source)
    {
        if (source.Empty())
            return false;

        MemoryBuffer buffer(source.CString(), source.Length());
        return Load(buffer);
    }

    XMLElement XMLFile::CreateRoot(const String& name)
    {
        document_->reset();
        pugi::xml_node root = document_->append_child(name.CString());
        return XMLElement(this, root.internal_object());
    }

    XMLElement XMLFile::GetOrCreateRoot(const String& name)
    {
        XMLElement root = GetRoot(name);
        if (root.NotNull())
            return root;
        root = GetRoot();
        if (root.NotNull())
            MY3D_LOGWARNING("XMLFile already has root " + root.GetName() + ", deleting it and creating root " + name);
        return CreateRoot(name);
    }

    XMLElement XMLFile::GetRoot(const String& name)
    {
        pugi::xml_node root = document_->first_child();
        if (root.empty())
            return XMLElement();

        if (!name.Empty() && name != root.name())
            return XMLElement();
        else
            return XMLElement(this, root.internal_object());
    }

    String XMLFile::ToString(const String& indentation) const
    {
        VectorBuffer dest;
        XMLWriter writer(dest);
        document_->save(writer, indentation.CString());
        return String((const char*)dest.GetData(), dest.GetSize());
    }
}