#pragma once

#include <memory>
#include <functional>

namespace vs
{

template<typename NodeT>
struct INodeContainer
{
	using NodeType = NodeT;
	using NodePtr = std::shared_ptr<NodeType>;
	using NodeWeakPtr = typename std::shared_ptr<NodeT>::weak_type;

	using ForEachFunctorType = std::function<void(NodePtr)>;
	using FindIfFunctorType = std::function<bool(NodePtr)>;
	using RemoveIfFunctorType = FindIfFunctorType;

	virtual ~INodeContainer() = default;

	virtual NodePtr InsertChild(const std::string& name) = 0;
	virtual void ForEachChild(const ForEachFunctorType& f) = 0;
	virtual NodePtr FindChild(const std::string& name) = 0;
	virtual NodePtr FindChildIf(const FindIfFunctorType& f) = 0;
	virtual void RemoveChildIf(const RemoveIfFunctorType& f) = 0;
};

} //namespace vs