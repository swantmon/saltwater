
#include "test_precompiled.h"

#include "base/base_include_glm.h"
#include "base/base_test_defines.h"
#include "base/base_serialize_std_string.h"
#include "base/base_serialize_std_vector.h"
#include "base/base_serialize_text_reader.h"
#include "base/base_serialize_text_writer.h"
#include "base/base_serialize_binary_reader.h"
#include "base/base_serialize_binary_writer.h"
#include "base/base_type_info.h"

#include <string>
#include <vector>

// -----------------------------------------------------------------------------

class CComplexClass
{
public:

    int a;

public:

    template <class TArchive>
    inline void Read(TArchive& _rCodec)
    {
        _rCodec >> a;
    }

    template <class TArchive>
    inline void Write(TArchive& _rCodec)
    {
        _rCodec << a;
    }
};

class CMoreComplexClass
{
public:

    int a;
    std::vector<CComplexClass> b;

public:

    CMoreComplexClass()
        : a(1337)
    {
        b.resize(glm::linearRand(1, 20));

        for (auto& Value : b) Value.a = glm::linearRand(1, 255);
    }

public:

    template <class TArchive>
    inline void Read(TArchive& _rCodec)
    {
        _rCodec >> a;

        Base::Serialize(_rCodec, b);
    }

    template <class TArchive>
    inline void Write(TArchive& _rCodec)
    {
        _rCodec << a;

        Base::Serialize(_rCodec, b);
    }
};

// -----------------------------------------------------------------------------

BASE_TEST(SerializeDataWithText)
{
    // -----------------------------------------------------------------------------
    // Data
    // -----------------------------------------------------------------------------
    int IntegerValue = 4;
    float FloatingValue = 13.37f;
    double DoubleValue = 12.34;
    char* pCharValue = "This is a test";
    std::string StringValue = "This is just another test!";

    std::vector<int> IntegerListValue = { 0, 1, 2, 3 };

    // -----------------------------------------------------------------------------
    // Stream (this could be also a file)
    // -----------------------------------------------------------------------------
    std::stringstream Stream;

    // -----------------------------------------------------------------------------
    // Writing
    // -----------------------------------------------------------------------------
    Base::CTextWriter Writer(Stream, 1);

    Writer << IntegerValue;
    Writer << FloatingValue;
    Writer << DoubleValue;
    Writer << pCharValue;

    Base::Serialize(Writer, StringValue);
    Base::Serialize(Writer, IntegerListValue);

    // -----------------------------------------------------------------------------
    // Test data
    // -----------------------------------------------------------------------------
    int IntegerValueTest;
    float FloatingValueTest;
    double DoubleValueTest;
    char* pCharValueTest;
    std::string StringValueTest;

    std::vector<int> IntegerListValueTest;

    // -----------------------------------------------------------------------------
    // Reading
    // -----------------------------------------------------------------------------
    Base::CTextReader Reader(Stream, 1);

    Reader >> IntegerValueTest;
    Reader >> FloatingValueTest;
    Reader >> DoubleValueTest;
    Reader >> pCharValueTest;

    Base::Serialize(Reader, StringValueTest);
    Base::Serialize(Reader, IntegerListValueTest);

    // -----------------------------------------------------------------------------
    // Check
    // -----------------------------------------------------------------------------
    BASE_CHECK(IntegerValue == IntegerValueTest);
    BASE_CHECK(FloatingValue == FloatingValueTest);
    BASE_CHECK(DoubleValue == DoubleValueTest);
    BASE_CHECK(strcmp(pCharValue, pCharValueTest) == 0);
    BASE_CHECK(StringValue == StringValueTest);

    int Index = 0;

    for (auto& Value : IntegerListValueTest)
    {
        BASE_CHECK(Value == IntegerListValue[Index]);

        ++Index;
    }
}

// -----------------------------------------------------------------------------

