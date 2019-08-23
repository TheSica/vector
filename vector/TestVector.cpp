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

struct StructWithDeletedMembers
{
	StructWithDeletedMembers() = default;
	StructWithDeletedMembers(const StructWithDeletedMembers& other) = delete;
	StructWithDeletedMembers(StructWithDeletedMembers&& other) = delete;
	~StructWithDeletedMembers() = default;
	StructWithDeletedMembers& operator=(const StructWithDeletedMembers& other) = default;
	StructWithDeletedMembers& operator=(StructWithDeletedMembers&& other) = default;
};
//template class std::vector<StructWithDeletedMembers>;

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

TYPED_TEST_P(VectorTest, GivenDefaultConstructedVector_IsEmptyAndValid)
{
	Vector<TypeParam> vector;

	ASSERT_TRUE(vector.validate());
	EXPECT_TRUE(vector.empty());
}

TYPED_TEST_P(VectorTest, GivenCopyConstructedVector_IsEqualAndValid)
{
	Vector<TypeParam> vector1(10);
	Vector<TypeParam> vector2(vector1);
	EXPECT_TRUE(vector2.validate());
	EXPECT_TRUE(vector2 == vector1);
}

TYPED_TEST_P(VectorTest, GivenNonEmptyVector_CopyAssignmentOperatorWorks)
{
	Vector<TypeParam> vector1(5);
	Vector<TypeParam> vector2(10);

	vector2 = vector1;

	EXPECT_TRUE(vector2.validate());
	EXPECT_TRUE(vector2 == vector1);
}

TYPED_TEST_P(VectorTest, GivenEmptyVector_CopyAssignmentOperatorWorks)
{
	Vector<TypeParam> vector1;
	Vector<TypeParam> vector2(10);

	vector2 = vector1;

	EXPECT_TRUE(vector2.validate());
	EXPECT_TRUE(vector2 == vector1);
}

TYPED_TEST_P(VectorTest, GivenNonEmptyVector_SizeIsCorrect)
{
	Vector<TypeParam> vector1(10);

	EXPECT_TRUE(vector1.validate());
	EXPECT_TRUE(vector1.size() == 10);
}

TYPED_TEST_P(VectorTest, GivenEmptyVector_DestructorWorks)
{
	auto intArray = new Vector<TypeParam>();
	EXPECT_NO_FATAL_FAILURE(delete intArray);
}

REGISTER_TYPED_TEST_SUITE_P(VectorTest,
	GivenDefaultConstructedVector_IsEmptyAndValid,
	GivenCopyConstructedVector_IsEqualAndValid,
	GivenNonEmptyVector_CopyAssignmentOperatorWorks,
	GivenNonEmptyVector_SizeIsCorrect,
	GivenEmptyVector_DestructorWorks,
	GivenEmptyVector_CopyAssignmentOperatorWorks
);

using TestTypes = ::testing::Types<int, TestObject, std::list<TestObject>>;
INSTANTIATE_TYPED_TEST_CASE_P(ContainerTypesInstantiation, VectorTest, TestTypes);

TEST(AtOperator, GivenNonEmptyArray_AtOperatorWorks)
{
	Vector<int> intArray(5);
	EXPECT_TRUE(intArray[3] == 0);

	Vector<TestObject> toArray(5);
	EXPECT_TRUE(toArray[3] == TestObject(0));
}

TEST(AtOperator, GivenNonEmptyArray_AtOperatorThrowsWhenOutOfRange)
{
	Vector<TestObject> vec01(5);

	try
	{
		TestObject& r01 = vec01[6];
		EXPECT_TRUE(!(r01 == TestObject(0)));  // Should not get here, as exception thrown.
	}
	catch (std::out_of_range&)
	{
		EXPECT_TRUE(true);
	}
	catch (...)
	{
		EXPECT_TRUE(false);
	}
}

TEST(PushBackTests, GivenNonEmptyVector_ElementsAreAtCorrectPosition)
{
	Vector<int> vec;

	vec.push_back(0);
	vec.push_back(1);
	vec.push_back(2);
	vec.push_back(3);
	vec.push_back(4);

	EXPECT_EQ(vec[0], 0);
	EXPECT_EQ(vec[1], 1);
	EXPECT_EQ(vec[2], 2);
	EXPECT_EQ(vec[3], 3);
	EXPECT_EQ(vec[4], 4);
}

