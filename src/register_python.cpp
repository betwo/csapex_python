
/// HEADER
#include <csapex/core/core_plugin.h>

/// PROJECT
#include <csapex/core/csapex_core.h>
#include <csapex/factory/node_factory.h>
#include <csapex/utility/register_apex_plugin.h>
#include <csapex/model/node_constructor.h>
#include "python_wrapper.h"

/// SYSTEM
#include <fstream>
#if WIN32
#define TIXML_USE_STL
#include <tinyxml/tinyxml.h>
#else
#include <tinyxml.h>
#endif
#include <boost/filesystem.hpp>
#include <boost/version.hpp>
#if (BOOST_VERSION / 100000) >= 1 && (BOOST_VERSION / 100 % 1000) >= 54
namespace bfs = boost::filesystem;
#else
namespace bfs = boost::filesystem3;
#endif

namespace csapex {

class CSAPEX_EXPORT_PLUGIN RegisterPython: public CorePlugin
{
public:
    RegisterPython()
    {

    }

    void init(CsApexCore& core)
    {
        CsApexCore* core_ptr = &core;

        core.getNodeFactory()->manifest_loaded.connect([this, core_ptr](const std::string& manifest_file, const TiXmlElement* root) {

            const TiXmlElement* library = root;
            if (library->ValueStr() != "library") {
                library = library->NextSiblingElement("library");
            }
            while (library != nullptr) {
                std::string scripts_path_str = library->Attribute("path");
                if (scripts_path_str.size() == 0) {
                    std::cerr << "[Plugin] Item in row" << library->Row() << " does not contain a path attribute" << std::endl;
                    continue;
                }

                bfs::path scripts_path(manifest_file);
                scripts_path = scripts_path.parent_path() / scripts_path_str;

                const TiXmlElement* python_element = library->FirstChildElement("python");
                while (python_element) {
                    std::string base_class_type = python_element->Attribute("base_class_type");
                    if(base_class_type == "csapex::Node") {
                        std::string file_name = python_element->Attribute("file");

                        bfs::path file = scripts_path / file_name;
                        if(!bfs::exists(file)) {
                            std::cerr << "the python script " << file_name << " does not exist" << std::endl;
                            continue;
                        }

                        std::string description = readString(python_element, "description");
                        std::string icon = readString(python_element, "icon");
                        std::string tags = readString(python_element, "tags") + ", Python";

                        NodeConstructor::Ptr constructor = std::make_shared<NodeConstructor>(file_name, [file](){
                            std::shared_ptr<PythonWrapper> res = std::make_shared<PythonWrapper>();

                            std::ifstream in(file.string().c_str());
                            std::stringstream sstr;
                            sstr << in.rdbuf();
                            res->setCode(sstr.str());
                            return res;
                        });
                        constructor->setDescription(description);
                        constructor->setIcon(icon);
                        constructor->setTags(tags);

                        core_ptr->getNodeFactory()->registerNodeType(constructor);

                    }

                    python_element = python_element->NextSiblingElement( "python" );
                }


                library = library->NextSiblingElement( "library" );
            }
        });
    }

    std::string readString(const TiXmlElement* class_element, const std::string& name) {
        const TiXmlElement* description = class_element->FirstChildElement(name);
        std::string description_str;
        if(description) {
            description_str = description->GetText() ? description->GetText() : "";
        }

        return description_str;
    }


    void shutdown()
    {

    }
};

}


CSAPEX_REGISTER_CLASS(csapex::RegisterPython, csapex::CorePlugin)