BASE_TEST(SerializeDataWithTextFile)
{
    // -----------------------------------------------------------------------------
    // Data
    // -----------------------------------------------------------------------------
    int IntegerValue = 4;
    float FloatingValue = 13.37f;
    double DoubleValue = 12.34;
    char* pCharValue = "This is a test";
    std::string StringValue = "This is just another test!";

    std::vector<int> IntegerListValue = { 0, 1, 2, 3 };

    CMoreComplexClass MoreCompexClass;

    std::vector<CMoreComplexClass> MoreComplexClassListValue;

    MoreComplexClassListValue.resize(glm::linearRand(1, 10));

    // -----------------------------------------------------------------------------
    // Stream
    // -----------------------------------------------------------------------------
    std::ofstream oStream;

    oStream.open("SerializeDataWithTextFile.txt");

    // -----------------------------------------------------------------------------
    // Writing
    // -----------------------------------------------------------------------------
    Base::CTextWriter Writer(oStream, 1);

    Writer << IntegerValue;
    Writer << FloatingValue;
    Writer << DoubleValue;
    Writer << pCharValue;
    Writer << MoreCompexClass;

    Base::Serialize(Writer, StringValue);
    Base::Serialize(Writer, IntegerListValue);
    Base::Serialize(Writer, MoreComplexClassListValue);

    oStream.close();

    // -----------------------------------------------------------------------------
    // Test data
    // -----------------------------------------------------------------------------
    int IntegerValueTest;
    float FloatingValueTest;
    double DoubleValueTest;
    char* pCharValueTest;
    std::string StringValueTest;

    std::vector<int> IntegerListValueTest;

    CMoreComplexClass MoreCompexClassTest;

    std::vector<CMoreComplexClass> MoreComplexClassListValueTest;

    // -----------------------------------------------------------------------------
    // Stream
    // -----------------------------------------------------------------------------
    std::ifstream iStream;

    iStream.open("SerializeDataWithTextFile.txt");

    // -----------------------------------------------------------------------------
    // Reading
    // -----------------------------------------------------------------------------
    Base::CTextReader Reader(iStream, 1);

    Reader >> IntegerValueTest;
    Reader >> FloatingValueTest;
    Reader >> DoubleValueTest;
    Reader >> pCharValueTest;
    Reader >> MoreCompexClassTest;

    Base::Serialize(Reader, StringValueTest);
    Base::Serialize(Reader, IntegerListValueTest);
    Base::Serialize(Reader, MoreComplexClassListValueTest);

    iStream.close();

    // -----------------------------------------------------------------------------
    // Check
    // -----------------------------------------------------------------------------
    BASE_CHECK(IntegerValue == IntegerValueTest);
    BASE_CHECK(FloatingValue == FloatingValueTest);
    BASE_CHECK(DoubleValue == DoubleValueTest);
    BASE_CHECK(strcmp(pCharValue, pCharValueTest) == 0);
    BASE_CHECK(StringValue == StringValueTest);
    BASE_CHECK(MoreCompexClassTest.a == MoreCompexClassTest.a);

    int Index = 0;
    int Index2 = 0;

    for (auto& Value2 : MoreCompexClassTest.b)
    {
        BASE_CHECK(Value2.a == MoreCompexClassTest.b[Index2].a);

        ++Index2;
    }

    for (auto& Value : IntegerListValueTest)
    {
        BASE_CHECK(Value == IntegerListValue[Index]);

        ++Index;
    }

    Index = 0;
    Index2 = 0;

    for (auto& Value : MoreComplexClassListValueTest)
    {
        BASE_CHECK(Value.a == MoreComplexClassListValue[Index].a);

        for (auto& Value2 : Value.b)
        {
            BASE_CHECK(Value2.a == MoreComplexClassListValue[Index].b[Index2].a);

            ++Index2;
        }

        Index2 = 0;

        ++Index;
    }
}

// -----------------------------------------------------------------------------

BASE_TEST(SerializeDataWithBinary)
{
    // -----------------------------------------------------------------------------
    // Data
    // -----------------------------------------------------------------------------
    int IntegerValue = 4;
    float FloatingValue = 13.37f;
    double DoubleValue = 12.34;
    char* pCharValue = "This is a test";
    std::string StringValue = "This is just another test!";

    std::vector<int> IntegerListValue = { 0, 1, 2, 3 };

    // -----------------------------------------------------------------------------
    // Stream (this could be also a file)
    // -----------------------------------------------------------------------------
    std::stringstream Stream;

    // -----------------------------------------------------------------------------
    // Writing
    // -----------------------------------------------------------------------------
    Base::CBinaryWriter Writer(Stream, 1);

    Writer << IntegerValue;
    Writer << FloatingValue;
    Writer << DoubleValue;
    Writer << pCharValue;

    Base::Serialize(Writer, StringValue);
    Base::Serialize(Writer, IntegerListValue);

    // -----------------------------------------------------------------------------
    // Test data
    // -----------------------------------------------------------------------------
    int IntegerValueTest;
    float FloatingValueTest;
    double DoubleValueTest;
    char* pCharValueTest;
    std::string StringValueTest;

    std::vector<int> IntegerListValueTest;

    // -----------------------------------------------------------------------------
    // Reading
    // -----------------------------------------------------------------------------
    Base::CBinaryReader Reader(Stream, 1);

    Reader >> IntegerValueTest;
    Reader >> FloatingValueTest;
    Reader >> DoubleValueTest;
    Reader >> pCharValueTest;

    Base::Serialize(Reader, StringValueTest);
    Base::Serialize(Reader, IntegerListValueTest);

    // -----------------------------------------------------------------------------
    // Check
    // -----------------------------------------------------------------------------
    BASE_CHECK(IntegerValue == IntegerValueTest);
    BASE_CHECK(FloatingValue == FloatingValueTest);
    BASE_CHECK(DoubleValue == DoubleValueTest);
    BASE_CHECK(strcmp(pCharValue, pCharValueTest) == 0);
    BASE_CHECK(StringValue == StringValueTest);

    int Index = 0;

    for (auto& Value : IntegerListValueTest)
    {
        BASE_CHECK(Value == IntegerListValue[Index]);

        ++Index;
    }
}

