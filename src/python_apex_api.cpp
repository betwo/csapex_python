/// HEADER
#include <csapex_python/python_apex_api.hpp>

/// PROJECT
#include <csapex_opencv/cv_mat_message.h>
#include <csapex/model/node_modifier.h>
#include <csapex/msg/input.h>
#include <csapex/msg/output.h>
#include <csapex/signal/event.h>
#include <csapex/signal/slot.h>
#include <csapex/msg/message.h>
#include <csapex/msg/io.h>
#include <csapex/msg/generic_value_message.hpp>
#include <csapex_opencv/yaml_io.hpp>
#include <csapex_point_cloud/msg/point_cloud_message.h>
#include <csapex/msg/any_message.h>

#define NPY_NO_DEPRECATED_API NPY_1_7_API_VERSION
#include <numpy-opencv/np_opencv_converter.hpp>

/// SYSTEM
#include <boost/python.hpp>
#include <pcl/PCLPointField.h>
#include <pcl/console/print.h>
#include <pcl/PCLPointCloud2.h>
#include <pcl/conversions.h>
#include <boost/mpl/for_each.hpp>
#include <boost/python/suite/indexing/vector_indexing_suite.hpp>

namespace pcl {
inline bool operator == (const pcl::PointXYZI& lhs, const pcl::PointXYZI& rhs) noexcept {
    return lhs.x == rhs.x && lhs.y == rhs.y && lhs.z == rhs.z && lhs.intensity == rhs.intensity;
}
inline bool operator == (const pcl::PointXYZRGB& lhs, const pcl::PointXYZRGB& rhs) noexcept {
    return lhs.x == rhs.x && lhs.y == rhs.y && lhs.z == rhs.z && lhs.r == rhs.r && lhs.g == rhs.g && lhs.b == rhs.b;
}
inline bool operator == (const pcl::PointXYZRGBL& lhs, const pcl::PointXYZRGBL& rhs) noexcept {
    return lhs.x == rhs.x && lhs.y == rhs.y && lhs.z == rhs.z && lhs.r == rhs.r && lhs.g == rhs.g && lhs.b == rhs.b && lhs.label == rhs.label;
}
inline bool operator == (const pcl::PointXYZRGBA& lhs, const pcl::PointXYZRGBA& rhs) noexcept {
    return lhs.x == rhs.x && lhs.y == rhs.y && lhs.z == rhs.z && lhs.r == rhs.r && lhs.g == rhs.g && lhs.b == rhs.b && lhs.a == rhs.a;
}
inline bool operator == (const pcl::PointXYZL& lhs, const pcl::PointXYZL& rhs) noexcept {
    return lhs.x == rhs.x && lhs.y == rhs.y && lhs.z == rhs.z && lhs.label == rhs.label;
}
inline bool operator == (const pcl::PointXYZ& lhs, const pcl::PointXYZ& rhs) noexcept {
    return lhs.x == rhs.x && lhs.y == rhs.y && lhs.z == rhs.z;
}
inline bool operator == (const pcl::PointNormal& lhs, const pcl::PointNormal& rhs) noexcept {
    return lhs.x == rhs.x && lhs.y == rhs.y && lhs.z == rhs.z&& lhs.normal_x == rhs.normal_x && lhs.normal_y == rhs.normal_y && lhs.normal_z == rhs.normal_z;
}
}

using namespace csapex;
using namespace boost::python;



namespace  {


/*
 * CORE
 */

Input* addInput(NodeModifier* modifier, const std::string& label, bool optional)
{
    return modifier->addInput(connection_types::makeEmpty<connection_types::AnyMessage>(), label, optional);
}
Output* addOutput(NodeModifier* modifier, const std::string& label)
{
    return modifier->addOutput(connection_types::makeEmpty<connection_types::AnyMessage>(), label);
}

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

    def("addInput", &addInput, args("label", "optional"), return_value_policy<reference_existing_object>());
    def("addOutput", &addOutput, args("label"), return_value_policy<reference_existing_object>());

    def("getMessage", static_cast<TokenDataConstPtr(*)(Input*)>(&msg::getMessage), args("input"));
    def("publish", static_cast<void(*)(Output*,TokenDataConstPtr)>(&msg::publish), args("output", "message"));

    register_ptr_to_python< std::shared_ptr<Input> >();
    register_ptr_to_python< std::shared_ptr<Output> >();
    register_ptr_to_python< std::shared_ptr<Event> >();
    register_ptr_to_python< std::shared_ptr<Slot> >();
    register_ptr_to_python< std::shared_ptr<TokenData> >();
    register_ptr_to_python< std::shared_ptr<TokenData const> >();

    apex_assert(opencv_error == 0);
}

