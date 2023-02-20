#ifndef __CIVAN_CONFIG_H__
#define __CIVAN_CONFIG_H__

#include <memory>
#include <string>
#include <sstream>
#include <boost/lexical_cast.hpp>
#include "log.h"
#include <map>
#include <typeinfo>
#include <yaml-cpp/yaml.h>
#include <ctype.h>
#include <vector>
#include <map>
#include <set>
#include <unordered_map>
#include <unordered_set>
#include <list>
#include <functional>
#include "thread.h"
#include "mutex.h"
namespace civan {

class ConfigVarBase {
public:
    typedef std::shared_ptr<ConfigVarBase> ptr;
    typedef RWMutex RWMutexType;
    ConfigVarBase(const std::string& name, const std::string& description = "")
                :m_name(name), m_description(description) {
        std::transform(m_name.begin(), m_name.end(), m_name.begin(), ::tolower);
    }
    virtual ~ConfigVarBase() {}

    const std::string& getName() const { return m_name; }
    const std::string& getDescription() const { return m_description; }

    virtual std::string toString() = 0;
    virtual bool fromString(const std::string& val) = 0;
    virtual std::string getTypeName() const = 0;
protected:
    std::string m_name;
    std::string m_description;
    mutable RWMutexType m_mutex;
};

template<typename F, typename T>
class LexicalCast {
public:
    T operator() (const F& v) {
        return boost::lexical_cast<T>(v);
    }
};


//vector
template<typename T>
class LexicalCast<std::string, std::vector<T> > {
public:
    std::vector<T> operator() (const std::string& v) {
        YAML::Node node = YAML::Load(v);
        std::stringstream ss;
        std::vector<T> vec;
        for (size_t i = 0; i < node.size(); ++i) {
            ss.str("");
            ss << node[i];
            vec.push_back(LexicalCast<std::string, T>()(ss.str()));
        }
        return vec;
    }
};

template<typename T>
class LexicalCast<std::vector<T>, std::string> {
public:
    std::string operator()(const std::vector<T>& v) {
        YAML::Node node;
        for (auto&i :v) {
            node.push_back(LexicalCast<T, std::string>()(i));
        }
        std::stringstream ss;
        ss << node;
        return ss.str();
    }
};

//list
template<typename T>
class LexicalCast<std::string, std::list<T> > {
public:
    std::list<T> operator() (const std::string& v) {
        YAML::Node node = YAML::Load(v);
        std::stringstream ss;
        std::list<T> vec;
        for (size_t i = 0; i < node.size(); ++i) {
            ss.str("");
            ss << node[i];
            vec.push_back(LexicalCast<std::string, T>()(ss.str()));
        }
        return vec;
    }
};

template<typename T>
class LexicalCast<std::list<T>, std::string> {
public:
    std::string operator()(const std::list<T>& v) {
        YAML::Node node;
        for (auto&i :v) {
            node.push_back(LexicalCast<T, std::string>()(i));
        }
        std::stringstream ss;
        ss << node;
        return ss.str();
    }
};


//set
template<typename T>
class LexicalCast<std::string, std::set<T> > {
public:
    std::set<T> operator() (const std::string& v) {
        YAML::Node node = YAML::Load(v);
        std::stringstream ss;
        std::set<T> set_r;
        for (size_t i = 0; i < node.size(); ++i) {
            ss.str("");
            ss << node[i];
            set_r.insert(LexicalCast<std::string, T>()(ss.str()));
        }
        return set_r;
    }
};

template<typename T>
class LexicalCast<std::set<T>, std::string> {
public:
    std::string operator()(const std::set<T>& s) {
        YAML::Node node;
        for (auto&i :s) {
            node.push_back(LexicalCast<T, std::string>()(i));
        }
        std::stringstream ss;
        ss << node;
        return ss.str();
    }
};


//unordered_set
template<typename T>
class LexicalCast<std::string, std::unordered_set<T> > {
public:
    std::unordered_set<T> operator() (const std::string& v) {
        YAML::Node node = YAML::Load(v);
        std::stringstream ss;
        std::unordered_set<T> set_r;
        for (size_t i = 0; i < node.size(); ++i) {
            ss.str("");
            ss << node[i];
            set_r.insert(LexicalCast<std::string, T>()(ss.str()));
        }
        return set_r;
    }
};

template<typename T>
class LexicalCast<std::unordered_set<T>, std::string> {
public:
    std::string operator()(const std::unordered_set<T>& s) {
        YAML::Node node;
        for (auto&i :s) {
            node.push_back(LexicalCast<T, std::string>()(i));
        }
        std::stringstream ss;
        ss << node;
        return ss.str();
    }
};

//map
template<typename T>
class LexicalCast<std::string, std::map<std::string, T> > {
public:
    std::map<std::string, T> operator() (const std::string& v) {
        YAML::Node node = YAML::Load(v);
        std::stringstream ss;
        std::map<std::string, T> map_r;
        for (auto it = node.begin(); it != node.end(); ++it) {
            ss.str("");
            ss << it->second;
            map_r.insert(std::make_pair(it->first.Scalar(), 
                            LexicalCast<std::string, T>()(ss.str())));
        }
        return map_r;
    }
};

template<typename T>
class LexicalCast<std::map<std::string, T>, std::string> {
public:
    std::string operator()(const std::map<std::string, T>& m) {
        YAML::Node node;
        for (auto&i :m) {
            node[i.first] = YAML::Load(LexicalCast<T, std::string>()(i.second));
        }
        std::stringstream ss;
        ss << node;
        return ss.str();
    }
};


//unordered_map
template<typename T>
class LexicalCast<std::string, std::unordered_map<std::string, T> > {
public:
    std::unordered_map<std::string, T> operator() (const std::string& v) {
        YAML::Node node = YAML::Load(v);
        std::stringstream ss;
        std::unordered_map<std::string, T> map_r;
        for (auto it = node.begin(); it != node.end(); ++it) {
            ss.str("");
            ss << it->second;
            map_r.insert(std::make_pair(it->first.Scalar(), 
                            LexicalCast<std::string, T>()(ss.str())));
        }
        return map_r;
    }
};

template<typename T>
class LexicalCast<std::unordered_map<std::string, T>, std::string> {
public:
    std::string operator()(const std::unordered_map<std::string, T>& m) {
        YAML::Node node;
        for (auto&i :m) {
            node[i.first] = YAML::Load(LexicalCast<T, std::string>()(i.second));
        }
        std::stringstream ss;
        ss << node;
        return ss.str();
    }
};


class Person {
public:
    std::string m_name;
    int m_age = 1;
    bool operator==(const Person& p) const {
        return this->m_name == p.m_name && this->m_age == p.m_age;
    }
};

template<>
class LexicalCast<std::string, Person> {
public:
    Person operator() (const std::string& v) {
        YAML::Node node = YAML::Load(v);
        Person res;
        res.m_name = node["name"].as<std::string>();
        res.m_age = node["age"].as<int>();
        return res;
    }
};

template<>
class LexicalCast<Person, std::string> {
public:
    std::string operator()(const Person& v) {
        YAML::Node node;
        node["name"] = v.m_name;
        node["age"] = v.m_age;

        std::stringstream ss;
        ss << node;
        return ss.str();
    }
};



template<typename T, typename FromStr = LexicalCast<std::string, T>
                        , typename ToStr = LexicalCast<T, std::string>>
class ConfigVar : public ConfigVarBase {
public:
    typedef std::shared_ptr<ConfigVar> ptr;
    typedef std::function<void (const T& old_value, const T& new_value)> on_change_cb;
    ConfigVar(const std::string& name
            ,const T& default_value
            , const std::string& description)
        :ConfigVarBase(name, description)
        , m_val(default_value) {

    }