// -----------------------------------------------------------------------------

BASE_TEST(SerializeComplexWithText)
{
    // -----------------------------------------------------------------------------
    // Data
    // -----------------------------------------------------------------------------
    CComplexClass CompexClass = { 1337 };

    std::vector<CComplexClass> ComplexClassListValue = { {0}, {1}, {2}, {3} };

    // -----------------------------------------------------------------------------
    // Stream (this could be also a file)
    // -----------------------------------------------------------------------------
    std::stringstream Stream;

    // -----------------------------------------------------------------------------
    // Writing
    // -----------------------------------------------------------------------------
    Base::CTextWriter Writer(Stream, 1);

    Writer << CompexClass;

    Base::Serialize(Writer, ComplexClassListValue);

    // -----------------------------------------------------------------------------
    // Test data
    // -----------------------------------------------------------------------------
    CComplexClass CompexClassTest;

    std::vector<CComplexClass> ComplexClassListValueTest;

    // -----------------------------------------------------------------------------
    // Reading
    // -----------------------------------------------------------------------------
    Base::CTextReader Reader(Stream, 1);

    Reader >> CompexClassTest;

    Base::Serialize(Reader, ComplexClassListValueTest);

    // -----------------------------------------------------------------------------
    // Check
    // -----------------------------------------------------------------------------
    BASE_CHECK(CompexClass.a == CompexClassTest.a);

    int Index = 0;

    for (auto& Class : ComplexClassListValue)
    {
        BASE_CHECK(Class.a == ComplexClassListValueTest[Index].a);

        ++Index;
    }
}

// -----------------------------------------------------------------------------

BASE_TEST(SerializeComplexWithBinary)
{
    // -----------------------------------------------------------------------------
    // Data
    // -----------------------------------------------------------------------------
    CComplexClass CompexClass = { 1337 };

    std::vector<CComplexClass> ComplexClassListValue = { {0}, {1}, {2}, {3} };

    // -----------------------------------------------------------------------------
    // Stream (this could be also a file)
    // -----------------------------------------------------------------------------
    std::stringstream Stream;

    // -----------------------------------------------------------------------------
    // Writing
    // -----------------------------------------------------------------------------
    Base::CBinaryWriter Writer(Stream, 1);

    Writer << CompexClass;

    Base::Serialize(Writer, ComplexClassListValue);

    // -----------------------------------------------------------------------------
    // Test data
    // -----------------------------------------------------------------------------
    CComplexClass CompexClassTest;

    std::vector<CComplexClass> ComplexClassListValueTest;

    // -----------------------------------------------------------------------------
    // Reading
    // -----------------------------------------------------------------------------
    Base::CBinaryReader Reader(Stream, 1);

    Reader >> CompexClassTest;

    Base::Serialize(Reader, ComplexClassListValueTest);

    // -----------------------------------------------------------------------------
    // Check
    // -----------------------------------------------------------------------------
    BASE_CHECK(CompexClass.a == CompexClassTest.a);

    int Index = 0;

    for (auto& Class : ComplexClassListValue)
    {
        BASE_CHECK(Class.a == ComplexClassListValueTest[Index].a);

        ++Index;
    }
}

// -----------------------------------------------------------------------------

#include <map>


class IFactoryBase
{
public:
    virtual inline void Read(Base::CTextReader& _rCodec) = 0;
    virtual inline void Write(Base::CTextWriter& _rCodec) = 0;
    virtual IFactoryBase* Create() = 0;
};

class CSerializeFactory
{

public:

    static CSerializeFactory& GetInstance()
    {
        static CSerializeFactory s_Instance;
        return s_Instance;
    }

public:

