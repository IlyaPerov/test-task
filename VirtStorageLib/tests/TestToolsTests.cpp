#include "gtest/gtest.h"

#include "TestTools.h"
#include "TestData.h"

using namespace std;
using namespace vs;

TEST(TestToolsTest, Merge_Raw_Nodes)
{
	auto root = cRawRoot1;
	root += cRawRoot2;
	EXPECT_EQ(root, cRawRoot12);

	root = cRawRoot2;
	root += cRawRoot1;
	EXPECT_NE(root, cRawRoot12);
}