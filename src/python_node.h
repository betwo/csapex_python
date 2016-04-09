#ifndef PYTHON_NODE_H
#define PYTHON_NODE_H

/// PROJECT
#include <csapex/model/tickable_node.h>
#include <csapex/model/variadic_io.h>

/// SYSTEM
#include <boost/python.hpp>

namespace csapex
{

class PythonNode : public TickableNode, public Variadic
{
public:
    PythonNode();
    ~PythonNode();

    std::string getCode() const;
    void setCode(const std::string& code);

    virtual void setup(csapex::NodeModifier& node_modifier) override;
    virtual void setupParameters(Parameterizable &parameters) override;
    virtual bool canTick() override;
    virtual void tick() override;
    virtual void process() override;
    virtual void processMarker(const connection_types::MessageConstPtr &marker) override;

    virtual void portCountChanged() override;


private:
    void refreshCode();

    void flush();
    bool exists(const std::string& method);
    void call(const std::string& method);

private:
    std::string code_;
    bool is_setup_;
    bool python_is_initialized_;

    PyThreadState* thread_state;
    boost::python::object globals;
    boost::python::dict locals;
};

}

#endif // PYTHON_NODE_H
