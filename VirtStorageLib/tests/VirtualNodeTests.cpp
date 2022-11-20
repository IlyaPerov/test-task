#include "gtest/gtest.h"

#include "Storage.h"
#include "Volume.h"
#include "TestTools.h"
#include "TestData.h"

using namespace std;
using namespace vs;

class VirtualNodeTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
    }

    const std::string cName = "Virtual Storage";
    const std::string cRootName = "Virtual Root";

    StorageType m_storage{ cName, cRootName };
};

TEST_F(VirtualNodeTest, Root)
{
    const auto root = m_storage.GetRoot();

    EXPECT_EQ(root->GetName(), cRootName);
}

TEST_F(VirtualNodeTest, AddChild_Remove_Find)
{
    auto root = m_storage.GetRoot();

    for (auto i = 0; i < 3; i++)
    {
        auto child = root->AddChild("Child" + std::to_string(i + 1));
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

TEST_F(VirtualNodeTest, RemovedChild_Produces_Exception)
{
    auto root = m_storage.GetRoot();

    for (auto i = 0; i < 3; i++)
    {
        auto child = root->AddChild("Child" + std::to_string(i + 1));
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

    //EXPECT_THROW(foundChild->GetName(), ActionOnRemovedNodeException);
}

TEST_F(VirtualNodeTest, MountNode)
{
    auto virtRoot = m_storage.GetRoot();

    auto root = cRawRoot1;
	const auto volume1 = CreateVolume(root, "Volume1", 100);

    EXPECT_TRUE(IsEqual(volume1->GetRoot(), root));

    root.name = "Virtual Root";

	virtRoot->Mount(volume1->GetRoot());

    EXPECT_TRUE(IsEqual(virtRoot, root));
}

TEST_F(VirtualNodeTest, Remove_Child_Adter_Mount)
{
    auto virtRoot = m_storage.GetRoot();

    auto root = cRawRoot1;
    const auto volume1 = CreateVolume(root, "Volume1", 100);

    EXPECT_TRUE(IsEqual(volume1->GetRoot(), root));

    root.name = "Virtual Root";

    virtRoot->Mount(volume1->GetRoot());
    EXPECT_TRUE(IsEqual(virtRoot, root));

    volume1->GetRoot()->RemoveChildIf(
        [](auto node)
        {
            return node->GetName() == "child1";
		});

    root.children.pop_front();

    EXPECT_TRUE(IsEqual(virtRoot, root));
}

TEST_F(VirtualNodeTest, Mount_Several_Nodes)
{
    const auto virtRoot = m_storage.GetRoot();

    auto root1 = cRawRoot1;
    const auto volume1 = CreateVolume(root1, "Volume1", 100);

    const auto& root2 = cRawRoot2;
    const auto volume2 = CreateVolume(root2, "Volume2", 100);


    virtRoot->Mount(volume1->GetRoot());
    virtRoot->Mount(volume2->GetRoot());

    root1 += root2;
    root1.name = "Virtual Root";

    EXPECT_TRUE(IsEqual(virtRoot, root1));
}