template <typename M>
void register_message()
{
    register_ptr_to_python< std::shared_ptr<M> >();
    register_ptr_to_python< std::shared_ptr<M const> >();
    implicitly_convertible<std::shared_ptr<M>, std::shared_ptr<TokenData> >();
    implicitly_convertible<std::shared_ptr<M const>, std::shared_ptr<TokenData const> >();
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

    register_message< connection_types::CvMatMessage >();

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


/*
 * PCL
 */

typedef
boost::variant<
pcl::PointCloud<pcl::PointXYZI>::Ptr,
pcl::PointCloud<pcl::PointXYZRGB>::Ptr,
pcl::PointCloud<pcl::PointXYZRGBL>::Ptr,
pcl::PointCloud<pcl::PointXYZRGBA>::Ptr,
pcl::PointCloud<pcl::PointXYZL>::Ptr,
pcl::PointCloud<pcl::PointXYZ>::Ptr,
pcl::PointCloud<pcl::PointNormal>::Ptr>
pc_variant;

struct RegisterPointType  {
    RegisterPointType()
    {}

    template <typename PointT>
    void operator () (PointT) const
    {
        std::string cloud_label = std::string("PointCloudMessage <") + connection_types::traits::name<PointT>() + ">";
        class_<pcl::PointCloud<PointT>>(cloud_label.c_str(), init<>() )
                .def_readwrite("points", &pcl::PointCloud<PointT>::points)
                ;


        std::string vector_label = std::string("vector <") + connection_types::traits::name<PointT>() + ">";
        class_< std::vector<PointT, Eigen::aligned_allocator<PointT>> >(vector_label.c_str())
                .def(vector_indexing_suite<std::vector<PointT, Eigen::aligned_allocator<PointT>>, true>())
                ;

        register_ptr_to_python< boost::shared_ptr<pcl::PointCloud<PointT>> >();
        register_ptr_to_python< boost::shared_ptr<pcl::PointCloud<PointT> const> >();
        implicitly_convertible< boost::shared_ptr<pcl::PointCloud<PointT>>, connection_types::PointCloudMessage::variant >();
    }

};

struct variant_to_object : boost::static_visitor<PyObject *> {
    static result_type convert(pc_variant const &v) {
        return boost::apply_visitor(variant_to_object(), v);
    }

    template<typename T>
    result_type operator()(T const &t) const {
        return boost::python::incref(boost::python::object(t).ptr());
    }
};

pc_variant getVariant(const connection_types::PointCloudMessage& cloud)
{
    return cloud.value;
}

void registerPointCloud()
{
    to_python_converter<pc_variant, variant_to_object>();

    class_<connection_types::PointCloudMessage, bases<TokenData>>("PointCloudMessage", init<std::string, u_int64_t>() )
            .add_property("value", &getVariant)
            ;

    register_message< connection_types::PointCloudMessage >();

    boost::mpl::for_each<connection_types::PointCloudPointTypes>( RegisterPointType() );


    class_<pcl::PointXYZI>(connection_types::traits::name<pcl::PointXYZI>().c_str())
            .def_readwrite("x", &pcl::PointXYZI::x)
            .def_readwrite("y", &pcl::PointXYZI::y)
            .def_readwrite("z", &pcl::PointXYZI::z)
            .def_readwrite("intensity", &pcl::PointXYZI::intensity)
            ;
    class_<pcl::PointXYZRGB>(connection_types::traits::name<pcl::PointXYZRGB>().c_str())
            .def_readwrite("x", &pcl::PointXYZRGB::x)
            .def_readwrite("y", &pcl::PointXYZRGB::y)
            .def_readwrite("z", &pcl::PointXYZRGB::z)
            .def_readwrite("r", &pcl::PointXYZRGB::r)
            .def_readwrite("g", &pcl::PointXYZRGB::g)
            .def_readwrite("b", &pcl::PointXYZRGB::b)
            ;
    class_<pcl::PointXYZRGBL>(connection_types::traits::name<pcl::PointXYZRGBL>().c_str())
            .def_readwrite("x", &pcl::PointXYZRGBL::x)
            .def_readwrite("y", &pcl::PointXYZRGBL::y)
            .def_readwrite("z", &pcl::PointXYZRGBL::z)
            .def_readwrite("r", &pcl::PointXYZRGBL::r)
            .def_readwrite("g", &pcl::PointXYZRGBL::g)
            .def_readwrite("b", &pcl::PointXYZRGBL::b)
            .def_readwrite("label", &pcl::PointXYZRGBL::label)
            ;
    class_<pcl::PointXYZRGBA>(connection_types::traits::name<pcl::PointXYZRGBA>().c_str())
            .def_readwrite("x", &pcl::PointXYZRGBA::x)
            .def_readwrite("y", &pcl::PointXYZRGBA::y)
            .def_readwrite("z", &pcl::PointXYZRGBA::z)
            .def_readwrite("r", &pcl::PointXYZRGBA::r)
            .def_readwrite("g", &pcl::PointXYZRGBA::g)
            .def_readwrite("b", &pcl::PointXYZRGBA::b)
            .def_readwrite("a", &pcl::PointXYZRGBA::a)
            ;
    class_<pcl::PointXYZL>(connection_types::traits::name<pcl::PointXYZL>().c_str())
            .def_readwrite("x", &pcl::PointXYZL::x)
            .def_readwrite("y", &pcl::PointXYZL::y)
            .def_readwrite("z", &pcl::PointXYZL::z)
            .def_readwrite("label", &pcl::PointXYZL::label)
            ;
    class_<pcl::PointXYZ>(connection_types::traits::name<pcl::PointXYZ>().c_str())
            .def_readwrite("x", &pcl::PointXYZ::x)
            .def_readwrite("y", &pcl::PointXYZ::y)
            .def_readwrite("z", &pcl::PointXYZ::z)
            ;
    class_<pcl::PointNormal>(connection_types::traits::name<pcl::PointNormal>().c_str())
            .def_readwrite("x", &pcl::PointNormal::x)
            .def_readwrite("y", &pcl::PointNormal::y)
            .def_readwrite("z", &pcl::PointNormal::z)
            .def_readwrite("normal_x", &pcl::PointNormal::normal_x)
            .def_readwrite("normal_y", &pcl::PointNormal::normal_y)
            .def_readwrite("normal_z", &pcl::PointNormal::normal_z)
            ;
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

    registerPointCloud();
}
