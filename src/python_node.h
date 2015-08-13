#ifndef PYTHON_NODE_H
#define PYTHON_NODE_H

/// PROJECT
#include <csapex/model/tickable_node.h>

/// SYSTEM
#include <boost/python.hpp>

using namespace boost::python;

namespace csapex
{

class PythonNode : public TickableNode
{
public:
    PythonNode();
    ~PythonNode();

    std::string getCode() const;
    void setCode(const std::string& code);

    virtual void setup(csapex::NodeModifier& node_modifier) override;
    virtual bool canTick() override;
    virtual void tick() override;
    virtual void process() override;

private:
    std::string code_;
    bool init_;

    PyThreadState* thread_state;
    object globals;
    dict locals;
};

}

#endif // PYTHON_NODE_H
