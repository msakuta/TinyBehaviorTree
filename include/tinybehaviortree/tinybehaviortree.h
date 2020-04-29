#ifndef TINYBEHAVIORTREE_H
#define TINYBEHAVIORTREE_H

#include <memory>
#include <vector>
#include <tuple>

namespace TBT{

enum class BehaviorResult {
	IDLE = 0,
	RUNNING,
	SUCCESS,
	FAILURE
};

class BehaviorNodeBase{
public:

protected:
	BehaviorNodeBase* parent = nullptr;
	static void setParent(BehaviorNodeBase* child, BehaviorNodeBase* parent){child->parent = parent;}
};

template<typename... Payload>
class BehaviorNode : public BehaviorNodeBase {
public:
	using This = BehaviorNode<Payload...>;
	This(){}
	This(const This&) = delete;
	virtual BehaviorResult tick(Payload...) = 0;
};

template<typename... Payload>
class SequenceNode : public BehaviorNode<Payload...> {
public:
	SequenceNode(){}
	SequenceNode(const SequenceNode&) = delete;
	void addChild(std::unique_ptr<This>&& node) {
		setParent(&*node, this);
		children.push_back(std::move(node));
	}
	BehaviorResult tick(Payload... payload) override {
		for (auto it = children.begin(); it != children.end(); ++it) {
			auto& node = *it;
			auto result = node->tick(payload...);
			if(result == BehaviorResult::FAILURE)
				return BehaviorResult::FAILURE;
		}
		return BehaviorResult::SUCCESS;
	}
protected:
	std::vector<std::unique_ptr<This>> children;
};


template<typename... Payload>
class PeelNodeBase : public BehaviorNode<Payload...> {
public:
	using TuplePayload = std::tuple<Payload...>;
	BehaviorResult tick(Payload... payload) override {
		return tickTuple(TuplePayload(payload...));
	}

	virtual BehaviorResult tickTuple(TuplePayload&) = 0;
};

// Template metaprogramming helpers to unpack tuple into argument list
template<int ...> struct Seq_ {};
template<int N, int ...S> struct Gens_ : Gens_<N - 1, N - 1, S...> { };
template<int ...S> struct Gens_<0, S...>{ typedef Seq_<S...> type; };


#define TBT_ARGS(...) __VA_ARGS__


#define TBT_PEEL_NODE_MACRO(name, base, sub, func) \
namespace TBT{ \
template<typename SuperNode, typename... SubPayload> \
class name ## PeelNodeMacro : public SuperNode { \
public: \
	using SubNode = BehaviorNode<sub>; \
	using TuplePayload = typename SuperNode::TuplePayload; \
	name ## PeelNodeMacro(){} \
	name ## PeelNodeMacro(const name ## PeelNodeMacro&) = delete; \
	void setChild(std::unique_ptr<SubNode>&& node) { \
		SuperNode::setParent(&*node, this); \
		child = std::move(node); \
	} \
\
	std::tuple<sub> peel(TuplePayload& payload) { \
		return func; \
	} \
\
	template<int ...S>\
	BehaviorResult callFunc(Seq_<S...>, std::tuple<sub>& params) \
	{ \
		return child->tick(std::get<S>(params) ...); \
	} \
\
	BehaviorResult tickTuple(TuplePayload& payload) override { \
		auto peeled = peel(payload); \
		return callFunc(typename Gens_<sizeof...(SubPayload)>::type(), peeled); \
	} \
protected: \
	std::unique_ptr<SubNode> child; \
};\
} /* namespace TBT */\
using name = name ## PeelNodeMacro<PeelNodeBase<base>, sub>;

template<typename... Payload>
class BehaviorTree {
public:
	BehaviorTree(){}
	void setRoot(std::unique_ptr<BehaviorNode<Payload...>>&& node) {
		rootNode = std::move(node);
	}

	void tickRoot(Payload... payload) {
		if(rootNode){
			rootNode->tick(payload...);
		}
	}
protected:
	std::unique_ptr<BehaviorNode<Payload...>> rootNode;
};

}

#endif // TINYBEHAVIORTREE_H
