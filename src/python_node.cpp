/// HEADER
#include "python_node.h"

/// PROJECT
#include <csapex/utility/register_apex_plugin.h>
#include <csapex/model/node_handle.h>
#include <csapex/msg/any_message.h>
#include <csapex/serialization/serialization.h>
#include <csapex/msg/no_message.h>
#include <csapex/msg/end_of_sequence_message.h>
#include <csapex/msg/end_of_program_message.h>

/// SYSTEM
#include <yaml-cpp/yaml.h>

CSAPEX_REGISTER_CLASS(csapex::PythonNode, csapex::Node)

using namespace csapex;
namespace bp = boost::python;

PythonNode::PythonNode()
    : is_setup_(false), python_is_initialized_(false)
{
    std::string def_code = "def setup(): \n"
                           "  print(inputs)\n"
                           "  print(outputs)\n"
                           "  input = inputs[0]\n"
                           "def process(): \n"
                           "  print(inputs)\n"
                           "  print(outputs)\n"
                           "  a = csapex.getString(inputs[0])\n"
                           "  csapex.publishString(outputs[0], a)\n"
            ;
    setCode(def_code);
}

PythonNode::~PythonNode()
{
    PyEval_AcquireThread(thread_state);

    Py_EndInterpreter(thread_state);

    PyEval_ReleaseLock();
}

void PythonNode::portCountChanged()
{
    refreshCode();
}

std::string PythonNode::getCode() const
{
    return code_;
}

void PythonNode::setCode(const std::string &code)
{
    code_ = code;

    static bool init = false;
    if(!init) {
        init = true;
        Py_Initialize();
        PyEval_InitThreads();
    }

    if(!python_is_initialized_) {
        thread_state = nullptr;
        PyThreadState *current_state = PyThreadState_Swap(nullptr);
        if (current_state == nullptr) {
            PyEval_AcquireLock();
        }

        thread_state = Py_NewInterpreter();

        bp::object main = bp::import("__main__");
        globals = main.attr("__dict__");

        globals["csapex"] = bp::handle<>(PyImport_ImportModule("csapex"));

        python_is_initialized_ = true;

    } else {
        PyEval_AcquireThread(thread_state);
    }

    if(node_handle_) {
        try {
            bp::list inputs;
            for(InputPtr& i : node_handle_->getExternalInputs()) {
                if(!node_handle_->isParameterInput(i.get())) {
                    inputs.append(i);
                }
            }
            globals["inputs"] = inputs;

            bp::list outputs;
            for(OutputPtr& o : node_handle_->getExternalOutputs()) {
                if(!node_handle_->isParameterOutput(o.get())) {
                    outputs.append(o);
                }
            }
            globals["outputs"] = outputs;

            bp::exec(code_.c_str(), globals, globals);

            flush();

            setTickEnabled(globals.contains("tick"));

            is_setup_ = true;

        } catch( bp::error_already_set ) {
            PyErr_Print();
        }
    }

    PyEval_ReleaseThread(thread_state);
}

void PythonNode::refreshCode()
{
    setCode(getCode());
}

void PythonNode::setup(NodeModifier& node_modifier)
{
    setupVariadic(node_modifier);

    if(exists("setup")) {
        call("setup");
    }

    refreshCode();

    setTickEnabled(false);
}

void PythonNode::setupParameters(Parameterizable &parameters)
{
    setupVariadicParameters(parameters);
}

bool PythonNode::canTick()
{
    return is_setup_;
}

void PythonNode::flush()
{
    bp::exec("import sys\n"
             "sys.stdout.flush()\n", globals, globals);
}

void PythonNode::tick()
{
    if(exists("tick")) {
        call("tick");
    }
}

void PythonNode::call(const std::string& method)
{
    PyEval_AcquireThread(thread_state);

    try {
        globals[method]();

        flush();

        std::cout << std::flush;
        std::cerr << std::flush;
        std::clog << std::flush;

    } catch( bp::error_already_set ) {
        PyErr_Print();
    }

    PyEval_ReleaseThread(thread_state);
}

bool PythonNode::exists(const std::string &method)
{
    PyEval_AcquireThread(thread_state);

    bool res = false;
    try {
        res = is_setup_ && globals.contains(method) != bp::object();
    } catch( bp::error_already_set ) {
        PyErr_Print();
    }

    PyEval_ReleaseThread(thread_state);

    return res;
}

void PythonNode::process()
{
    if(exists("process")) {
        call("process");
    }
}


void PythonNode::processMarker(const connection_types::MessageConstPtr &marker)
{
    if(std::dynamic_pointer_cast<connection_types::NoMessage const>(marker)) {
        if(exists("processNoMessage")) {
            call("processNoMessage");
        }

    } else if(std::dynamic_pointer_cast<connection_types::EndOfProgramMessage const>(marker)) {
        if(exists("processEndOfProgram")) {
            call("processEndOfProgram");
        }

    } else if(std::dynamic_pointer_cast<connection_types::EndOfSequenceMessage const>(marker)) {
        if(exists("processEndOfSequence")) {
            call("processEndOfSequence");
        }
    }
}


namespace csapex
{
class PythonNodeSerializer
{
public:
    static void serialize(const PythonNode& node, YAML::Node& doc)
    {
        doc["code"] = node.getCode();
    }

    static void deserialize(PythonNode& node, const YAML::Node& doc)
    {
        if(doc["code"].IsDefined()) {
            node.setCode(doc["code"].as<std::string>());
        }
    }
};
}


CSAPEX_REGISTER_SERIALIZER(csapex::PythonNode, PythonNodeSerializer)
