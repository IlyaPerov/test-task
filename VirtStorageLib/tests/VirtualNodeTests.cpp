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

    StorageType m_storage{ cRootName };
};

TEST_F(VirtualNodeTest, Root)
{
    const auto root = m_storage.GetRoot();

    EXPECT_EQ(root->GetName(), cRootName);
}

TEST_F(VirtualNodeTest, AddChild_Remove_Find)
{
    const auto root = m_storage.GetRoot();

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
    const auto root = m_storage.GetRoot();

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

    foundChild = root->FindChildIf(
        [&childName](auto child)
        {
            return child->GetName() == childName;
        });

    EXPECT_EQ(foundChild, nullptr);

    EXPECT_THROW(foundChild->GetName(), ActionOnRemovedNodeException);
}

TEST_F(VirtualNodeTest, MountNode)
{
    const auto virtRoot = m_storage.GetRoot();

	const auto volume1 = CreateVolume(cRawRoot1, "Volume1", 100);
    EXPECT_TRUE(IsEqual(volume1->GetRoot(), cRawRoot1));

	virtRoot->Mount(volume1->GetRoot());
    EXPECT_TRUE(IsEqual(virtRoot, cRawRoot1));
}

TEST_F(VirtualNodeTest, Remove_Child_Adter_Mount)
{
    auto virtRoot = m_storage.GetRoot();

    const auto volume1 = CreateVolume(cRawRoot1, "Volume1", 100);
    EXPECT_TRUE(IsEqual(volume1->GetRoot(), cRawRoot1));

    virtRoot->Mount(volume1->GetRoot());
    EXPECT_TRUE(IsEqual(virtRoot, cRawRoot1));

    static const string cChildForRemove = "child";

	volume1->GetRoot()->RemoveChildIf(
        [&](auto node)
        {
            return node->GetName() == cChildForRemove;
		});

    auto root = cRawRoot1;
    root.children.erase(std::find_if(root.children.begin(), root.children.end(),
        [&](const auto& child)
        {
            return child.name == cChildForRemove;
        }));

    EXPECT_TRUE(IsEqual(virtRoot, root));
}

TEST_F(VirtualNodeTest, Mount_Several_Nodes)
{
    const auto virtRoot = m_storage.GetRoot();

    auto volume1 = CreateVolume(cRawRoot1, "Volume1", 200);
    auto volume2 = CreateVolume(cRawRoot2, "Volume2", 100);

    virtRoot->Mount(volume1->GetRoot());
    virtRoot->Mount(volume2->GetRoot());

    auto root = cRawRoot1;
    root.Merge(cRawRoot2);

    EXPECT_TRUE(IsEqual(virtRoot, root));

    virtRoot->UnmountIf(
        [](auto&)
        {
            return true;
        }
    );

    volume1 = CreateVolume(cRawRoot1, "Volume1", 100);
    volume2 = CreateVolume(cRawRoot2, "Volume2", 200);

    virtRoot->Mount(volume1->GetRoot());
    virtRoot->Mount(volume2->GetRoot());

    root = cRawRoot2;
    root.Merge(cRawRoot1);

    EXPECT_TRUE(IsEqual(virtRoot, root));
}

TEST_F(VirtualNodeTest, UnmountNode)
{
    const auto virtRoot = m_storage.GetRoot();

    const auto volume1 = CreateVolume(cRawRoot1, "Volume1", 100);
    const auto volume2 = CreateVolume(cRawRoot2, "Volume2", 200);

    virtRoot->Mount(volume1->GetRoot());
    virtRoot->Mount(volume2->GetRoot());

    auto root = cRawRoot2;
    root.Merge(cRawRoot1);
    EXPECT_TRUE(IsEqual(virtRoot, root));

    virtRoot->Unmount(volume1->GetRoot());

	EXPECT_TRUE(IsEqual(virtRoot, cRawRoot2));
}