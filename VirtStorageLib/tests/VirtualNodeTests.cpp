#include "gtest/gtest.h"

#include "Storage.h"
#include "Volume.h"
#include "TestTools.h"
#include "TestData.h"

using namespace std;
using namespace vs;
using namespace test_tools;

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

TEST_F(VirtualNodeTest, InsertChild_Remove_Find)
{
    const auto root = m_storage.GetRoot();

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

TEST_F(VirtualNodeTest, RemovedChild_Produces_Exception)
{
    const auto root = m_storage.GetRoot();

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

TEST_F(VirtualNodeTest, MountNode)
{
    const auto virtRoot = m_storage.GetRoot();

	const auto volume1 = CreateVolume(cRawRoot1, 100);
    EXPECT_TRUE(IsEqual(volume1.GetRoot(), cRawRoot1));

	virtRoot->Mount(volume1.GetRoot());
    EXPECT_TRUE(IsEqual(virtRoot, cRawRoot1));
}

TEST_F(VirtualNodeTest, MountNode_To_Some_Child)
{
    const auto virtRoot = m_storage.GetRoot();

    const auto volume1 = CreateVolume(cRawRoot1, 100);
    EXPECT_TRUE(IsEqual(volume1.GetRoot(), cRawRoot1));

    const auto virtChild = virtRoot->InsertChild("newchild1")->InsertChild("newchild12");
    virtChild->Mount(volume1.GetRoot());
    EXPECT_TRUE(IsEqual(virtChild, cRawRoot1));

    RawNode rawRoot;
    rawRoot.children.push_back({ "newchild1" });
    auto& newChild1 = rawRoot.children.back();
    newChild1.children.push_back({ "newchild12" });
    auto& newChild12 = newChild1.children.back();
    newChild12.Merge(cRawRoot1);
	EXPECT_TRUE(IsEqual(virtRoot, rawRoot));
}

TEST_F(VirtualNodeTest, Remove_Child_After_Mount)
{
    const auto virtRoot = m_storage.GetRoot();

    const auto volume1 = CreateVolume(cRawRoot1, 100);
    EXPECT_TRUE(IsEqual(volume1.GetRoot(), cRawRoot1));

    virtRoot->Mount(volume1.GetRoot());
    EXPECT_TRUE(IsEqual(virtRoot, cRawRoot1));

    static const string cChildForRemove = "child";

	volume1.GetRoot()->RemoveChildIf(
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

TEST_F(VirtualNodeTest, Add_Child_After_Mount)
{
    const auto virtRoot = m_storage.GetRoot();

    const auto volume1 = CreateVolume(cRawRoot1, 100);
    EXPECT_TRUE(IsEqual(volume1.GetRoot(), cRawRoot1));

    virtRoot->Mount(volume1.GetRoot());
    EXPECT_TRUE(IsEqual(virtRoot, cRawRoot1));

    static const string cChildToAdd = "NewChild";

    const auto newChild = volume1.GetRoot()->InsertChild(cChildToAdd);
    for (int i = 0; i < 3; i++)
        newChild->Insert(i, i * 100);

    auto root = cRawRoot1;
    root.children.push_back(ToRawNode(newChild));

    EXPECT_TRUE(IsEqual(virtRoot, root));
}

TEST_F(VirtualNodeTest, Mount_Several_Nodes)
{
    const auto virtRoot = m_storage.GetRoot();

    auto volume1 = CreateVolume(cRawRoot1, 200);
    auto volume2 = CreateVolume(cRawRoot2, 100);

    virtRoot->Mount(volume1.GetRoot());
    virtRoot->Mount(volume2.GetRoot());

    auto root = cRawRoot1;
    root.Merge(cRawRoot2);

    EXPECT_TRUE(IsEqual(virtRoot, root));

    virtRoot->UnmountIf(
        [](auto)
        {
            return true;
        }
    );

    volume1 = CreateVolume(cRawRoot1, 100);
    volume2 = CreateVolume(cRawRoot2, 200);

    virtRoot->Mount(volume1.GetRoot());
    virtRoot->Mount(volume2.GetRoot());

    root = cRawRoot2;
    root.Merge(cRawRoot1);

    EXPECT_TRUE(IsEqual(virtRoot, root));
}

TEST_F(VirtualNodeTest, Mount_To_Several_Storages)
{
	const StorageType virtStorage1{ "VirtRoot1" };
	const StorageType virtStorage2{ "VirtRoot2" };

    const auto virtRoot1 = virtStorage1.GetRoot();
    const auto virtRoot2 = virtStorage2.GetRoot();

    RawNode rawRoot = cRawRoot1;
	auto volume = CreateVolume(rawRoot, 100);

    virtRoot1->Mount(volume.GetRoot());
    virtRoot2->Mount(volume.GetRoot());

    EXPECT_TRUE(IsEqual(virtRoot1, virtRoot2));

    const auto childToChange = "child";
    const auto newValue = "new value!";

    // change child/values from one virtual node...
	virtRoot1->FindChildIf(
        [&childToChange](auto node)
        {
            return node->GetName() == childToChange;
        }
    )->Insert(1000, "new value!");

    std::find_if(rawRoot.children.begin(), rawRoot.children.end(),
        [&](const auto& child)
        {
            return child.name == childToChange;
        })->values[1000] = newValue;

	EXPECT_TRUE(IsEqual(virtRoot1, rawRoot));
	EXPECT_TRUE(IsEqual(virtRoot1, virtRoot2)); //... and see these changes in another virtual node
}

TEST_F(VirtualNodeTest, UnmountNode)
{
    const auto virtRoot = m_storage.GetRoot();

    const auto volume1 = CreateVolume(cRawRoot1, 100);
    const auto volume2 = CreateVolume(cRawRoot2, 200);

    virtRoot->Mount(volume1.GetRoot());
    virtRoot->Mount(volume2.GetRoot());

    auto root = cRawRoot2;
    root.Merge(cRawRoot1);
    EXPECT_TRUE(IsEqual(virtRoot, root));

    virtRoot->Unmount(volume1.GetRoot());

	EXPECT_TRUE(IsEqual(virtRoot, cRawRoot2));
}