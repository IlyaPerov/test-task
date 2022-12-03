// tests.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include "gtest/gtest.h"

#include "TestTools.h"

using namespace std;
using namespace vs;
using namespace test_tools;

class VolumeNodeTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
    }

    const Priority cPriority = 0;
    const std::string cName = "Volume";
    const std::string cRootName = "Root";

	VolumeType m_volume{ cRootName, cPriority };
};

TEST_F(VolumeNodeTest, Root)
{
    const auto root = m_volume.GetRoot();

    EXPECT_EQ(root->GetName(), cRootName);
    EXPECT_EQ(root->GetPriority(), cPriority);
}

TEST_F(VolumeNodeTest, InsertChild_Remove_Find)
{
    auto root = m_volume.GetRoot();

    for (auto i = 0; i < 3; i++)
    {
        auto child = root->InsertChild("Child" + std::to_string(i + 1));
        EXPECT_NE(child, nullptr);
    }

    const string childName = "Child1";

    EXPECT_NE(
        root->FindChildIf(
	        [&childName](auto child)
	        {
	            return child->GetName() == childName;
	        }),
        nullptr);

    root->RemoveChildIf(
        [&childName](auto child)
        {
            return child->GetName() == childName;
        });

    EXPECT_EQ(
        root->FindChildIf(
            [&childName](auto child)
            {
                return child->GetName() == childName;
            }),
        nullptr);

}

TEST_F(VolumeNodeTest, RemovedChild_Produces_Exception)
{
    auto root = m_volume.GetRoot();

    for (auto i = 0; i < 3; i++)
    {
        auto child = root->InsertChild("Child" + std::to_string(i + 1));
        EXPECT_NE(child, nullptr);
    }

    const string childName = "Child1";

    auto foundChild = root->FindChildIf(
        [&childName](auto child)
        {
            return child->GetName() == childName;
        });

    EXPECT_NE(foundChild, nullptr);

    root->RemoveChildIf(
        [&childName](auto child)
        {
            return child->GetName() == childName;
        });

    EXPECT_THROW(foundChild->GetName(), ActionOnRemovedNodeException);
}

TEST_F(VolumeNodeTest, Insert_Erase_Find_Contains_Replace)
{
    const auto root = m_volume.GetRoot();

    root->Insert(100, "test");

    EXPECT_TRUE(root->Contains(100));

    ValueVariant value;
	EXPECT_TRUE(root->Find(100, value));
    EXPECT_EQ(get<string>(value), "test");

    EXPECT_TRUE(root->Replace(100, 3.14));
    EXPECT_TRUE(root->Find(100, value));
    EXPECT_EQ(get<double>(value), 3.14);

    root->Erase(100);
    EXPECT_FALSE(root->Contains(100));
    EXPECT_FALSE(root->Replace(100, 100));
    EXPECT_FALSE(root->Find(100, value));
}

TEST_F(VolumeNodeTest, TryInsert)
{
    const auto root = m_volume.GetRoot();

    EXPECT_TRUE(root->TryInsert(100, "test"));
    EXPECT_FALSE(root->TryInsert(100, "new test"));
}