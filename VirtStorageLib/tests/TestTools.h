#pragma once

#include "gtest/gtest-assertion-result.h"

#include "Storage.h"
#include "Volume.h"

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



std::shared_ptr<VolumeType> CreateVolume(const RawNode& raw, std::string name, vs::Priority priority);

bool IsEqual(const VolumeType::NodePtr& node, const RawNode& raw);
bool IsEqual(const StorageType::NodePtr& node, const RawNode& raw);