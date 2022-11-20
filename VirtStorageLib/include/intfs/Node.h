#pragma once

#include <functional>

namespace vs
{

template<typename KeyT, typename ValueHolderT>
struct INode
{
	using ForEachKeyValueFunctorType = std::function<void(const KeyT&, ValueHolderT&)>;

	virtual ~INode() = default;

	virtual const std::string& GetName() const = 0;

	virtual void Insert(const KeyT& key, const ValueHolderT& value) = 0;
	virtual void Insert(const KeyT& key, ValueHolderT&& value) = 0;
	virtual void Erase(const KeyT& key) = 0;
	virtual bool Find(const KeyT& key, ValueHolderT& value) const = 0;
	virtual bool Contains(const KeyT& key) const = 0;
	virtual bool TryInsert(const KeyT& key, const ValueHolderT& value) = 0;
	virtual bool TryInsert(const KeyT& key, ValueHolderT&& value) = 0;
	virtual bool Replace(const KeyT& key, const ValueHolderT& value) = 0;
	virtual bool Replace(const KeyT& key, ValueHolderT&& value) = 0;
	virtual void ForEachKeyValue(const ForEachKeyValueFunctorType& f) = 0;
};

} //namespace vs