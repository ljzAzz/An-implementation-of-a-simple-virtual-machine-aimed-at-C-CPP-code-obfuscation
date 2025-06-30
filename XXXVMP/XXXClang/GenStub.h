#pragma once
#include <iostream>
#include <fstream>
#include <string>
#include <nlohmann/json.hpp>

constexpr const char *create_file_name = "/home/ljz/XXXVMP/XXXClang/build/XXXlib.json";
constexpr const char *stub_name = "/home/ljz/XXXVMP/XXXClang/build/XXXstub.h";

int type_count=0;
// 改进后的FunGen生成逻辑
std::string FunGen(std::string Func_Type, std::string name,std::string MangledName,
                   const std::vector<std::string> &params,
                   const std::string &returnType,const std::string is_CXXmember,bool isCXXConversion=false)
{
    // 判断是否为成员函数
    bool is_member = is_CXXmember=="true"?true:false;

    // 构造参数类型列表
    std::string param_list;
    for (size_t i = 0; i < params.size(); ++i)
    {
        if (i > 0)
            param_list += ", ";
        param_list += params[i];
    }

    // 生成函数指针类型
    std::string func_ptr_type;
    if (is_member)
    {
        size_t scope_pos = name.rfind("::");
        std::string class_name = name.substr(0, scope_pos);
        func_ptr_type = returnType + " (" + class_name + "::*)(" + param_list + ")"+(Func_Type.ends_with(")")?"":Func_Type.substr(Func_Type.find(")")+1));
    }
    else
    {
        func_ptr_type = returnType + " (*)(" + param_list + ")"+(Func_Type.ends_with(")")?"":Func_Type.substr(Func_Type.find(")")+1));
    }

    // 生成static_cast表达式
    std::string cast_expr;
    if (is_member&&!isCXXConversion)
    {
        cast_expr = "static_cast<" + func_ptr_type + ">(&" + name + ")";
    }else if(isCXXConversion)
    {
        cast_expr = "static_cast<" + func_ptr_type + ">(&" + name.substr(0,name.rfind(' ')+1)+ returnType + ")";
    }
    else
    {
        cast_expr = "static_cast<" + func_ptr_type + ">(&" + name + ")";
    }
    
   
    return cast_expr;

}

std::string VarGen(std::string Type, std::string name,std::string mangledName)
{
    return name;
}
int var_count=-1;
int func_count=-1;
std::set<std::string> CXXConstructorGen_set;
std::string CXXConstructorGen(std::string mname,std::string name,const std::vector<std::string>& params,const std::string &Func_Type){
    name = name.substr(0, name.rfind("::"));

    CXXConstructorGen_set.insert(mname);
    std::string param_list;
    for (size_t i = 0; i < params.size(); ++i)
    {
        if (i > 0)
            param_list += ", ";
        param_list += params[i];
    }
    func_count++;
    return "using t"+ std::to_string(func_count) +" = "+name+";";
}
std::string GenTemplate()
{
    return R"(
template<typename T>
auto VarGen(){
}

template<typename T>
auto FunGen(){
}

)";
}


std::string Map_Table(std::string mangledName, std::string type, bool isVar=true){
    std::string mapping;

    if(isVar){
        var_count++;
        return R"(
auto& )" + std::string("v")+std::to_string(var_count) + R"( = )" + type + R"(;
        )";
        
    }else{
        func_count++;
        return R"(
auto )" + std::string("f")+std::to_string(func_count) + R"( = )" + type + R"(;
        )";
        
    }
}
// std::string Mapinstance(std::string mangledName, std::string name, bool isVar=true)
// {
//     std::string mapping;
//     if (isVar)
//     {
//         return R"(
//         {)" + mangledName + R"(, VarGen<)" + name + R"(>()};
//         )";
//     }
//     else
//     {
//         return R"(
//         {)" + mangledName + R"(, FunGen<)" + name + R"(>()};
//         )";
//     }
// }
// std::string GenMappingTable(std::vector<std::tuple<std::string, std::string, bool>> &mappingTable)
// {
//     std::string table=R"(
// template<typename T>
// std::unordered_map<std::string, TypeMappingTable<T>> mappingTable = {
// )";
//     for (auto &item : mappingTable)
//     {
//         table += Mapinstance(std::get<0>(item), std::get<1>(item), std::get<2>(item));
//     }
//     table += R"(
// };
// )";
//     return table;
// }

