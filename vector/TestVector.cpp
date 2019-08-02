#include <gtest/gtest.h>
#include "Vector.h"
#include "TestObject.h"

int64_t TestObject::sTOCount = 0;
int64_t TestObject::sTOCtorCount = 0;
int64_t TestObject::sTODtorCount = 0;
int64_t TestObject::sTODefaultCtorCount = 0;
int64_t TestObject::sTOArgCtorCount = 0;
int64_t TestObject::sTOCopyCtorCount = 0;
int64_t TestObject::sTOMoveCtorCount = 0;
int64_t TestObject::sTOCopyAssignCount = 0;
int64_t TestObject::sTOMoveAssignCount = 0;
int     TestObject::sMagicErrorCount = 0;

// EA_NON_COPYABLE
	//
	// This macro defines as a class as not being copy-constructable
	// or assignable. This is useful for preventing class instances 
	// from being passed to functions by value, is useful for preventing
	// compiler warnings by some compilers about the inability to 
	// auto-generate a copy constructor and assignment, and is useful 
	// for simply declaring in the interface that copy semantics are
	// not supported by the class. Your class needs to have at least a
	// default constructor when using this macro.
	//
	// Beware that this class works by declaring a private: section of 
	// the class in the case of compilers that don't support C++11 deleted
	// functions. 
	//
	// Note: With some pre-C++11 compilers (e.g. Green Hills), you may need 
	//       to manually define an instances of the hidden functions, even 
	//       though they are not used.
	//
	// Example usage:
	//    class Widget {
	//       Widget();
	//       . . .
	//       EA_NON_COPYABLE(Widget)
	//    };
	//
#if !defined(EA_NON_COPYABLE)
#if defined(EA_COMPILER_NO_DELETED_FUNCTIONS)
#define EA_NON_COPYABLE(EAClass_)               \
			  private:                                      \
				EAClass_(const EAClass_&);                  \
				void operator=(const EAClass_&);
#else
#define EA_NON_COPYABLE(EAClass_)               \
				EAClass_(const EAClass_&) = delete;         \
				void operator=(const EAClass_&) = delete;
#endif
#endif

// ------------------------------------------------------------------------
	// EANonCopyable
	//
	// Declares a class as not supporting copy construction or assignment.
	// May be more reliable with some situations that EA_NON_COPYABLE alone,
	// though it may result in more code generation.
	//
	// Note that VC++ will generate warning C4625 and C4626 if you use EANonCopyable
	// and you are compiling with /W4 and /Wall. There is no resolution but
	// to redelare EA_NON_COPYABLE in your subclass or disable the warnings with
	// code like this:
	//     EA_DISABLE_VC_WARNING(4625 4626)
	//     ...
	//     EA_RESTORE_VC_WARNING()
	//
	// Example usage:
	//     struct Widget : EANonCopyable {
	//        . . .
	//     };
	//
#ifdef __cplusplus
struct EANonCopyable
{
#if defined(EA_COMPILER_NO_DEFAULTED_FUNCTIONS) || defined(__EDG__) // EDG doesn't appear to behave properly for the case of defaulted constructors; it generates a mistaken warning about missing default constructors.
	EANonCopyable() {} // Putting {} here has the downside that it allows a class to create itself, 
	~EANonCopyable() {} // but avoids linker errors that can occur with some compilers (e.g. Green Hills).
#else
	EANonCopyable() = default;
	~EANonCopyable() = default;
#endif
	EA_NON_COPYABLE(EANonCopyable)
};
#endif

// Template instantations.
// These tell the compiler to compile all the functions for the given class.
template Vector<bool>;
template Vector<int>;
template class Vector<TestObject>;

struct AddressOfOperatorResult {};
struct HasAddressOfOperator
{
	// problematic 'addressof' operator that doesn't return a pointer type
	AddressOfOperatorResult operator&() const { return {}; }
	bool operator==(const HasAddressOfOperator&) const { return false; }
};
template class Vector<HasAddressOfOperator>;  // force compile all functions of vector

// Test compiler issue that appeared in VS2012 relating to kAlignment
struct StructWithContainerOfStructs
{
	Vector<StructWithContainerOfStructs> children;
};

struct ScenarioRefEntry
{
	ScenarioRefEntry(const std::string& contextDatabase) : ContextDatabase(contextDatabase) {}

	struct RowEntry
	{
		RowEntry(int levelId, int sceneId, int actorId, int partId, const std::string& controller)
			: LevelId(levelId), SceneId(sceneId), ActorId(actorId), PartId(partId), Controller(controller)
		{
		}

		int LevelId;
		int SceneId;
		int ActorId;
		int PartId;
		const std::string& Controller;
	};
	const std::string& ContextDatabase;  // note:  const class members prohibits move semantics
	typedef Vector<RowEntry> RowData;
	RowData Rows;
};
typedef Vector<ScenarioRefEntry> ScenarRefData;
struct AntMetaDataRecord
{
	ScenarRefData ScenarioRefs;
};
typedef Vector<AntMetaDataRecord> MetadataRecords;

struct StructWithConstInt
{
	StructWithConstInt(const int& _i) : i(_i) {}
	const int i;
};

struct StructWithConstRefToInt
{
	StructWithConstRefToInt(const int& _i) : i(_i) {}
	const int& i;
};

struct ItemWithConst
{
	ItemWithConst& operator=(const ItemWithConst&);

public:
	ItemWithConst(int _i) : i(_i) {}
	ItemWithConst(const ItemWithConst& x) : i(x.i) {}
	const int i;
};

struct testmovable
{
	EA_NON_COPYABLE(testmovable)
public:
	testmovable() noexcept {}

	testmovable(testmovable&&) noexcept {}

	testmovable& operator=(testmovable&&) noexcept { return *this; }
};

struct TestMoveAssignToSelf
{
	TestMoveAssignToSelf() noexcept : mMovedToSelf(false) {}
	TestMoveAssignToSelf(const TestMoveAssignToSelf& other) { mMovedToSelf = other.mMovedToSelf; }
	TestMoveAssignToSelf& operator=(TestMoveAssignToSelf&&) { mMovedToSelf = true; return *this; }
	TestMoveAssignToSelf& operator=(const TestMoveAssignToSelf&) = delete;

	bool mMovedToSelf;
};

template <typename T>
class VectorTest : public ::testing::Test { };

TYPED_TEST_CASE_P(VectorTest);

using TestTypes = ::testing::Types<int, TestObject, std::list<TestObject>>;

TYPED_TEST_SUITE(VectorTest, TestTypes);

TYPED_TEST_P(VectorTest, GivenDefaultConstructedVector_IsEmptyAndValid)
{
	TypeParam vector;
	EXPECT_TRUE(vector.validate());
	EXPECT_TRUE(vector.empty());
}

TYPED_TEST_P(VectorTest, GivenCopyConstructedVector_IsEqualAndValid)
{
	TypeParam vector1(10);
	TypeParam vector2(vector1);

	EXPECT_TRUE(vector2.validate());
	EXPECT_TRUE(vector2 == vector1);
}

REGISTER_TYPED_TEST_CASE_P(VectorTest, 
						   GivenDefaultConstructedVector_IsEmptyAndValid, 
						   GivenCopyConstructedVector_IsEqualAndValid);

TEST(ConstructorTests, GivenEmptyVector_DestructorWorks)
{
	TestObject::Reset();

	auto intArray = new Vector<int>();
	EXPECT_NO_FATAL_FAILURE(delete intArray);
}
