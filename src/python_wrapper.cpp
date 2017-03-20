/// HEADER
#include "python_wrapper.h"

/// PROJECT
#include <csapex/utility/register_apex_plugin.h>
#include <csapex/model/node_handle.h>
#include <csapex/msg/any_message.h>
#include <csapex/serialization/node_serializer.h>
#include <csapex/msg/no_message.h>
#include <csapex/msg/end_of_sequence_message.h>
#include <csapex/msg/end_of_program_message.h>
#include <csapex/msg/input.h>
#include <csapex/msg/output.h>
#include <csapex/signal/event.h>
#include <csapex/signal/slot.h>

/// SYSTEM
#include <yaml-cpp/yaml.h>

using namespace csapex;
namespace bp = boost::python;

namespace
{
std::string parse_python_exception(){
    PyObject *type_ptr = NULL, *value_ptr = NULL, *traceback_ptr = NULL;
    PyErr_Fetch(&type_ptr, &value_ptr, &traceback_ptr);
    std::string ret("Unfetchable Python error");

    if(type_ptr != NULL){
        bp::handle<> h_type(type_ptr);
        bp::str type_pstr(h_type);
        bp::extract<std::string> e_type_pstr(type_pstr);
        if(e_type_pstr.check())
            ret = e_type_pstr();
        else
            ret = "Unknown exception type";
    }

    if(value_ptr != NULL){
        bp::handle<> h_val(value_ptr);
        bp::str a(h_val);
        bp::extract<std::string> returned(a);
        if(returned.check())
            ret +=  ": " + returned();
        else
            ret += std::string(": Unparseable Python error: ");
    }

    if(traceback_ptr != NULL){
        bp::handle<> h_tb(traceback_ptr);
        bp::object tb(bp::import("traceback"));
        bp::object fmt_tb(tb.attr("format_tb"));
        bp::object tb_list(fmt_tb(h_tb));
        bp::object tb_str(bp::str("\n").join(tb_list));
        bp::extract<std::string> returned(tb_str);
        if(returned.check())
            ret += ": " + returned();
        else
            ret += std::string(": Unparseable Python traceback");
    }
    return ret;
}
}

PythonWrapper::PythonWrapper()
    : is_setup_(false), python_is_initialized_(false)
{
}

PythonWrapper::~PythonWrapper()
{
    PyEval_AcquireThread(thread_state);

    Py_EndInterpreter(thread_state);

    PyEval_ReleaseLock();
}


std::string PythonWrapper::getCode() const
{
    return code_;
}

void PythonWrapper::setCode(const std::string &code)
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

        try {
            globals["csapex"] = bp::import("csapex");
        }catch(boost::python::error_already_set const &){
            std::string perror_str = parse_python_exception();
            std::cout << "Error in Python: " << perror_str << std::endl;
        }

        python_is_initialized_ = true;

    } else {
        PyEval_AcquireThread(thread_state);
    }

    PyEval_ReleaseThread(thread_state);
}

void PythonWrapper::setupIO()
{
    if(!is_setup_) {
        PyEval_AcquireThread(thread_state);

        if(node_handle_) {
            try {
                bp::list inputs;
                for(InputPtr i : node_modifier_->getMessageInputs()) {
                    if(!node_handle_->isParameterInput(i.get())) {
                        inputs.append(bp::pointer_wrapper<Input*>(i.get()));
                    }
                }
                globals["inputs"] = inputs;

                bp::list outputs;
                for(OutputPtr o : node_modifier_->getMessageOutputs()) {
                    if(!node_handle_->isParameterOutput(o.get())) {
                        outputs.append(bp::pointer_wrapper<Output*>(o.get()));
                    }
                }
                globals["outputs"] = outputs;

                bp::list slot;
                for(SlotPtr i : node_modifier_->getSlots()) {
                    slot.append(bp::pointer_wrapper<Slot*>(i.get()));
                }
                globals["slots"] = slot;

                bp::list events;
                for(EventPtr o : node_modifier_->getEvents()) {
                    events.append(bp::pointer_wrapper<Event*>(o.get()));
                }
                globals["events"] = events;

                bp::exec(code_.c_str(), globals, globals);

                flush();

                is_setup_ = true;

            } catch( bp::error_already_set ) {
                PyErr_Print();
            }
        }

        PyEval_ReleaseThread(thread_state);
    }
}

void PythonWrapper::setup(NodeModifier& node_modifier)
{
    setupIO();

    if(exists("setup")) {
        call("setup", &node_modifier);

        is_setup_ = false;
    }
}

void PythonWrapper::setupParameters(Parameterizable &parameters)
{
}

bool PythonWrapper::canProcess() const
{
    return is_setup_;
}

void PythonWrapper::flush()
{
    bp::exec("import sys\n"
             "sys.stdout.flush()\n", globals, globals);
}

void PythonWrapper::call(const std::string& method, NodeModifier* modifier)
{
    PyEval_AcquireThread(thread_state);

    try {
        if(modifier) {
            globals[method](bp::pointer_wrapper<NodeModifier*>(modifier));
        } else {
            globals[method]();
        }

        flush();

        std::cout << std::flush;
        std::cerr << std::flush;
        std::clog << std::flush;

    } catch( bp::error_already_set ) {
        PyErr_Print();
    }

    PyEval_ReleaseThread(thread_state);
}

bool PythonWrapper::exists(const std::string &method)
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

void PythonWrapper::process()
{
    setupIO();

    if(exists("process")) {
        call("process", nullptr);
    }
}


void PythonWrapper::processMarker(const connection_types::MessageConstPtr &marker)
{
    if(std::dynamic_pointer_cast<connection_types::NoMessage const>(marker)) {
        if(exists("processNoMessage")) {
            call("processNoMessage", nullptr);
        }

    } else if(std::dynamic_pointer_cast<connection_types::EndOfProgramMessage const>(marker)) {
        if(exists("processEndOfProgram")) {
            call("processEndOfProgram", nullptr);
        }

    } else if(std::dynamic_pointer_cast<connection_types::EndOfSequenceMessage const>(marker)) {
        if(exists("processEndOfSequence")) {
            call("processEndOfSequence", nullptr);
        }
    }
}