    std::string toString() override {
        try {
            RWMutexType::ReadLock l(m_mutex);
            //m_name = boost::lexical_cast<std::string>(m_val);
            return ToStr()(m_val);
        } catch(std::exception& e) {
            CIVAN_LOG_ERROR(CIVAN_LOG_ROOT()) << "ConfigVar::toString exception"
                << e.what() << " convert: " << typeid(m_val).name() << " to string";

        }
        return "";
    }

    bool fromString(const std::string& val) override {
        RWMutexType::WriteLock l(m_mutex);
        try {
            //m_val = boost::lexical_cast<T>(val);
            setValue(FromStr()(val));
            return true;
        } catch(std::exception& e) {
            CIVAN_LOG_ERROR(CIVAN_LOG_ROOT()) << "ConfigVar::toString exception"
                << e.what() << " convert: " << typeid(m_val).name() << " to string";
        }
        return false;
    }
    const T& getValue() const { 
        RWMutexType::ReadLock l(m_mutex);
        return m_val; 
    }
    std::string getTypeName() const override { return typeid(T).name(); }

    void setValue(const T& v) {
        {
            RWMutexType::ReadLock l(m_mutex);
            if (v == m_val) {
                return;
            }
            for (auto&i : m_cbs) {
                i.second(m_val, v);
            }
        }
        RWMutexType::WriteLock l(m_mutex);
        m_val = v;
    }