    using CFactoryMap     = std::map<Base::ID, IFactoryBase*>;
    using CFactoryMapPair = std::pair<Base::ID, IFactoryBase*>;

public:

    CFactoryMap m_Factory;

public:

    template<class T>
    void Register(IFactoryBase* _pBase)
    {
        auto ID = Base::CTypeInfo::GetTypeID<T>();

        if (m_Factory.find(ID) == m_Factory.end()) m_Factory.insert(CFactoryMapPair(ID, _pBase));
    }

    IFactoryBase* Allocate(Base::ID _ID)
    {
        if (m_Factory.find(_ID) != m_Factory.end()) return m_Factory.find(_ID)->second->Create();

        return nullptr;
    }
};

class CDerivedA : public IFactoryBase
{
public:

    int a;

public:

    inline void Read(Base::CTextReader& _rCodec) override
    {
        _rCodec >> a;
    }

    inline void Write(Base::CTextWriter& _rCodec) override
    {
        _rCodec << a;
    }

    IFactoryBase* Create() override
    {
        return new CDerivedA();
    }
};
struct SRegisterSerializeDerivedA
{
    SRegisterSerializeDerivedA()
    {
        static CDerivedA s_FactorySRegisterSerializeDerivedA;
        CSerializeFactory::GetInstance().Register<CDerivedA>(&s_FactorySRegisterSerializeDerivedA);
    }
} g_SRegisterSerializeDerivedA;


class CDerivedB : public IFactoryBase
{
public:

    float b;

public:

    inline void Read(Base::CTextReader& _rCodec) override
    {
        _rCodec >> b;
    }

    inline void Write(Base::CTextWriter& _rCodec) override
    {
        _rCodec << b;
    }

    IFactoryBase* Create() override
    {
        return new CDerivedB();
    }
};
struct SRegisterSerializeDerivedB
{
    SRegisterSerializeDerivedB()
    {
        static CDerivedB s_FactorySRegisterSerializeDerivedB;
        CSerializeFactory::GetInstance().Register<CDerivedB>(&s_FactorySRegisterSerializeDerivedB);
    }
} g_SRegisterSerializeDerivedB;

BASE_TEST(SerializeInterfaceWithText)
{
    // -----------------------------------------------------------------------------
    // Data
    // -----------------------------------------------------------------------------
    CDerivedA CompexClassA;
    CDerivedB CompexClassB;

    CompexClassA.a = 4;
    CompexClassB.b = 13.37f;

    // -----------------------------------------------------------------------------
    // Stream (this could be also a file)
    // -----------------------------------------------------------------------------
    std::stringstream Stream;

    // -----------------------------------------------------------------------------
    // Writing
    // -----------------------------------------------------------------------------
    Base::CTextWriter Writer(Stream, 1);

    auto ID = Base::CTypeInfo::GetTypeID<CDerivedA>();
    Writer << ID;
    Writer << CompexClassA;

    ID = Base::CTypeInfo::GetTypeID<CDerivedB>();
    Writer << ID;
    Writer << CompexClassB;

    // -----------------------------------------------------------------------------
    // Test data
    // -----------------------------------------------------------------------------
    Base::ID IDTest;

    IFactoryBase* pBaseCompexClassATest;
    IFactoryBase* pBaseCompexClassBTest;

    // -----------------------------------------------------------------------------
    // Reading
    // -----------------------------------------------------------------------------
    Base::CTextReader Reader(Stream, 1);

    Reader >> IDTest;

    pBaseCompexClassATest = CSerializeFactory::GetInstance().Allocate(IDTest);

    Reader >> (*pBaseCompexClassATest);

    Reader >> IDTest;

    pBaseCompexClassBTest = CSerializeFactory::GetInstance().Allocate(IDTest);

    Reader >> (*pBaseCompexClassBTest);

    // -----------------------------------------------------------------------------
    // Convert
    // -----------------------------------------------------------------------------
    CDerivedA& rCompexClassATest = *reinterpret_cast<CDerivedA*>(pBaseCompexClassATest);
    CDerivedB& rCompexClassBTest = *reinterpret_cast<CDerivedB*>(pBaseCompexClassBTest);

    // -----------------------------------------------------------------------------
    // Check
    // -----------------------------------------------------------------------------
    BASE_CHECK(CompexClassA.a == rCompexClassATest.a);
    BASE_CHECK(CompexClassB.b == rCompexClassBTest.b);

    // -----------------------------------------------------------------------------
    // Remove
    // -----------------------------------------------------------------------------
    delete pBaseCompexClassATest;
    delete pBaseCompexClassBTest;
}