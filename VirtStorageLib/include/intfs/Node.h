#pragma once

template<typename KeyT, typename ValueHolderT>
struct INode
{
    virtual ~INode() = default;

    virtual const std::string& GetName() const noexcept = 0;
    
    virtual void Insert(const KeyT& Key, const ValueHolderT& Value) = 0;
    virtual void Insert(const KeyT& Key, ValueHolderT&& Value) = 0;
    virtual void Erase(const KeyT& Key) = 0;
    virtual bool Find(const KeyT& Key, ValueHolderT& Value) const = 0;
    virtual bool Replace(const KeyT& Key, const ValueHolderT& Value) = 0;
    virtual bool Replace(const KeyT& Key, ValueHolderT&& Value) = 0;
};