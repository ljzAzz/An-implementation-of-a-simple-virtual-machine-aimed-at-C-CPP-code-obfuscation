#pragma once
#include "XXXVMPCore.h"

#include <iostream>
#include <fstream>
#include <string>
#include <vector>

class RetTypeCollector
{   
    private:
        std::string retType;
        std::string TypeName;
    public:
        RetTypeCollector(std::string retType)
        {
            this->retType = retType;
        }
        std::string getRetType()
        {
            return this->retType;
        }
        void dump()
        {
            std::fstream out("retType.h", std::ios::out);
            out << R"(#pragma once)";
            out << "\n";
            out << R"(#include "XXXVMPCore.h")";
            out << "\n";
            out << retType;
            out << "\n";
            out << R"(template <>)";
            out << TypeName;
            out << " ";
            out << R"(XXXVMPCore::handleRET<)" + TypeName + R"(>(VMINST inst))";
            out << "\n";
            out << R"({
        }