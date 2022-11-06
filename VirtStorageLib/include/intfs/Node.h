#pragma once

template<typename KeyT, typename ValueHolderT>
struct INode
{
    virtual ~INode() = default;

    virtual const std::string& GetName() const = 0;

    virtual void Insert(const KeyT& key, const ValueHolderT& value) = 0;
    virtual void Insert(const KeyT& key, ValueHolderT&& value) = 0;
    virtual void Erase(const KeyT& key) = 0;
    virtual bool Find(const KeyT& key, ValueHolderT& value) const = 0;
    virtual bool Replace(const KeyT& key, const ValueHolderT& value) = 0;
    virtual bool Replace(const KeyT& key, ValueHolderT&& value) = 0;
};