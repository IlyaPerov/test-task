#include "TestTools.h"

using namespace std;
using namespace vs;


//
// RawNode
//

void RawNode::Merge(const RawNode& rhs)
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
			foundChild->Merge(rhsChild);
		else
			children.push_back(rhsChild);
	}
}

bool IsEqual(const RawNode& lhs, const RawNode& rhs, bool ignoreRootName = false)
{
	if ((ignoreRootName || (lhs.name == rhs.name))
		&& (lhs.values == rhs.values)
		&& (lhs.children.size() == rhs.children.size()))
	{
		struct Hasher :
			std::hash<std::string>
		{
			std::size_t operator()(const RawNode& raw) const noexcept
			{
				return std::hash<std::string>::operator()(raw.name);
			}
		};

		using RNSet = std::unordered_set<RawNode, Hasher>;

		RNSet s1, s2;
		s1.reserve(lhs.children.size());
		s2.reserve(lhs.children.size());

		s1.insert(lhs.children.begin(), lhs.children.end());
		s2.insert(rhs.children.begin(), rhs.children.end());

		//definetely not optimal, but...
		return s1 == s2;
	}

	return false;
}

bool RawNode::operator == (const RawNode& rhs) const
{
	return IsEqual(*this, rhs);
}
bool RawNode::operator != (const RawNode& rhs) const
{
	return !(*this == rhs);
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
	auto res = make_shared<VolumeType>(raw.name, priority);
	FillNode(res->GetRoot(), raw);

	return res;
}


////TODO: improve
//template<typename NodeT>
//testing::AssertionResult IsEqualImpl(const NodeT& node, const RawNode& raw)
//{
//	if (node->GetName() != raw.name)
//		return testing::AssertionFailure() << "names are different: \n"
//		<< "node name == '" << node->GetName() << "'\n"
//		<< "raw name == '" << raw.name << "'\n";
//
//	for (const auto& rawKeyValue : raw.values)
//	{
//		ValueVariant value;
//		if (!node->Find(rawKeyValue.first, value))
//		{
//			return testing::AssertionFailure() << "key ["
//				<< rawKeyValue.first << "] not found";
//		}
//
//		if (rawKeyValue.second != value)
//		{
//			return testing::AssertionFailure() << "values for key ["
//				<< rawKeyValue.first << "] are not equal:";// << value << " and " << it->second;
//		}
//	}
//
//	for (const auto& rawChild : raw.children)
//	{
//		auto child = node->FindChildIf(
//			[&](auto childNode)
//			{
//				if (childNode->GetName() == rawChild.name)
//				return true;
//
//		return false;
//			});
//
//		if (!child)
//			return testing::AssertionFailure() << "child with name '" << rawChild.name << "' not found";
//
//		auto res = IsEqualImpl(child, rawChild);
//		if (!res)
//			return res;
//	}
//
//	return testing::AssertionSuccess();
//
//}

template<typename NodeT>
RawNode ToRawNode(const NodeT& node)
{
	RawNode res {node->GetName()};
	node->ForEachKeyValue(
		[&res](const auto& key, auto& value)
		{
			res.values[key] = value;
		}
	);

	node->ForEachChild(
		[&res](auto child)
		{
			res.children.push_back(ToRawNode(child));
		}
	);
	return res;
}

// IsEqual
template <typename NodeT>
bool IsEqualImpl(const NodeT& node, const RawNode& raw)
{
	return IsEqual(ToRawNode(node), raw, true);
}

// IsEqual
bool IsEqual(const VolumeType::NodePtr& node, const RawNode& raw)
{
	return IsEqualImpl(node, raw);
}

// IsEqual
bool IsEqual(const StorageType::NodePtr& node, const RawNode& raw)
{
	return IsEqualImpl(node, raw);
}