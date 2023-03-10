#include "../civan/config.h"
#include "../civan/log.h"
#include <yaml-cpp/yaml.h>
#include <iostream>
// civan::ConfigVar<int>::ptr g_int_value_config = 
//     civan::Config::Lookup("system.port", (int)8080, "system port");
// // civan::ConfigVar<float>::ptr g_int_valuex_config =
// //     civan::Config::Lookup("system.port", (float)8080, "system port");
// civan::ConfigVar<std::vector<int>>::ptr g_vec_value_config = 
//     civan::Config::Lookup("system.int_vec", std::vector<int>{5,5}, "system int_vec");

// civan::ConfigVar<civan::Person>::ptr g_person_value_config = 
//     civan::Config::Lookup("system.person", civan::Person(), "system int_vec");

// civan::ConfigVar<std::map<std::string, int>>::ptr g_map_value_config = 
//     civan::Config::Lookup("system.str_int_map", std::map<std::string, int>{{"a", 4}, {"b", 3}}, "system int_vec");

// civan::ConfigVar<std::map<std::string, civan::Person> >::ptr g_person_map_value_config = 
//     civan::Config::Lookup("system.person_map" 
//                             , std::map<std::string, civan::Person>{ {"aa", civan::Person()}, {"cc", civan::Person()}}
//                             , "system int_vec");

void print_yaml(const YAML::Node& node, int level) {
    if(node.IsScalar()) {
        CIVAN_LOG_INFO(CIVAN_LOG_ROOT()) << std::string(level * 4, ' ')
            << node.Scalar() << " - " << node.Type() << " - " << level;
    } else if(node.IsNull()) {
        CIVAN_LOG_INFO(CIVAN_LOG_ROOT()) << std::string(level * 4, ' ')
            << "NULL - " << node.Type() << " - " << level;
    } else if(node.IsMap()) {
        for(auto it = node.begin();
                it != node.end(); ++it) {
            CIVAN_LOG_INFO(CIVAN_LOG_ROOT()) << std::string(level * 4, ' ')
                    << it->first << " - " << it->second.Type() << " - " << level;
            print_yaml(it->second, level + 1);
        }
    } else if(node.IsSequence()) {
        for(size_t i = 0; i < node.size(); ++i) {
            CIVAN_LOG_INFO(CIVAN_LOG_ROOT()) << std::string(level * 4, ' ')
                << i << " - " << node[i].Type() << " - " << level;
            print_yaml(node[i], level + 1);
        }
    }
}

void test_yaml() {
    std::cout<< civan::LoggerMgr::GetInstance()->toYamlString() << std::endl;
    YAML::Node root = YAML::LoadFile("/home/dell/jqchen/cpp_project/civan/bin/conf/log.yml");
    print_yaml(root, 0);
    
}


void test_log() {
    CIVAN_LOG_DEBUG(CIVAN_LOG_NAME("root")) << "ffff";
    std::cout<< civan::LoggerMgr::GetInstance()->toYamlString() << std::endl;
    YAML::Node root = YAML::LoadFile("/home/dell/jqchen/cpp_project/civan/bin/conf/log.yml");
    civan::Config::LoadFromYaml(root);
    //CIVAN_LOG_INFO(CIVAN_LOG_ROOT()) << root;
    std::cout << std::endl;
    std::cout << std::endl;
    std::cout << std::endl;
    std::cout << std::endl;

    std::cout<< civan::LoggerMgr::GetInstance()->toYamlString() <<  std::endl;

    auto logger = CIVAN_LOG_NAME("system");
    std::cout << logger->toYamlString() << std::endl;
    CIVAN_LOG_FATAL(logger) << "ffff";
}
int main(int argc, char** argv) {

    // CIVAN_LOG_INFO(CIVAN_LOG_ROOT()) << g_map_value_config->getValue();
    //CIVAN_LOG_INFO(CIVAN_LOG_ROOT()) << g_map_value_config->toString();
    //CIVAN_LOG_INFO(CIVAN_LOG_ROOT()) << g_person_map_value_config->toString();
    // auto v = g_map_value_config->getValue();
    // for (auto i : v) {
    //     CIVAN_LOG_INFO(CIVAN_LOG_ROOT()) << i.first << " " << i.second;
    // }
    //test_yaml();
    // YAML::Node root = YAML::LoadFile("/home/dell/jqchen/cpp_project/civan/bin/conf/log.yml");
    // civan::Config::LoadFromYaml(root);
    //CIVAN_LOG_INFO(CIVAN_LOG_ROOT()) << g_int_value_config->getValue();
    // v = g_map_value_config->getValue();
    // for (auto i : v) {
    //     CIVAN_LOG_INFO(CIVAN_LOG_ROOT()) << i.first << " " << i.second;
    // }

    // CIVAN_LOG_INFO(CIVAN_LOG_ROOT()) << g_person_map_value_config->toString();
    test_log();
    // std::cout << "visit" << std::endl;
    // civan::Config::Visit([](civan::ConfigVarBase::ptr var) {
    //     CIVAN_LOG_INFO(CIVAN_LOG_ROOT()) << "name=" << var->getName()
    //                 << " description=" << var->getDescription()
    //                 << " typename=" << var->getTypeName()
    //                 << " value=" << var->toString();
    // });
    // std::cout << "visit" << std::endl;


    return 0;
}