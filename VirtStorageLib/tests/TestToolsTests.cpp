#include "gtest/gtest.h"

#include "TestTools.h"
#include "TestData.h"

using namespace std;
using namespace vs;

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