    uint64_t addListener(on_change_cb cb) {
        RWMutexType::WriteLock l(m_mutex);
        static uint64_t key = 0; 
        m_cbs[key] = cb;
        key++;
        return key;
    }

    void delListener(uint64_t key) {
        RWMutexType::WriteLock l(m_mutex);
        m_cbs.erease(key);
    }

    on_change_cb getListener(uint64_t key) {
        RWMutexType::ReadLock l(m_mutex);
        auto it = m_cbs.find(key);
        return it == m_cbs.end() ? nullptr : it->second;
    }

    void clearListener() {
        RWMutexType::WriteLock lock(m_mutex);
        m_cbs.clear();
    }

private:
    T m_val;
    //变更回调组
    std::map<uint64_t, on_change_cb> m_cbs;
};

class Config {
public:
    typedef std::map<std::string, ConfigVarBase::ptr> ConfigVarMap;
    typedef RWMutex RWMutexType;
    template<typename T>
    static typename ConfigVar<T>::ptr Lookup(const std::string& name, 
        const T& default_value, const std::string& description = "") {
        auto tmp = Lookup<T>(name);
        if (tmp) {
            return tmp;
        }
        typename ConfigVar<T>::ptr v(new ConfigVar<T>(name, default_value, description));
        RWMutexType::WriteLock l(GetMutex());
        GetDatas()[name] = v;
        return v;
    }

    template<typename T>
    static typename ConfigVar<T>::ptr Lookup(const std::string& name) {
        RWMutexType::ReadLock l(GetMutex());
        auto it = GetDatas().find(name);
        if (it != GetDatas().end()) {
            auto tmp = std::dynamic_pointer_cast<ConfigVar<T> >(it->second);
            if (tmp) {
                CIVAN_LOG_INFO(CIVAN_LOG_ROOT()) << "Lookup name=" << name << "exists";
                return tmp;
            } else {
                CIVAN_LOG_ERROR(CIVAN_LOG_ROOT()) << "Lookup name=" << name << "exists but type not " 
                                                << typeid(T).name() << " real type = " << it->second->getTypeName();
                return nullptr;
            }
            
        } else {
            if (name.find_first_not_of("abcdefghijklmnopqrstuvwxyz._0123456789")
                != std::string::npos) {
            CIVAN_LOG_ERROR(CIVAN_LOG_ROOT()) << "Lookup name invalid" << name;
            throw std::invalid_argument(name);
            }
            return nullptr;
        }

    }

    static void LoadFromYaml(const YAML::Node& root);

    static ConfigVarBase::ptr LookupBase(const std::string& name);

    static void Visit(std::function<void(ConfigVarBase::ptr)> cb);

private:
    RWMutexType m_mutex;
    static ConfigVarMap& GetDatas() {
        static ConfigVarMap s_datas;
        return s_datas;
    }

    static RWMutexType& GetMutex() {
        static RWMutexType m_mutex;
        return m_mutex;
    }    
};



}  //namespace civan

#endif

