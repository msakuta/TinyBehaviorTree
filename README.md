# TinyBehaviorTree
An experimental C++17 header only library to support behavior tree in minimal footprint


## Overview

This project is inspired by [BehaviorTreeCPP](https://github.com/BehaviorTree/BehaviorTree.CPP.git),
but tries to avoid overhead and make the tree as compact as possible.

## Problems of BehaviorTreeCPP

BehaviorTreeCPP has a lot of housekeeping variables in each node, which makes even the
simplest node have hundreds of bytes.
It was 480 bytes in my particular implementation (Visual Studio 2019).
On the other hand, a node in this library merely has minimum 16 bytes (2 pointers) in a 64 bit computer.
Of course it would be bigger if you have additional features like subnodes.
 
Another problem is that it's not easy to pass around common variables among multiple nodes.
It has input/output ports and blackboard, but they're hardly intuitive and, most importantly,
not scalable.
The blackboard is essentially a big global namespace that you need to care about name collisions,
which undermines the point of composability and reusability of individual node.
Furthermore, accessing entries in a blackboard is not very efficient -- you need to
lookup entry by a string *for every iteration for every node*.

It's also not very efficient when you want to initialize nodes with custom data
before ticking the tree.
In fact, the [official tutorial](https://www.behaviortree.dev/tutorial_08_additional_args/)
shows looping all the nodes in the tree and retrieve compatible node by `dynamic_cast`!
If you don't know how terrible this is, 1. dynamic_cast has overhead and 2. looping
all the nodes essentially scans the tree twice for nothing.

```cpp
    // Iterate through all the nodes and call init() if it is an Action_B
    for( auto& node: tree.nodes )
    {
        if( auto action_B_node = dynamic_cast<Action_B*>( node.get() ))
        {
            action_B_node->init( 69, 9.99, "interesting_value" );
        }
    }

    tree.tickRoot();
```


## Our solution

We don't need to pass around data with ports nor `dynamic_cast`s in TinyBehaviorTree.
We can simply call the tree with 

```cpp
    tree.tickRoot(69, 9.99, "interesting_value");
```

and the parameters will be passed down to each node's tick() function via arguments, like

```cpp
class Action_B: public BehaviorNode<int, double, std::string>
{
    BehaviorResult tick(int arg1, double arg2, std::string arg3) override
    {
        std::cout << "Action_B: " << arg1 << " / " << arg2 << " / "
                  << arg3 << std::endl;
        return BehaviorResult::SUCCESS;
    }
};
```

Passing down data as arguments has another advantage that you won't need to
store the excess information into each node.
It also helps to keep the size of each node small, making cache hit rate high
and general performance better.

The drawback of this approach is that we need to put argument type information
as part of the node and tree types (which is a cost in exchange of no runtime overhead).
So, if two nodes have different arguments, they cannot be part of the same tree,
unless you do the trick below.


## Heterogeneous tree

Even if you have nodes with different argument types, you can compose them into a single
tree using "Peel" nodes, that will transform data in the parent node into what
a child node can accept.
We call this node PeelNode because usually child node sees smaller view of parent node's accessible data.

Suppose we are developing two-armed robot and want to design a behavior node that processes
either one of the arms.
The data we are passing to the entire tree is like this

```cpp
struct Arm {
    std::string name;
};

struct Body {
    Arm leftArm = {"leftArm"};
    Arm rightArm = {"righttArm"};
};
```

We can define the node to process an arm:

```cpp
class PrintArmNode : public BehaviorNode<Arm&> {
public:
    BehaviorResult tick(Arm& arm) override {
        std::cout << arm.name << "\n";
        return BehaviorResult::SUCCESS;
    }
};
```

But the entire tree should accept Body as the argument.
How do we do it?

The answer is to define PeelNodes for left and right arms.
There is a macro `TBT_PEEL_NODE_MACRO` to simplify this process.

```cpp
TBT_PEEL_NODE_MACRO(PeelLeftArm, Body&, Arm&, {std::get<0>(payload).leftArm})
TBT_PEEL_NODE_MACRO(PeelRightArm, Body&, Arm&, {std::get<0>(payload).rightArm})
```

The first argument to this macro is the name of PeelNode, the second is parent node's argument types
(if you pass multiple arguments, you can group them by `TBT_ARGS()` macro, note that parentheses will
not work!), the third is child node's argument types and the fourth argument is conversion code.

Since the macro is implemented with extensive template metaprogramming and macros, it has special
way to write conversion code.

* It has to be evaluated as a tuple of argument types for child node.
* The parent arguments are passed as a tuple named `payload`, which you can operate on with `get<n>()`.

Once you define these PeelNodes, you can use them to compose the same `PrintArmNode` to
both arms.

```cpp
    BehaviorTree<Body&> tree;

    auto rootNode = std::make_unique<SequenceNode<Body&>>();
    auto peelLeftArm = std::make_unique<PeelLeftArm>();
    peelLeftArm->setChild(std::make_unique<PrintArmNode>());
    rootNode->addChild(std::move(peelLeftArm));
    auto peelRightArm = std::make_unique<PeelRightArm>();
    peelRightArm->setChild(std::make_unique<PrintArmNode>());
    rootNode->addChild(std::move(peelRightArm));

    tree.setRoot(std::move(rootNode));
```

The tree structure will be:

* tree
  * SequenceNode
    * PeelLeftArm
      * PrintArmNode
    * PeelRightNode
      * PrintArmNode

And the output of `tree.tickRoot()` will be:

    leftArm
    rightArm

## Non-goals

* Full-featured behavior tree for production
* Asynchronous nodes/coroutines
* Dynamic reconfiguration

