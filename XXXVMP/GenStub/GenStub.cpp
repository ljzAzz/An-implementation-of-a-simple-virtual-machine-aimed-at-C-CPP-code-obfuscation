#include <iostream>
#include <fstream>
#include <string>
#include <nlohmann/json.hpp>

constexpr const char *create_file_name  ="/home/ljz/XXXVMP/XXXClang/build/XXXlib.json";
constexpr const char *stub_name = "/home/ljz/XXXVMP/XXXClang/build/XXXstub.cpp";

#define FunGen(Type,Name) R"(\
template<>\
auto& FuncGen<Type>(std::string Name){\
    static_cast<Type>(&Name);\
}\
)"



#define VarGen(Type,Name) R"(\
template<>\
auto& VarGen<Type>(std::string Name){\
    static_cast<Type>(&Name);\
}\
)"
int main(){
    
    std::ifstream create_file(create_file_name);
    std::ofstream stub_file(stub_name);
    nlohmann::ordered_json j;
    if(!stub_file.is_open()){
        std::cout<<"err, no stub file\n";
        return 0;
    }
    if(create_file.is_open()){
        if(create_file.peek() == EOF){
            std::cout<<"err, no json info\n";
            abort();
            return 1;
        }
        create_file >> j;
        if(j.find("headers")!=j.end()){
            std::cout<<"handle headers\n";
            if(j["headers"].find("<string>")==j["headers"].end()){
                stub_file<<"#include <string>\n";
            }
            for(auto &header:j["headers"]){
                stub_file<<"#include "<<header.dump().substr(1,header.dump().size()-2)<<"\n";
            }
            stub_file<<"\n";
        }
        if(j.find("Var")!=j.end()){
            std::cout<<"handle Var\n";
            for(auto &var:j["Var"]){
                stub_file<<VarGen(var["type"],var["name"])<<"\n";
            }
            stub_file<<"\n";
        }
        if(j.find("Func")!=j.end()){
            std::cout<<"handle Func\n";
            for(auto &func:j["Func"]){
                stub_file<<FunGen(func["Func_Type"],func["name"])<<"\n";
            }
            stub_file<<"\n";
        }
    }
    create_file.close();
    return 0;
}