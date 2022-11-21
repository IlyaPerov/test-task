#pragma once

#include "gtest/gtest-assertion-result.h"

#include "Storage.h"
#include "Volume.h"

using KeyType = int;
using ValueType = vs::ValueVariant;

using DictType = std::unordered_map<KeyType, ValueType>;

using VolumeType = vs::Volume<KeyType, ValueType>;
using StorageType = vs::Storage<KeyType, ValueType>;

struct RawNode
{
	std::string name;
	DictType values;
	std::list<RawNode> children;

	RawNode& operator += (const RawNode& rhs);
	RawNode operator + (const RawNode& rhs) const;
	bool operator == (const RawNode& rhs) const
	{
		return (name == rhs.name) && (values == rhs.values) && (children == rhs.children);
	}
	bool operator != (const RawNode& rhs) const
	{
		return !(*this == rhs);
	}
};

std::shared_ptr<VolumeType> CreateVolume(const RawNode& raw, std::string name, vs::Priority priority);

testing::AssertionResult IsEqual(const VolumeType::NodePtr& node, const RawNode& raw);
testing::AssertionResult IsEqual(const StorageType::NodePtr& node, const RawNode& raw);