TEST(PushBackTests, GivenAntMetaDataRecord_PushBackWorksOnMetadataRecords)
{
	MetadataRecords mMetadataRecords;
	AntMetaDataRecord r, s;
	EXPECT_NO_FATAL_FAILURE(mMetadataRecords.push_back(r));
	EXPECT_NO_FATAL_FAILURE(mMetadataRecords.push_back(s));
}

TEST(PushBackTests, T)
{
	Vector<int> intArray(6);

	for (int i = 0; i < 99; ++i)
	{
		intArray.push_back(99);
	}

	EXPECT_TRUE(intArray.validate());
	EXPECT_EQ(intArray.size(), 105);
	EXPECT_EQ(intArray[76], 99);
}

TEST(ConstructorTests, GivenTestObjectVetor_MoveConstructorWorks)
{
	TestObject::Reset();

	Vector<TestObject> vec;
	TestObject to(33);

	vec.push_back(to);
	vec.push_back(to);
	vec.push_back(to);

	Vector<TestObject> toVectorA(std::move(vec));
	EXPECT_EQ(toVectorA.size(), 3);
	EXPECT_EQ(toVectorA.front().mX, 33);
	EXPECT_EQ(vec.size(), 0);
}

TEST(IteratorTests, GivenNonEmptyVector_AccessorsWork)
{
	Vector<int> intArray(10);
	intArray[0] = 10;
	intArray[1] = 11;
	intArray[2] = 12;

	EXPECT_TRUE(intArray.data() == &intArray[0]);
	EXPECT_TRUE(*intArray.data() == 10);
	EXPECT_TRUE(intArray.front() == 10);
	EXPECT_TRUE(intArray.back() == 0);
}

TEST(IteratorTests, GivenNonEmptyVector_AccessIteratorsWork)
{
	Vector<int> intArray(20);
	int i = 0;
	for (i = 0; i < 20; ++i)
	{
		intArray[i] = i;
	}

	i = 0;
	for (Vector<int>::iterator it = intArray.begin(); it != intArray.end(); ++it, ++i)
	{
		EXPECT_TRUE(*it == i);
	}
}

TEST(EmplaceBackTests, GivenItemWithConstMembers_EmplacBackWorks)
{
	Vector<ItemWithConst> myVec2;
	ItemWithConst& ref = myVec2.emplace_back(42);
	EXPECT_EQ(myVec2.back().i, 42);
	EXPECT_EQ(ref.i, 42);
}

TEST(EmplaceBackTests, GivenTestObjectVector_SizeIsCorrect)
{
	Vector<TestObject> toVectorA;

	toVectorA.emplace_back(2, 3, 4);

	EXPECT_EQ(toVectorA.size(), 1);
}

TEST(EmplaceBackTests, GivenTestObjectVector_ObjectIsConstructedProperly)
{
	Vector<TestObject> toVectorA;

	auto& to = toVectorA.emplace_back(2, 3, 4);

	EXPECT_EQ(toVectorA.back().mX, (2 + 3 + 4));
	EXPECT_EQ(to.mX, (2 + 3 + 4));
}

TEST(EmplaceBackTests, GivenTestObjectVector_ASingleObjectIsConstructed)
{
	TestObject::Reset();

	Vector<TestObject> toVectorA;

	toVectorA.emplace_back(2, 3, 4);
	EXPECT_EQ(TestObject::sTOCtorCount, 1);
}

TEST(EraseTests, GivenNonEmptyIntArray_SingleElementIsErased)
{
	Vector<int> intArray(20);

	for (int i = 0; i < 20; ++i)
	{
		intArray[i] = i;
	}

	// 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19

	intArray.erase(intArray.begin() + 10);  // Becomes: 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 11, 12, 13, 14, 15, 16, 17, 18, 19
	EXPECT_TRUE(intArray.validate());
	EXPECT_EQ(intArray.size(), 19);
}

TEST(EraseTests, GivenNonEmptyIntArray_ElementsAreShifted)
{
	Vector<int> intArray(20);

	for (int i = 0; i < 20; ++i)
	{
		intArray[i] = i;
	}

	// 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19

	intArray.erase(intArray.begin() + 10);  // Becomes: 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 11, 12, 13, 14, 15, 16, 17, 18, 19
	EXPECT_TRUE(intArray.validate());
	EXPECT_EQ(intArray[0], 0);
	EXPECT_EQ(intArray[10], 11);
	EXPECT_EQ(intArray[18], 19);
}

