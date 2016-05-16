/// HEADER
#include <csapex_python/python_apex_api.hpp>

/// PROJECT
#include <csapex_vision/cv_mat_message.h>
#include <csapex/model/node_modifier.h>
#include <csapex/msg/input.h>
#include <csapex/msg/output.h>
#include <csapex/msg/message.h>
#include <csapex/msg/io.h>
#include <csapex/msg/generic_value_message.hpp>

/// SYSTEM
#include <boost/python.hpp>

using namespace csapex;
using namespace boost::python;

BOOST_PYTHON_MODULE(libcsapex_python)
{

    class_<csapex::Input, boost::noncopyable>("Input", no_init)
    ;
    class_<csapex::Output, boost::noncopyable>("Output", no_init)
    ;
    class_<csapex::NodeModifier, boost::noncopyable>("NodeModifier", no_init)
    ;

    def("getMessage",
        static_cast<TokenDataConstPtr(*)(Input*)>(&msg::getMessage),
        args("input"));

    def("getInt", &msg::getValue<int>, args("input"));
    def("getDouble", &msg::getValue<double>, args("input"));
    def("getString", &msg::getValue<std::string>, args("input"));

    def("publish",
        static_cast<void(*)(Output*,TokenDataConstPtr)>(&msg::publish),
        args("output", "message"));

    def("publishInt", msg::publish<int>, ( arg("output"), arg("message"), arg("frame")="/") );
    def("publishDouble",
        static_cast<void(*)(Output*,double,std::string)>(msg::publish<double>),
        ( arg("output"), arg("message"), arg("frame")="/") );
    def("publishString", msg::publish<std::string>, ( arg("output"), arg("message"), arg("frame")="/") );

    class_<csapex::Encoding>("Encoding")
            ;

    class_<csapex::connection_types::CvMatMessage>("CvMatMessage", init<Encoding, u_int64_t>() )
            ;


    register_ptr_to_python< std::shared_ptr<csapex::Input> >();
    register_ptr_to_python< std::shared_ptr<csapex::Output> >();
    register_ptr_to_python< std::shared_ptr<csapex::connection_types::Message> >();
    register_ptr_to_python< std::shared_ptr<csapex::connection_types::Message const> >();
}
