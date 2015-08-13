/// HEADER
#include <csapex_python/python_apex_api.hpp>

/// PROJECT
#include <csapex_vision/cv_mat_message.h>

/// SYSTEM
#include <boost/python.hpp>

using namespace csapex;
using namespace boost::python;

BOOST_PYTHON_MODULE(apex)
{
    class_<csapex::Encoding>("Encoding")
            ;

    class_<csapex::connection_types::CvMatMessage>("CvMatMessage", init<Encoding, u_int64_t>() )
            ;

}