std::vector<std::tuple<std::string, std::string, bool>> Collector;

bool GenStub()
{

    std::ifstream create_file(create_file_name);
    std::ofstream stub_file(stub_name);
    nlohmann::ordered_json j;
    if (!stub_file.is_open())
    {
        std::cout << "err, no stub file\n";
        return 0;
    }
    else
    {
        // 如果stub文件存在，先清空
        stub_file.clear();
    }
    if (create_file.is_open())
    {
        if (create_file.peek() == EOF)
        {
            std::cout << "err, no json info\n";
            abort();
            return 1;
        }
        create_file >> j;
        if (j.find("headers") != j.end())
        {
            stub_file << "#pragma once\n";
            std::cout << "handle headers\n";
            for (auto &header : j["headers"])
            {
               if(header.dump().find("<") != std::string::npos){
                stub_file << "#include " + header.dump().substr(1, header.dump().size() - 2) + "\n";
                }else{
                    stub_file << "#include \"" + header.dump().substr(3, header.dump().size() - 6) +"\"\n";
                }
            }
            if(j["headers"].find("<functional>") == j["headers"].end()){
                stub_file << "#include <functional>\n";
            }
            if(j["headers"].find("<new>") == j["headers"].end()){
                stub_file << "#include <new>\n";
            }
            stub_file << "\n";
        }
        stub_file << GenTemplate();
        if (j.find("Var") != j.end())
        {
            std::cout << "handle Var\n";
            for (auto &var : j["Var"])
            {
                // 也去掉引号

                auto cast =  VarGen(var["Type"].dump().substr(1, var["Type"].dump().size() - 2), var["name"].dump().substr(1, var["name"].dump().size() - 2),var["mangledName"].dump().substr(1,var["mangledName"].dump().size()-2));
                Collector.push_back(std::make_tuple(var["mangledName"].dump(), var["Type"].dump().substr(1, var["Type"].dump().size() - 2), true));
                stub_file << Map_Table(var["mangledName"].dump().substr(1,var["mangledName"].dump().size()-2), cast) << "\n";
                var["key"] = std::string("v") + std::to_string(var_count);
            }
            stub_file << "\n";
        }
        if (j.find("Func") != j.end())
        {
            std::cout << "handle Func\n";
            for (auto &func : j["Func"])
            {
                if(func["isCXXConstructor"]=="true"){
                    
                    auto s = CXXConstructorGen(func["signature"].dump().substr(1, func["signature"].dump().size() - 2),func["name"].dump().substr(1, func["name"].dump().size() - 2),func["params"],func["Func_Type"]);
                    if(s == "wrong"){
                        func_count++;
                        func["key"] = std::string("t") + std::to_string(func_count);
                        continue;
                    }else{
                        stub_file << s << "\n";
                        func["key"] = std::string("t") + std::to_string(func_count);
                    }
                    
                    continue;
                }
                bool isCXXConversion = false;
                if(func["isCXXConversionFunction"]=="true"){
                    isCXXConversion = true;
                }
                auto cast =  FunGen(func["Func_Type"].dump().substr(1, func["Func_Type"].dump().size() - 2), func["name"].dump().substr(1, func["name"].dump().size() - 2),func["mangledName"].dump().substr(1,func["mangledName"].dump().size()-2), func["params"], func["returnType"].dump().substr(1, func["returnType"].dump().size() - 2),func["isCXXClassMember"].dump().substr(1,func["isCXXClassMember"].dump().size()-2),isCXXConversion);

                Collector.push_back(std::make_tuple(func["mangledName"].dump(), func["Func_Type"].dump().substr(1, func["Func_Type"].dump().size() - 2), false));
                stub_file << Map_Table(func["mangledName"].dump().substr(1,func["mangledName"].dump().size()-2), cast, false) << "\n";
                func["key"] = std::string("f") + std::to_string(func_count);
            }
            stub_file << "\n";
        }
        // stub_file << GenMappingTable(Collector);
    }
    create_file.close();
    stub_file.close();
    std::ofstream create_file2(create_file_name);
    create_file2 << j.dump(4);
    create_file2.close();
    return true;
}