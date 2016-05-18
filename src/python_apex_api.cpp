/// HEADER
#include <csapex_python/python_apex_api.hpp>

/// PROJECT
#include <csapex_vision/cv_mat_message.h>
#include <csapex/model/node_modifier.h>
#include <csapex/msg/input.h>
#include <csapex/msg/output.h>
#include <csapex/signal/event.h>
#include <csapex/signal/slot.h>
#include <csapex/msg/message.h>
#include <csapex/msg/io.h>
#include <csapex/msg/generic_value_message.hpp>
#include <csapex_vision/yaml_io.hpp>

#include <numpy-opencv/np_opencv_converter.hpp>

/// SYSTEM
#include <boost/python.hpp>

using namespace csapex;
using namespace boost::python;

namespace  {


/*
 * CORE
 */

void registerCore()
{
    class_<Input, boost::noncopyable>("Input", no_init)
    ;
    class_<Output, boost::noncopyable>("Output", no_init)
    ;
    class_<Event, boost::noncopyable>("Event", no_init)
            .def("trigger", &Event::trigger)
    ;
    class_<Slot, boost::noncopyable>("Slot", no_init)
    ;
    class_<NodeModifier, boost::noncopyable>("NodeModifier", no_init)
    ;

    class_<TokenData, boost::noncopyable>("TokenData", no_init)
    ;

    def("getMessage", static_cast<TokenDataConstPtr(*)(Input*)>(&msg::getMessage), args("input"));
    def("publish", static_cast<void(*)(Output*,TokenDataConstPtr)>(&msg::publish), args("output", "message"));

    register_ptr_to_python< std::shared_ptr<Input> >();
    register_ptr_to_python< std::shared_ptr<Output> >();
    register_ptr_to_python< std::shared_ptr<Event> >();
    register_ptr_to_python< std::shared_ptr<Slot> >();
    register_ptr_to_python< std::shared_ptr<TokenData> >();
    register_ptr_to_python< std::shared_ptr<TokenData const> >();
}


/*
 * VALUE
 */

template <typename Instance, typename Payload>
Payload getTemplateValue(const Instance& self)
{
    return self.value;
}


template <typename Payload>
void register_generic_value_message(const std::string& name)
{
    class_<connection_types::GenericValueMessage<Payload>, bases<TokenData>>(name.c_str())
            .add_property("value", &getTemplateValue<connection_types::GenericValueMessage<Payload>, Payload>)
            ;
    implicitly_convertible<std::shared_ptr<connection_types::GenericValueMessage<Payload>>, std::shared_ptr<TokenData> >();
    implicitly_convertible<std::shared_ptr<connection_types::GenericValueMessage<Payload> const>, std::shared_ptr<TokenData const> >();

    def("publish", msg::publish<Payload>, ( arg("output"), arg("message"), arg("frame")="/") );
}

void registerGenericValueMessages()
{
    register_generic_value_message< int >( "int" );
    register_generic_value_message< double >( "double" );
    register_generic_value_message< std::string >( "string" );
}

/*
 * VISION
 */

connection_types::CvMatMessage::ConstPtr getCvMatMessage(Input* input)
{
    return msg::getMessage<connection_types::CvMatMessage>(input);
}
Encoding getEncoding(const connection_types::CvMatMessage& cvmat)
{
    return cvmat.getEncoding();
}
cv::Mat getCvMat(const connection_types::CvMatMessage& cvmat)
{
    return cvmat.value;
}

void publishCvMat(Output* output, const cv::Mat& cvmat, Encoding enc)
{
    connection_types::CvMatMessage::Ptr msg = connection_types::makeEmpty<connection_types::CvMatMessage>();
    msg->value = cvmat;
    msg->setEncoding(enc);
    msg::publish(output, msg);
}
void registerCsApexVision()
{
    class_<Encoding>("Encoding")
            ;

    class_<connection_types::CvMatMessage, bases<TokenData>>("CvMatMessage", init<Encoding, u_int64_t>() )
            .add_property("encoding", &getEncoding)
            .add_property("value", &getTemplateValue<connection_types::CvMatMessage, cv::Mat>)
            ;

    def("publish", &publishCvMat, (args("output"), args("img"), args("encoding")));

    register_ptr_to_python< std::shared_ptr<connection_types::CvMatMessage> >();
    register_ptr_to_python< std::shared_ptr<connection_types::CvMatMessage const> >();
    implicitly_convertible<std::shared_ptr<connection_types::CvMatMessage>, std::shared_ptr<TokenData> >();
    implicitly_convertible<std::shared_ptr<connection_types::CvMatMessage const>, std::shared_ptr<TokenData const> >();

    fs::python::init_and_export_converters();

    std::string nested_name = py::extract<std::string>(py::scope().attr("__name__") + ".enc");
    py::object nested_module(py::handle<>(py::borrowed(PyImport_AddModule(nested_name.c_str()))));
    py::scope().attr("enc") = nested_module;
    py::scope parent = nested_module;

    parent.attr("mono") = enc::mono;
    parent.attr("bgr") = enc::bgr;
    parent.attr("unknown") = enc::unknown;
    parent.attr("rgb") = enc::rgb;
    parent.attr("hsv") = enc::hsv;
    parent.attr("hsl") = enc::hsl;
    parent.attr("yuv") = enc::yuv;
    parent.attr("depth") = enc::depth;
    parent.attr("lab") = enc::lab;
}
}


/*
 * MODULE
 */

BOOST_PYTHON_MODULE(libcsapex_python)
{
    registerCore();

    registerGenericValueMessages();

    registerCsApexVision();
}
