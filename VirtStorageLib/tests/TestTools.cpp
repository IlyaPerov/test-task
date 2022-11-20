#include "TestTools.h"

using namespace std;
using namespace vs;


//
// RawNode
//

RawNode& RawNode::operator += (const RawNode& rhs)
{
	values.insert(rhs.values.begin(), rhs.values.end());

	for (const auto& rhsChild : rhs.children)
	{
		auto foundChild = std::find_if(children.begin(), children.end(),
			[&](const auto& child)
			{
				if (child.name == rhsChild.name)
				return true;
		return false;
			});

		if (foundChild != children.end())
			foundChild->operator+=(rhsChild);
		else
			children.push_back(rhsChild);
	}

	return *this;
}

RawNode RawNode::operator + (const RawNode& rhs) const
{
	auto res = *this;
	res += rhs;

	return res;
}

void FillNode(const VolumeType::NodePtr& node, const RawNode& from)
{
	for (const auto& v : from.values)
		node->TryInsert(v.first, v.second);

	for (const auto& c : from.children)
	{
		const auto child = node->AddChild(c.name);
		FillNode(child, c);
	}
}

// CreateVolume
std::shared_ptr<VolumeType> CreateVolume(const RawNode& raw, std::string name, Priority priority)
{
	auto res = make_shared<VolumeType>(priority, std::move(name), raw.name);
	FillNode(res->GetRoot(), raw);

	return res;
}


//TODO: improve
template<typename NodeT>
testing::AssertionResult IsEqualImpl(const NodeT& node, const RawNode& raw)
{
	if (node->GetName() != raw.name)
		return testing::AssertionFailure() << "names are different: \n"
		<< "node name == '" << node->GetName() << "'\n"
		<< "raw name == '" << raw.name << "'\n";

	for (const auto& rawKeyValue : raw.values)
	{
		ValueVariant value;
		if (!node->Find(rawKeyValue.first, value))
		{
			return testing::AssertionFailure() << "key ["
				<< rawKeyValue.first << "] not found";
		}

		if (rawKeyValue.second != value)
		{
			return testing::AssertionFailure() << "values for key ["
				<< rawKeyValue.first << "] are not equal:";// << value << " and " << it->second;
		}
	}

	for (const auto& rawChild : raw.children)
	{
		auto child = node->FindChildIf(
			[&](auto childNode)
			{
				if (childNode->GetName() == rawChild.name)
				return true;

		return false;
			});

		if (!child)
			return testing::AssertionFailure() << "child with name '" << rawChild.name << "' not found";

		auto res = IsEqualImpl(child, rawChild);
		if (!res)
			return res;
	}

	return testing::AssertionSuccess();

}

// IsEqual
testing::AssertionResult IsEqual(const VolumeType::NodePtr& node, const RawNode& raw)
{
	return IsEqualImpl(node, raw);
}

// IsEqual
testing::AssertionResult IsEqual(const StorageType::NodePtr& node, const RawNode& raw)
{
	return IsEqualImpl(node, raw);
}