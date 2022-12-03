#include "gtest/gtest.h"

#include "TestTools.h"
#include "TestData.h"

using namespace std;
using namespace vs;
using namespace test_tools;

TEST(TestToolsTest, IsEqual)
{
	EXPECT_TRUE(IsEqual({}, {}, true));
	EXPECT_TRUE(IsEqual({}, {}, false));

	RawNode node1 = { "node1" };
	RawNode node2 = { "node2" };

	EXPECT_FALSE(IsEqual(node1, node2));
	EXPECT_TRUE(IsEqual(node1, node2, true));
	EXPECT_NE(node1, node2);

	EXPECT_TRUE(IsEqual(cRawRoot1, cRawRoot1));
	EXPECT_EQ(cRawRoot1, cRawRoot1);

	const string newChild = "NewChild";
	node1 = cRawRoot1;
	node1.children.push_back({ newChild });

	node2 = cRawRoot1;
	node2.children.push_front({ newChild });

	EXPECT_EQ(node1, node2);

	const string newValue = "new value!";
	node2.children.front().values[100] = newValue; // a new value for NewChild
	EXPECT_NE(node1, node2);

	node1.children.back().values[100] = newValue + "!!"; // a new value for NewChild
	EXPECT_NE(node1, node2);

	node1.children.back().values[100] = newValue;
	EXPECT_EQ(node1, node2);

}

TEST(TestToolsTest, ToRawNode)
{
	const auto volume1 = CreateVolume(cRawRoot1, 100);
	EXPECT_EQ(ToRawNode(volume1.GetRoot()), cRawRoot1);
}

TEST(TestToolsTest, Merge_Raw_Nodes)
{
	auto root { cRawRoot1 };
	root.Merge(cRawRoot2);
	EXPECT_EQ(root, cRawRoot12);

	root = cRawRoot2;
	root.Merge(cRawRoot1);
	EXPECT_NE(root, cRawRoot12);
}