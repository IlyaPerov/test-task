#pragma once

#include "Storage.h"
#include "Volume.h"

namespace test_tools
{

using KeyType = int;
using ValueType = vs::ValueVariant;

using VolumeType = vs::Volume<KeyType, ValueType>;
using StorageType = vs::Storage<KeyType, ValueType>;

struct RawNode
{
	using DictType = std::unordered_map<KeyType, ValueType>;
	using ChildrenContainerType = std::list<RawNode>;

	std::string name;
	DictType values;
	ChildrenContainerType children;

	void Merge(const RawNode& raw);
	bool operator == (const RawNode& rhs) const;
	bool operator != (const RawNode& rhs) const;
};

// IsEqual
bool IsEqual(const RawNode& lhs, const RawNode& rhs, bool ignoreRootName = false);

// CreateVolume
VolumeType CreateVolume(const RawNode& raw, vs::Priority priority);

// ToRawNode
template<typename NodeT>
RawNode ToRawNode(const NodeT& node)
{
	RawNode res{ node->GetName() };
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
bool IsEqual(const NodeT& node, const RawNode& raw)
{
	return IsEqual(ToRawNode(node), raw, true); // ignores root names
}

template <typename NodeT1, typename NodeT2>
bool IsEqual(const NodeT1& node1, const NodeT2& node2)
{
	return IsEqual(ToRawNode(node1), ToRawNode(node2), true); // ignores root names
}



} // namespace test_tools
