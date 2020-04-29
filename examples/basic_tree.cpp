#include "../include/tinybehaviortree/tinybehaviortree.h"


#include <string>
#include <iostream>

using namespace TBT;


struct Arm {
    std::string name;
};

struct Body {
    Arm leftArm = {"leftArm"};
    Arm rightArm = {"righttArm"};
};

class PrintArmNode : public BehaviorNode<Arm&> {
public:
    BehaviorResult tick(Arm& arm) override {
        std::cout << arm.name << "\n";
        return BehaviorResult::SUCCESS;
    }
};

TBT_PEEL_NODE_MACRO(PeelLeftArm, Body&, Arm&, {std::get<0>(payload).leftArm})
TBT_PEEL_NODE_MACRO(PeelRightArm, Body&, Arm&, {std::get<0>(payload).rightArm})


int main()
{
    Body body;


    BehaviorTree<Body&> tree;

    auto rootNode = std::make_unique<SequenceNode<Body&>>();
    auto peelLeftArm = std::make_unique<PeelLeftArm>();
    peelLeftArm->setChild(std::make_unique<PrintArmNode>());
    rootNode->addChild(std::move(peelLeftArm));
    auto peelRightArm = std::make_unique<PeelRightArm>();
    peelRightArm->setChild(std::make_unique<PrintArmNode>());
    rootNode->addChild(std::move(peelRightArm));

    tree.setRoot(std::move(rootNode));

    tree.tickRoot(body);

    return 0;
}

/* Expected output:
*
    leftArm
    rightArm
*/
