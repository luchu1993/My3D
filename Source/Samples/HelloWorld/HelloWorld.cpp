#include "Launch/Application.h"
#include "Math/Vector2.h"
#include "Container/HashMap.h"
#include "Container/Vector.h"
#include "IO/Log.h"
#include "Input/Input.h"
#include "Core/Variant.h"
#include "Container/HashSet.h"


using namespace My3D;


class CustomKey
{
public:
    CustomKey() = default;
    explicit CustomKey(float v) : value(v) { }

    unsigned ToHash() const { return (unsigned) value; }
    bool operator ==(const CustomKey& rhs) const { return value == rhs.value; }

    float value;
};


class HelloWorld : public Application
{
    MY3D_OBJECT(HelloWorld, Application)
public:
    explicit HelloWorld(Context* context)
        : Base(context)
    {

    }

    void Setup() override
    {
        MY3D_LOGINFO("Setup My3D Engine!");
    }

    void Start() override
    {
        MY3D_LOGINFO("Start My3D Engine!");

        /// Test Vector
        Vector<String> v;
        v.Push("Alice");
        v.Push("Jack");

        Vector<String> v1(2, "Test");
        v1.Push(v);
        v1.Insert(1, "Nice");
        auto it = v1.Find("Test");
        v1.Erase(it);

        MY3D_LOGINFOF("size: %d", v1.Size());
        MY3D_LOGINFOF("index of Nice: %d", v1.IndexOf("Nice"));
        MY3D_LOGINFOF("contains nothing: ", v1.Contains("Nothing"));

        for (const auto& e : v1)
        {
            MY3D_LOGINFO(e);
        }

        /// Test Hashmap
        MY3D_LOGINFO("----------------------------- Map Test ------------------------------");
        HashMap<String, String> Map;
        Map["Jack"] = "JACK";
        Map.Populate("Alice", "Alice");

        Map.Insert(MakePair(String("Alice"), String("alice")));
        Map.Populate("Nike", "nike");
        Map.Populate("First", "first", "Second", "second");

        MY3D_LOGINFOF("size: %d", Map.Size());

        for (auto  const& kv : Map)
        {
            MY3D_LOGINFOF("%s = %s", *kv.first_, *kv.second_);
        }

        String key = "Hack";
        String result;
        if (Map.TryGetValue("JACK1", result))
            MY3D_LOGINFOF("found %s = %s", *key, *result);
        else
            MY3D_LOGINFOF("not found %s", *key);

        auto keys = Map.Keys();
        for (auto  const& k : keys)
        {
            MY3D_LOGINFO(k);
        }

        HashMap<CustomKey, String> Map2;
        Map2.Populate(CustomKey(1.0f), "one");
        Map2.Populate(CustomKey(2.0f), "second");

        for (const auto& kv : Map2)
        {
            MY3D_LOGINFOF("%f = %s", kv.first_.value, *kv.second_);
        }
        // Test hashset
        MY3D_LOGINFO("----------------------------- Test Hashset ------------------------------");
        HashSet<String> Set;
        Set.Insert("hello world");
        Set.Insert("Hello World");
        Set.Insert("Hello World");

        for (auto const& item : Set)
        {
            MY3D_LOGINFO(item);
        }

        // Test variant
        MY3D_LOGINFO("----------------------------- Test Variant ------------------------------");
        Variant var = 100.0f;
        MY3D_LOGINFOF("TypeName = %s, Value = %f", *var.GetTypeName(), var.Get<float>());

        VariantMap M;
        M.Populate("TestKey", "TestValue");
        var = M;

        for (const auto& kv : M)
        {
            MY3D_LOGINFOF("TypeName = %s, Value = { Key = %s, Value = %s }", *var.GetTypeName(), *kv.first_.ToString(), *kv.second_.Get<String>());
        }

        using namespace ExitRequested;
        SendEvent(E_EXITREQUESTED);
    }

    void Stop() override
    {
        MY3D_LOGINFO("Stop My3D Engine!");
    }
};


MY3D_DEFINE_APPLICATION_MAIN(HelloWorld)
