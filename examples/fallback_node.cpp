#include "tinybehaviortree/tinybehaviortree.h"


#include <string>
#include <iostream>

using namespace TBT;


struct Door {
    bool open = false;
    bool locked = false;
};

struct Agent {
    bool hasKey = false;
};

class IsDoorOpen : public BehaviorNode<Door&> {
public:
    BehaviorResult tick(Door& door) override {
        std::cout << "The door is " << (door.open ? "open" : "closed") << ".\n";
        return door.open ? BehaviorResult::SUCCESS : BehaviorResult::FAILURE;
    }
};

class OpenDoor : public BehaviorNode<Door&> {
public:
    BehaviorResult tick(Door& door) override {
        if(!door.locked){
            door.open = true;
            std::cout << "Door opened!\n";
            return BehaviorResult::SUCCESS;
        }
        else{
            std::cout << "Door was unable to open because it's locked!\n";
            return BehaviorResult::FAILURE;
        }
    }
};

class HaveKey : public BehaviorNode<Agent&> {
public:
    BehaviorResult tick(Agent& agent) override {
        return agent.hasKey ? BehaviorResult::SUCCESS : BehaviorResult::FAILURE;
    }
};

class UnlockDoor : public BehaviorNode<Door&> {
public:
    BehaviorResult tick(Door& door) override {
        door.locked = false;
        std::cout << "Door unlocked!\n";
        return BehaviorResult::SUCCESS;
    }
};

class SmashDoor : public BehaviorNode<Door&> {
public:
    BehaviorResult tick(Door& door) override {
        std::cout << "You smashed the door, but it didn't move a bit.\n";
        return BehaviorResult::FAILURE;
    }
};

class EnterRoom : public BehaviorNode<Agent&, Door&> {
public:
    BehaviorResult tick(Agent& agent, Door& door) override {
        std::cout << "You entered the room. Congrats!\n";
        return BehaviorResult::SUCCESS;
    }
};


TBT_PEEL_NODE_MACRO(PeelAgent, TBT_ARGS(Agent&, Door&), Agent&, {std::get<0>(payload)})
TBT_PEEL_NODE_MACRO(PeelDoor, TBT_ARGS(Agent&, Door&), Door&, {std::get<1>(payload)})


int main()
{


    auto wrapPeelAgent = [](std::unique_ptr<BehaviorNode<Agent&>>&& child){
        auto peelAgent = std::make_unique<PeelAgent>();
        peelAgent->setChild(std::move(child));
        return std::move(peelAgent);
    };
    auto wrapPeelDoor = [](std::unique_ptr<BehaviorNode<Door&>>&& child){
        auto peelDoor = std::make_unique<PeelDoor>();
        peelDoor->setChild(std::move(child));
        return std::move(peelDoor);
    };

    auto rootNode = std::make_unique<SequenceNode<Agent&, Door&>>();
        auto tryOpenDoorNode = std::make_unique<FallbackNode<Agent&, Door&>>();
            tryOpenDoorNode->addChild(wrapPeelDoor(std::make_unique<IsDoorOpen>()));
            tryOpenDoorNode->addChild(wrapPeelDoor(std::make_unique<OpenDoor>()));

            auto tryUnlockNode = std::make_unique<SequenceNode<Agent&, Door&>>();
                tryUnlockNode->addChild(wrapPeelAgent(std::make_unique<HaveKey>()));
                tryUnlockNode->addChild(wrapPeelDoor(std::make_unique<UnlockDoor>()));
                tryUnlockNode->addChild(wrapPeelDoor(std::make_unique<OpenDoor>()));
            tryOpenDoorNode->addChild(std::move(tryUnlockNode));
        tryOpenDoorNode->addChild(wrapPeelDoor(std::make_unique<SmashDoor>()));
    rootNode->addChild(std::move(tryOpenDoorNode));
    rootNode->addChild(std::make_unique<EnterRoom>());

    BehaviorTree<Agent&, Door&> tree;
    tree.setRoot(std::move(rootNode));

    auto tryScenario = [&tree](Agent agent, Door door){
        tree.tickRoot(agent, door);
    };

    // The easiest case. The door is open.
    std::cout << "\n# First scenario...\n";
    tryScenario(Agent{ false }, Door{ true, false });

    // The door has been closed, you need to open it before entering.
    std::cout << "\n# Second scenario...\n";
    tryScenario(Agent{ false }, Door{ false, false } );

    // Oh shit, the door has been locked!
    std::cout << "\n# Third scenario...\n";
    tryScenario(Agent{ false }, Door{ false, true } );

    // We got a key!
    std::cout << "\n# Fourth scenario...\n";
    tryScenario(Agent{ true }, Door{ false, true } );

    return 0;
}

/* Expected output:
*
# First scenario...
The door is open.
You entered the room. Congrats!

# Second scenario...
The door is closed.
Door opened!
You entered the room. Congrats!

# Third scenario...
The door is closed.
Door was unable to open because it's locked!
You smashed the door, but it didn't move a bit.

# Fourth scenario...
The door is closed.
Door was unable to open because it's locked!
Door unlocked!
Door opened!
You entered the room. Congrats!
*/
