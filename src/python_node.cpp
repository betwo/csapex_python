/// HEADER
#include "python_node.h"

/// PROJECT
#include <csapex/utility/register_apex_plugin.h>

CSAPEX_REGISTER_CLASS(csapex::PythonNode, csapex::Node)

using namespace csapex;

PythonNode::PythonNode()
    : init_(false)
{
    code_ = "import apex\n"
            "test = 0\n"
            "input = apex.Input()\n"
            "def setup(NodeModifier& node_modifier):\n"
            "\tprint \"setup\"\n"
            "def tick():\n"
            "\tglobal test\n"
            "\tprint \"tick \", test\n"
            "\ttest += 1\n"
            "def process():\n"
            "\tprint \"process\"\n";
}

PythonNode::~PythonNode()
{
    PyEval_AcquireThread(thread_state);

    Py_EndInterpreter(thread_state);

    PyEval_ReleaseLock();
}

std::string PythonNode::getCode() const
{
    return code_;
}

void PythonNode::setCode(const std::string &code)
{
    code_ = code;

    try {
        PyEval_AcquireThread(thread_state);

        exec(code_.c_str(), globals, locals);

        exec("setup(NodeModifier& node_modifier)", globals, locals);

        PyEval_ReleaseThread(thread_state);

        init_ = true;
    } catch( error_already_set ) {
        PyErr_Print();
    }
}

void PythonNode::setup(NodeModifier& node_modifier)
{
    static bool init = false;
    if(!init) {
        init = true;
        Py_Initialize();
        PyEval_InitThreads();
    }

    thread_state = nullptr;
    PyThreadState *current_state = PyThreadState_Swap(nullptr);
    if (current_state == nullptr) {
        PyEval_AcquireLock();
    }

    thread_state = Py_NewInterpreter();

    object main = import("__main__");
    globals = main.attr("__dict__");

    PyEval_ReleaseThread(thread_state);

}

bool PythonNode::canTick()
{
    return init_;
}

void PythonNode::tick()
{
    try {
        PyEval_AcquireThread(thread_state);

        exec("tick()", globals, locals);

        PyEval_ReleaseThread(thread_state);

        std::cout << std::flush;
        std::cerr << std::flush;
        std::clog << std::flush;
    } catch( error_already_set ) {
        PyErr_Print();
    }
}

void PythonNode::process()
{

}