TEST(EraseTests, GivenNonEmptyIntArray_MultipleElementsAreErased)
{
	Vector<int> intArray(20);

	for (int i = 0; i < 20; ++i)
	{
		intArray[i] = i;
	}

	// 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19

	intArray.erase(intArray.begin() + 10); // Becomes: 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 11, 12, 13, 14, 15, 16, 17, 18, 19
	intArray.erase(intArray.begin() + 10, intArray.begin() + 15);  // Becomes: 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 16, 17, 18, 19
	EXPECT_TRUE(intArray.validate());
	EXPECT_EQ(intArray.size(), 14);
	EXPECT_EQ(intArray[9], 9);
	EXPECT_EQ(intArray[13], 19);

	intArray.erase(intArray.begin() + 1, intArray.begin() + 5);  // Becomes: 0, 5, 6, 7, 8, 9, 16, 17, 18, 19
	EXPECT_TRUE(intArray.validate());
	EXPECT_EQ(intArray.size(), 10);
	EXPECT_EQ(intArray[0], 0);
	EXPECT_EQ(intArray[1], 5);
	EXPECT_EQ(intArray[9], 19);

	intArray.erase(intArray.begin() + 7, intArray.begin() + 10);  // Becomes: 0, 5, 6, 7, 8, 9, 16
	EXPECT_TRUE(intArray.validate());
	EXPECT_EQ(intArray.size(), 7);
	EXPECT_EQ(intArray[0], 0);
	EXPECT_EQ(intArray[1], 5);
	EXPECT_EQ(intArray[6], 16);
}

TEST(EraseTests, GivenNonEmptyTestObjectArray_MultipleElementsAreErased)
{
	Vector<TestObject> toArray(20);
	for (int i = 0; i < 20; i++)
	{
		toArray[i] = TestObject(i);
	}

	toArray.erase(toArray.begin() + 10);
	EXPECT_TRUE(toArray.validate());
	EXPECT_EQ(toArray.size(), 19);
	EXPECT_EQ(toArray[10], TestObject(11));

	toArray.erase(toArray.begin() + 10, toArray.begin() + 15);
	EXPECT_TRUE(toArray.validate());
	EXPECT_EQ(toArray.size(), 14);
	EXPECT_EQ(toArray[10], TestObject(16));
}

TEST(AtTest, GivenNonEmptyArray_AtAccessorReturnsExpectedValue)
{
	Vector<int> intArray(20);

	for (int i = 0; i < 20; ++i)
	{
		intArray[i] = i;
	}

	// 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19

	intArray.erase(intArray.begin() + 10); // Becomes: 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 11, 12, 13, 14, 15, 16, 17, 18, 19
	intArray.erase(intArray.begin() + 10, intArray.begin() + 15);  // Becomes: 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 16, 17, 18, 19
	EXPECT_TRUE(intArray.validate());
	EXPECT_EQ(intArray.size(), 14);
	EXPECT_EQ(intArray.at(9), 9);
	EXPECT_EQ(intArray.at(13), 19);

	intArray.erase(intArray.begin() + 1, intArray.begin() + 5);  // Becomes: 0, 5, 6, 7, 8, 9, 16, 17, 18, 19
	EXPECT_TRUE(intArray.validate());
	EXPECT_EQ(intArray.size(), 10);
	EXPECT_EQ(intArray.at(0), 0);
	EXPECT_EQ(intArray.at(1), 5);
	EXPECT_EQ(intArray.at(9), 19);

	intArray.erase(intArray.begin() + 7, intArray.begin() + 10);  // Becomes: 0, 5, 6, 7, 8, 9, 16
	EXPECT_TRUE(intArray.validate());
	EXPECT_EQ(intArray.size(), 7);
	EXPECT_EQ(intArray.at(0), 0);
	EXPECT_EQ(intArray.at(1), 5);
	EXPECT_EQ(intArray.at(6), 16);
}

TEST(AtTest, GivenNonEmptyArray_AtAccessorModifiesValue)
{
	Vector<int> intArray(20);

	for (int i = 0; i < 20; ++i)
	{
		intArray[i] = i;
	}

	// 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19

	intArray.at(5) = 99;
	EXPECT_EQ(intArray.at(5), 99);
}

TEST(AtTest, GivenNonEmptyArray_AtAccessorThrowsWhenOutOfRange)
{
	Vector<int> intArray(3);

	EXPECT_THROW(intArray.at(99), std::out_of_range);
}
