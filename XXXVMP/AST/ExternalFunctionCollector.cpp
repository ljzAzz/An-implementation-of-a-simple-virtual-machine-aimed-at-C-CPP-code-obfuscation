#include <clang/AST/AST.h>
#include <clang/AST/ASTConsumer.h>
#include <clang/AST/ASTContext.h>
#include <clang/Basic/SourceManager.h>
#include <clang/AST/ParentMapContext.h>
#include <clang/Frontend/CompilerInstance.h>
#include <clang/Lex/HeaderSearch.h>
#include <clang/Lex/Preprocessor.h>
#include <clang/Frontend/FrontendActions.h>
#include <clang/Frontend/FrontendPluginRegistry.h>
#include <clang/AST/Mangle.h>
#include <clang/AST/Expr.h>
#include <clang/AST/Decl.h>
#include <clang/AST/RecursiveASTVisitor.h>
#include "llvm/Demangle/Demangle.h"
#include <nlohmann/json.hpp>
#include <fstream>
#include <iostream>
#include <vector>
#include <clang/AST/DeclObjC.h>
#include <clang/AST/Attr.h>
using namespace clang;

class preprocessor : public PPCallbacks
{
    std::once_flag flag;
    SourceManager &SM;

public:
    preprocessor(SourceManager &SM) : SM(SM) {}
    // 处理宏定义
    void MacroDefined(const Token &MacroNameTok, const MacroDirective *MD) override
    {
        // 输出宏定义
        // llvm::outs() << "Macro defined: " << MacroNameTok.getIdentifierInfo()->getName() << "\n";
    }
    // 处理头文件包含
    void InclusionDirective(SourceLocation HashLoc,
                            const Token &IncludeTok, StringRef FileName,
                            bool IsAngled, CharSourceRange FilenameRange,
                            OptionalFileEntryRef File,
                            StringRef SearchPath, StringRef RelativePath,
                            const Module *Imported,
                            SrcMgr::CharacteristicKind FileType) override
    {
        // 只处理直接包含的头文件，不处理间接包含
        nlohmann::ordered_json j;
        std::string create_file_name = "/home/ljz/XXXVMP/XXXClang/build/XXXlib.json";
        // 不存在就创建文件
        std::ifstream create_file(create_file_name);

        if (SM.isInMainFile(HashLoc))
        {
            llvm::errs() << "[Direct Include]： " << FileName << "\n";
            if (create_file.is_open())
            {
                if (create_file.peek() != EOF)
                {
                    create_file >> j;
                    if (j.find("headers") != j.end())
                    {
                        std::call_once(flag, [&j]
                                       { j["headers"].clear(); });
                        j["headers"].push_back(IsAngled ? "<" + FileName.str() + ">" : R"(")" + FileName.str() + R"(")");
                    }
                }
                else
                {
                    std::call_once(flag, [&j, &FileName, &IsAngled]
                                   {
                        j=nlohmann::json::object();
                        j["headers"]=std::vector<std::string>();
                        j["headers"].push_back(IsAngled ? "<" + FileName.str() + ">" : R"(")" + FileName.str() + R"(")"); });
                }
            }
            create_file.close();
            std::ofstream create_file2(create_file_name);
            create_file2 << j.dump(4);
            create_file2.close();
        }
    }
};

class ReferenceCollector : public RecursiveASTVisitor<ReferenceCollector>
{
private:
    ASTContext *Context;
    SourceManager &SM;
    std::set<Decl *> Decls_used;
    std::set<FunctionDecl *> User_defines;

public:
    explicit ReferenceCollector(ASTContext *Context, SourceManager &SM) : Context(Context), SM(SM) {
        
    }
        // 启用模板实例化遍历
    bool shouldVisitTemplateInstantiations() const {
        return true;  // 关键设置点
    }
    bool VisitDeclRefExpr(DeclRefExpr *DRE)
    {
        if (SM.isInMainFile(DRE->getBeginLoc()))
        {
            Decls_used.insert(DRE->getDecl());
        }
        return true;
    }
    bool VisitFunctionDecl(FunctionDecl *FD)
    {
        if (SM.isInMainFile(FD->getBeginLoc()))
        {
            User_defines.insert(FD);
        }
        return true;
    }
    bool VisitCallExpr(CallExpr *CE)
    {
        if (SM.isInMainFile(CE->getBeginLoc()))
        {
            Decls_used.insert(CE->getCalleeDecl());
        }
        return true;
    }
    bool VisitVarDecl(VarDecl *VD)
    {
        if (SM.isInMainFile(VD->getBeginLoc()))
        {
            Decls_used.insert(VD);
        }
        return true;
    }
    
    std::set<Decl *> getDecls_used()
    {
        return Decls_used;
    }
    std::set<FunctionDecl *> getUser_defines()
    {
        return User_defines;
    }
};

class ExternalFunctionCollector : public ASTConsumer
{
    ASTContext &Context;
    SourceManager &SM;
    CompilerInstance &CI;
    std::string CurrentFile;
    class ReferenceCollector RC;
    struct Extern                           //基类结构
    {
        std::string mangledName;            // 储存C++函数mangled name
        std::string signature;              // 储存函数签名
        std::string headerPath;             // 储存头文件路径
        std::string includeDirective;       // 储存头文件包含指令
        virtual ~Extern() {};
    };
    struct FunctionMeta : Extern                    //函数元数据
    {
        std::string name;                           // 未修饰的C++函数名称 
        std::string returnType;                     // 函数返回值类型
        std::vector<std::string> params;            // 函数参数列表
        std::string function_type;                  // 函数类型，已经弃用，用下面三个字段替代
        std::string isCXXClassMember = "false";     // 是否为类成员函数   
        std::string isCXXConstructor = "false";     // 是否为构造函数
        std::string isCXXDestructor  = "false";     // 是否为析构函数
        std::string isCXXConversionFunction = "false"; // 是否为转换函数
        virtual ~FunctionMeta() {};
    };
    struct VariableMeta : Extern                    //变量元数据
    {
        std::string type;                           // 变量类型
        virtual ~VariableMeta() {};
    };
    struct TemplateParamInfo
    {
        std::string type;
        std::string name;
        std::string defaultValue;
        std::string actualArgument;
        bool isParameterPack = false;
    };

    struct TemplateFunctionMeta : FunctionMeta
    {
        TemplateFunctionMeta() {}
        TemplateFunctionMeta(FunctionMeta base) : FunctionMeta(base) {}
        std::vector<TemplateParamInfo> templateParams;

        virtual ~TemplateFunctionMeta() {};
    };

    //自定义比较器，当且仅当两个Extern对象的mangledName相等时，才认为它们相等
    struct ExternCompare {
        bool operator()(const Extern* a, const Extern* b) const {
            return a->mangledName < b->mangledName;
        }
    };
    // 收集外部函数和全局变量的元数据
    std::set<Extern *,ExternCompare> externMeta;

    bool isUserDefined(const FunctionDecl *FD)   // 判断是否为用户自定义函数

    {
        // 允许处理std命名空间中的符号，因为用户可能重载了std命名空间中的函数或者实例化了模板
        if (FD->getEnclosingNamespaceContext()->isStdNamespace())
            return true;

        return !SM.isInSystemHeader(FD->getLocation()) &&
               !SM.isInExternCSystemHeader(FD->getLocation());
    }

    bool isTargetDeclaration(const FunctionDecl *FD)
    {
        // 系统头文件中的符号
        if (SM.isInSystemHeader(FD->getLocation()) || SM.isInExternCSystemHeader(FD->getLocation()))
        {
            return true;
        }

        // 用户代码中的外部可见符号
        return FD->isExternallyVisible() && !FD->hasBody();
    }
    // void ProcessCXXRecord(CXXRecordDecl *RD){
    //     llvm::outs() << "Processing CXXRecord: " << RD->getQualifiedNameAsString() << "\n";
    //     for (Decl* member : RD->decls()) {
    //         if (auto* method = dyn_cast<CXXMethodDecl>(member)) {
    //             // 判断是否为模板
    //             if (method->getDescribedFunctionTemplate()) {
    //                 // 这是成员函数模板
    //                 FunctionTemplateDecl* ftemplate = method->getDescribedFunctionTemplate();
    //                 ProcessTemplateFunctionDecl(ftemplate);
    //             }else{
    //                 ProcessFunctionDecl(method);
    //             }
    //         }else{
    //             llvm::outs() << "Not method: " << "\n";
    //             abort();
    //         }
    //     }
    // }
    void ProcessTemplateFunctionDecl(FunctionTemplateSpecializationInfo *FTD)
    {
        llvm::outs() << "Processing template function: " << "\n";
        if (!FTD)
        {
            llvm::outs() << "FTD is null\n";
            return;
        }
        MangleContext *MC = CI.getASTContext().createMangleContext();
        std::string MangledName;
        llvm::raw_string_ostream OS(MangledName);
        TemplateFunctionMeta *meta = new TemplateFunctionMeta();
        auto *FD = FTD->getTemplate();
        llvm::outs() << "Processing template function: " << FD->getQualifiedNameAsString() << "\n";
        MC->mangleName(FD->getAsFunction(), OS);
        meta->name = FD->getQualifiedNameAsString();
        meta->returnType = "";
        meta->signature = MangledName;
        meta->includeDirective = "";
        meta->headerPath = "";
        meta->function_type = "";
        auto templateParams = FTD->TemplateArguments->asArray();
        for (auto param : templateParams)
        {
            if (param.getKind() == TemplateArgument::ArgKind::Type)
            {
                TemplateParamInfo info;
                info.type = param.getAsType().getAsString();
                info.name = param.getAsType().getAsString();
                info.defaultValue = "";
                info.actualArgument = param.getAsType().getAsString();
                meta->templateParams.push_back(info);
            }
            else if (param.getKind() == TemplateArgument::ArgKind::Integral)
            {
                llvm::outs() << "Integral template argument: " << param.getAsIntegral() << "\n";
            }
            else if (param.getKind() == TemplateArgument::ArgKind::Template)
            {
                llvm::outs() << "Template template argument: " << param.getAsTemplate().getAsTemplateDecl()->getQualifiedNameAsString() << "\n";
            }
            else if (param.getKind() == TemplateArgument::ArgKind::Expression)
            {
                llvm::outs() << "Expression template argument: " << param.getAsExpr()->getStmtClassName() << "\n";
            }
            else if (param.getKind() == TemplateArgument::ArgKind::Pack)
            {
                llvm::outs() << "Pack template argument: " << param.pack_size() << "\n";
            }
            else
            {
                llvm::outs() << "Unknown template argument kind: " << param.getKind() << "\n";
            }
        }
        externMeta.insert(meta);
    }

    void TestFunction(clang::FunctionDecl *FD)
    {
        // 基本信息获取
        std::string funcName = FD->getNameAsString();
        SourceLocation loc = FD->getLocation();
        auto PP = FD->getASTContext().getPrintingPolicy();
        PP.SuppressScope = false;
        PP.FullyQualifiedName = true;
        PP.PrintCanonicalTypes = true;
        PP.PrintInjectedClassNameWithArguments = true;
        PP.SuppressUnwrittenScope = false;
        PP.SuppressTagKeyword = true;
        PP.SuppressDefaultTemplateArgs = false;
        llvm::outs() << "Function: " << FD->getQualifiedNameAsString() << "\n";

        // 1. 获取返回类型（处理可能存在的类型别名）
        QualType returnType = FD->getReturnType();
        std::string returnTypeStr = returnType.getAsString(
            PP);

        // 2. 参数列表处理
        std::vector<std::pair<std::string, std::string>> params;
        for (auto param : FD->parameters())
        {
            std::string paramName = param->getQualifiedNameAsString();
            if (paramName.empty())
                paramName = "(unnamed)";

            QualType paramType = param->getType();
            std::string typeStr = paramType.getAsString(
                PP);

            params.emplace_back(typeStr, paramName);
        }

        // 3. 模板处理（包括模板实例）
        if (FunctionTemplateDecl *FTD = FD->getPrimaryTemplate())
        {
            // 获取模板参数声明
            const TemplateParameterList *TPL = FTD->getTemplateParameters();

            // 处理每个模板参数
            for (auto param : *TPL)
            {
                if (TemplateTypeParmDecl *TTP = dyn_cast<TemplateTypeParmDecl>(param))
                {
                    // 类型模板参数处理
                    std::string paramName = TTP->getQualifiedNameAsString();
                    // ... 记录类型参数信息
                    llvm::outs() << "Type template parameter: " << paramName << "\n";
                }
                else if (NonTypeTemplateParmDecl *NTTP = dyn_cast<NonTypeTemplateParmDecl>(param))
                {
                    // 非类型模板参数处理
                    QualType paramType = NTTP->getType();
                    // ... 记录非类型参数信息
                    llvm::outs() << "Non-type template parameter: " << paramType.getAsString(PP) << "\n";
                }
            }
        }

        // 4. 处理模板实例化
        if (FD->isTemplateInstantiation())
        {
            if (const TemplateArgumentList *TAL =
                    FD->getTemplateSpecializationArgs())
            {

                // 遍历模板实例参数
                for (unsigned i = 0; i < TAL->size(); ++i)
                {
                    const TemplateArgument &TA = TAL->get(i);

                    // 处理不同类型的模板参数
                    if (TA.getKind() == TemplateArgument::Type)
                    {
                        QualType T = TA.getAsType();
                        std::string typeName = T.getAsString(PP);
                        // ... 记录类型参数
                        llvm::outs() << "Template type argument: " << typeName << "\n";
                    }
                    // 处理非类型参数...
                    else if (TA.getKind() == TemplateArgument::Integral)
                    {
                        llvm::outs() << "Template integral argument: " << TA.getAsIntegral() << "\n";
                    }
                    else if (TA.getKind() == TemplateArgument::Expression)
                    {
                        llvm::outs() << "Template expression argument: " << TA.getAsExpr()->getStmtClassName() << "\n";
                    }
                    else if (TA.getKind() == TemplateArgument::Pack)
                    {
                        llvm::outs() << "Template pack argument: " << TA.pack_size() << "\n";
                    }
                    else
                    {
                        llvm::outs() << "Unknown template argument kind: " << TA.getKind() << "\n";
                    }
                }
            }
        }

        // 输出调试信息
        llvm::errs() << "Function: " << returnTypeStr << " " << funcName << "(";
        for (size_t i = 0; i < params.size(); ++i)
        {
            if (i > 0)
                llvm::errs() << ", ";
            llvm::errs() << params[i].first << " " << params[i].second;
        }
        llvm::errs() << ")\n";
    }
    void ProcessFunctionDecl(FunctionDecl *FD)
    {
        FunctionMeta *meta = new FunctionMeta();

        if (FD->isTemplated())
        {
            llvm::outs() << "isTemplated function: " << FD->getQualifiedNameAsString() << "\n";
        }
        if (FD->isTemplateDecl())
        {
            llvm::outs() << "isTemplateDecl function: " << FD->getQualifiedNameAsString() << "\n";
        }
        if (FD->isTemplateInstantiation())
        {
            llvm::outs() << "isTemplateInstantiation function: " << FD->getQualifiedNameAsString() << "\n";
            auto *FTD = FD->getTemplateSpecializationInfo();
            // ProcessTemplateFunctionDecl(FTD);
        }
        if (FD->isExternCXXContext())
        {
            llvm::outs() << "isCXXOperatorStaticMember function: " << FD->getQualifiedNameAsString() << "\n";
        }
        if(FD->isCXXClassMember())
        {
            //分析类成员函数属于哪个类
            meta->isCXXClassMember = "true";
            llvm::outs() << "isCXXClassMember function: " << FD->getQualifiedNameAsString() << "\n";
        }
        if(FD->isCXXInstanceMember())
        {
            meta->isCXXClassMember = "true";
            llvm::outs() << "isCXXInstanceMember function: " << FD->getQualifiedNameAsString() << "\n";
        }
        if(FD->isExternCXXContext())
        {
            llvm::outs() << "isCXXOperatorStaticMember function: " << FD->getQualifiedNameAsString() << "\n";
        }
        // llvm::outs() << "Processing function: " << FD->getQualifiedNameAsString()
        //              << " | hasBody: " << FD->hasBody()
        //              << " | isExternallyVisible: " << FD->isExternallyVisible()
        //              << "\n";
        if (isTargetDeclaration(FD))
        {
            
            // 带有模板参数的函数名
            llvm::outs() << "Found function: " << FD->getQualifiedNameAsString() << "\n";
            clang::MangleContext *MC = CI.getASTContext().createMangleContext();
            std::string MangledName;
            llvm::raw_string_ostream OS(MangledName);
            if (MC)
            {
                if (auto *Ctor = dyn_cast<CXXConstructorDecl>(FD))
                {
                    // 处理构造函数
                    GlobalDecl GD(Ctor, Ctor_Complete); // 必须指定构造类型

                    MC->mangleName(GD, OS); // 传入GlobalDecl而非Decl
                }
                else if (auto *Dtor = dyn_cast<CXXDestructorDecl>(FD))
                {
                    // 处理析构函数同理,但是析构函数还是跳过吧
                    GlobalDecl GD(Dtor, Dtor_Complete);
                    MC->mangleName(GD, OS);
                    llvm::outs()<<"SKIP CXXDestructor: "<<FD->getQualifiedNameAsString()<<"\n";
                    return;
                }
                else
                {
                    // 普通函数处理
                    MC->mangleName(FD, OS);
                }
            }
            if(llvm::dyn_cast<CXXConstructorDecl>(FD))
            {
                meta->isCXXConstructor = "true";
            }
            else if(llvm::dyn_cast<CXXDestructorDecl>(FD))
            {
                meta->isCXXDestructor = "true";
            }
            if(SM.isInExternCSystemHeader(FD->getLocation()))
            {
                
                MangledName.clear();
                MangledName = FD->getName();
                llvm::outs() << "Found externC function: " << FD->getName() << "\n";

            }

            llvm::outs() << "Mangled name: " << MangledName << "\n";
            meta->mangledName = MangledName;
            // llvm新的打印方式
            PrintingPolicy PP = Context.getPrintingPolicy();
            // 打印策略配置：
            PP.SuppressScope = false;                      // 显示完整作用域
            PP.FullyQualifiedName = false;                 // 强制全限定名称
            PP.PrintCanonicalTypes = true;                 // 使用规范类型名称
            PP.PrintInjectedClassNameWithArguments = true; // 对CRTP模式有效
            PP.SuppressUnwrittenScope = false;             // 显示所有作用域
            PP.SuppressTagKeyword = true;                  // 移除冗余的class/struct关键字
            PP.SuppressDefaultTemplateArgs = false;        // 移除默认模板参数


            if (const CXXConversionDecl* ConvDecl = dyn_cast<CXXConversionDecl>(FD)) {
                QualType ConvType = ConvDecl->getConversionType();
                PrintingPolicy Policy = ConvDecl->getASTContext().getPrintingPolicy();
                Policy.PrintCanonicalTypes = true;
                meta->returnType = ConvType.getAsString(Policy);
                meta->isCXXConversionFunction = "true";
            }else{
                meta->returnType = FD->getReturnType().getAsString(PP);
            }
            
            std::string qualifiedName;

            llvm::raw_string_ostream os(qualifiedName);
            FD->printQualifiedName(os, PP); // 使用printQualifiedName代替getQualifiedNameAsString

            meta->name = FD->getQualifiedNameAsString();

            for (auto param : FD->parameters())
            {
                meta->params.push_back(param->getType().getAsString(PP));
            }

            // 获取头文件信息
            SourceLocation Loc = FD->getLocation();
            if (Loc.isValid())
            {
                meta->headerPath = SM.getFilename(Loc).str();

                // 使用现代头文件搜索接口
                if (auto FE = SM.getFileEntryRefForID(SM.getFileID(Loc)))
                {
                    clang::HeaderSearch &HS = CI.getPreprocessor().getHeaderSearchInfo();
                    SmallString<128> IncludeName;
                    bool IsAngled = HS.getFileDirFlavor(*FE) == SrcMgr::CharacteristicKind::C_System;
                    HS.suggestPathToFileForDiagnostics(*FE, IncludeName, &IsAngled);
                    meta->includeDirective = IncludeName.str().str();
                }
            }
            meta->function_type = FD->getType().getAsString(PP);
            meta->signature = demangle(MangledName);

            externMeta.insert(meta);
            if (MC)
            {
                OS.flush();
                MangledName.clear();
                if (auto *Ctor = dyn_cast<CXXConstructorDecl>(FD))
                {
                    // 处理构造函数
                    GlobalDecl GD(Ctor, Ctor_Base); // 必须指定构造类型

                    MC->mangleName(GD, OS); // 传入GlobalDecl而非Decl
                }
                else if (auto *Dtor = dyn_cast<CXXDestructorDecl>(FD))
                {
                    // 处理析构函数同理,但是析构函数还是跳过吧
                    GlobalDecl GD(Dtor, Dtor_Base);
                    MC->mangleName(GD, OS);
                    llvm::outs()<<"SKIP CXXDestructor: "<<FD->getQualifiedNameAsString()<<"\n";
                    return;
                }
                else
                {
                    // 普通函数处理
                    MC->mangleName(FD, OS);
                }
                FunctionMeta *baseMeta = new FunctionMeta();
                *baseMeta = *meta;
                baseMeta->mangledName = MangledName;
                baseMeta->signature = demangle(OS.str());
                externMeta.insert(baseMeta);
            }
        }
    }

    void ProcessOperatorOverload(FunctionDecl *FD)
    {
        FunctionMeta *meta = new FunctionMeta();
        // 配置打印策略,打印出模板参数类似std::operator<<<std::char_traits<char>>
        PrintingPolicy PP = Context.getPrintingPolicy();
        PP.SuppressScope = false;
        PP.FullyQualifiedName = true;
        PP.PrintCanonicalTypes = true;
        PP.PrintInjectedClassNameWithArguments = true;
        // 获取返回值类型
        meta->returnType = FD->getReturnType().getAsString(PP);

        // 获取带模板参数的完整名称
        std::string qualifiedName;
        {
            llvm::raw_string_ostream os(qualifiedName);
            FD->printQualifiedName(os, PP); // 使用printQualifiedName代替getQualifiedNameAsString
        }
        meta->signature = qualifiedName;
        // 参数处理
        for (auto *Param : FD->parameters())
        {
            meta->params.push_back(Param->getType().getAsString(PP));
        }

        externMeta.insert(meta);
    }

    std::string demangle(const std::string &name)
    {
        char *demangled = llvm::itaniumDemangle(name.c_str());
        if (!demangled)
            return name;
        std::string result(demangled);
        std::free(demangled);
        return result;
    }

public:
    ExternalFunctionCollector(ASTContext &Context, CompilerInstance &CI)
        : Context(Context), SM(Context.getSourceManager()), CI(CI), RC(&Context, SM)
    {
        // 使用现代文件路径获取方式
        if (auto FE = SM.getFileEntryRefForID(SM.getMainFileID()))
        {
            CurrentFile = FE->getName().str();
        }
    }

    void generateBinding(std::set<Extern *,ExternCompare> Meta,
                         const std::vector<std::string> &opts, const std::set<FunctionDecl *> &FDs)
    {
        constexpr const char *create_file_name = "/home/ljz/XXXVMP/XXXClang/build/XXXlib.json";
        nlohmann::ordered_json f;
        nlohmann::json jv = nlohmann::json::array();
        nlohmann::json jf = nlohmann::json::array();
        nlohmann::json uf = nlohmann::json::array();
        nlohmann::json tf = nlohmann::json::array();
        std::ifstream create_file(create_file_name);
        if (create_file.is_open())
        {
            if (create_file.peek() == EOF)
            {
                llvm::outs() << "err, no headers info\n";
                return;
            }
            create_file >> f;
            if (f.find("Var") != f.end())
            {
                f["Var"].clear();
            }
            if (f.find("Func") != f.end())
            {
                f["Func"].clear();
            }
            if(f.find("UserFunc") != f.end())
            {
                f["UserFunc"].clear();
            }
            if(f.find("Native") != f.end())
            {
                f["Native"].clear();
            }
        }
        create_file.close();

        std::cout << "\n// Linker options:\n";
        for (const auto &opt : opts)
        {
            if (opt.starts_with("-l") || opt.starts_with("-L") || opt.starts_with("-I"))
            {
                std::cout << opt << " ";
            }
        }

        std::cout << "\n\nstruct VMBinding {\n";
        for (auto *M : Meta)
        {
            llvm::outs() << "    " << M->signature << ";\n";

            if (auto f = dynamic_cast<FunctionMeta *>(M))
            {

                llvm::outs() << "    " << "function_type: " << f->function_type << "\n";
                llvm::outs() << "    " << "name: " << f->name << "\n";
                llvm::outs() << "    " << "returnType: " << f->returnType << "\n";
                llvm::outs() << "    " << "params: ";
                nlohmann::json jp = nlohmann::json::array();
                for (auto &p : f->params)
                {
                    llvm::outs() << p << " ,  ";
                    jp.push_back(p);
                }
                llvm::outs() << "\n";
                nlohmann::json j = nlohmann::json::object();
                j["signature"] = f->signature;
                j["mangledName"]=f->mangledName;
                j["Func_Type"] = f->function_type;
                j["name"] = f->name;
                j["returnType"] = f->returnType;
                j["params"] = jp;
                j["isCXXClassMember"] = f->isCXXClassMember;
                j["isCXXConstructor"] = f->isCXXConstructor;
                j["isCXXDestructor"] = f->isCXXDestructor;
                j["isCXXConversionFunction"] = f->isCXXConversionFunction;
                jf.push_back(j);
            }
            else if (auto v = dynamic_cast<VariableMeta *>(M))
            {
                std::cout << "    " << "Var.Type     " << v->type << "\n";
                nlohmann::json j = nlohmann::json::object();
                j["signature"] = v->signature;
                j["Type"] = v->type;
                j["mangledName"]=v->mangledName;
                j["name"] = v->signature;
                jv.push_back(j);
            }
            else if (auto f = dynamic_cast<TemplateFunctionMeta *>(M))
            {
                llvm::outs() << "    " << "function_type: " << f->function_type << "\n";
                llvm::outs() << "    " << "name: " << f->name << "\n";
                for (auto &p : f->templateParams)
                {
                    llvm::outs() << "    " << "templateParam: " << p.type << " " << p.name << " = " << p.defaultValue << " " << p.actualArgument << "\n";
                }
                llvm::outs() << "\n";
                llvm::outs() << "    " << "returnType: " << f->returnType << "\n";
                llvm::outs() << "    " << "params: ";
                for (auto &p : f->params)
                {
                    llvm::outs() << p << " ,  ";
                }
                llvm::outs() << "\n";
                nlohmann::json j = nlohmann::json::object();
                j["Func_Type"] = f->function_type;
                j["name"] = f->name;
                nlohmann::json jp = nlohmann::json::object();
                for (auto &p : f->params)
                {
                    jp.push_back(p);
                }
                j["params"] = jp;
                nlohmann::json jt = nlohmann::json::object();
                for (auto &p : f->templateParams)
                {
                    jt[p.name] = p.actualArgument;
                }
                j["templateParams"] = jt;
                jf.push_back(j);
            }
        }
        clang::MangleContext *MC = CI.getASTContext().createMangleContext();

        for (auto *FD : FDs)
        {
            nlohmann::json j = nlohmann::json::object();
            if(FD->hasAttr<AnnotateAttr>()){
                const auto *attr = FD->getAttr<AnnotateAttr>();
                if(attr->getAnnotation() == "protect")
                {
                    llvm::outs() << "Found protect function: " << FD->getQualifiedNameAsString() << "\n";
                    std::string MangledName;
                    llvm::raw_string_ostream OS(MangledName);
                    if (MC)
                    {
                        MC->mangleName(FD, OS);
                    }
                    j["mangleName"] = MangledName;
                    j["returnType"] = FD->getReturnType().getAsString();
                    for(auto *P : FD->parameters())
                    {
                        j["params"].push_back(P->getType().getAsString());
                    }
                    tf.push_back(j);

                }
            }
            llvm::outs() << "    " << FD->getName() << ";\n";
            std::string MangledName;
            llvm::raw_string_ostream OS(MangledName);
            if (MC)
            {
                MC->mangleName(FD, OS);
            }
            j["mangledName"]=MangledName;
            j["name"] = FD->getName().str();

            uf.push_back(j);
        }

        f["Var"] = jv;
        f["Func"] = jf;
        f["UserFunc"] = uf;
        f["Native"] = tf;
        std::ofstream create_file2(create_file_name);
        create_file2 << f.dump(4);
        create_file2.close();

        std::cout << "};\n";

        return;
    }

    void ProcessDeclaration(Decl *D)
    {
        // llvm::outs() << "Processing declaration: ";
        // 中期答辩过后再改
        if (VarDecl *VD = dyn_cast<VarDecl>(D))
        {

            auto T_ = VD->getType();        //获取变量类型
            T_ = T_.getCanonicalType();
                
            if (T_->isBuiltinType()) {  // 基本类型判断
                
                llvm::outs() << "Primitive type: " << T_.getAsString() << "\n"; // 跳过处理int/float等基础类型
            }
        
            
            if (const CXXRecordDecl *RD = T_->getAsCXXRecordDecl()) {   // 类类型判断（包括struct/union/enum class）
                llvm::outs() << "Class type: " << RD->getNameAsString() << "\n";
                for(auto method : RD->methods()){    //如果是外部类/结构体，将声明为public的所有成员方法加入到FunctionMeta中
                    llvm::outs() << "Method: " << method->getQualifiedNameAsString() << "\n";
                    if (method->getAccess() == AS_private) {
                        llvm::outs() << "Private method\n";
                    } else if (method->getAccess() == AS_protected) {
                        llvm::outs() << "Protected method\n";
                    }
                    else if(method->isReachable()){             //额外判断，只有可以到达的类型方法才加入
                        ProcessFunctionDecl(method);
                    }
                }
                // 测试，看看是不是STL类型
                if (RD->isInStdNamespace()) {
                    llvm::outs() << "  (STL standard type)\n";
                }
            }
            if (SM.isInSystemHeader(VD->getLocation()) || SM.isInExternCSystemHeader(VD->getLocation())) // 暂时这样处理
            {

                VariableMeta *meta = new VariableMeta();
                meta->signature = VD->getNameAsString();
                clang::MangleContext *MC = CI.getASTContext().createMangleContext();
                std::string MangledName;
                llvm::raw_string_ostream OS(MangledName);
                if (MC)
                {
                    if (auto *Ctor = dyn_cast<CXXConstructorDecl>(VD))
                    {
                        // 处理构造函数
                        GlobalDecl GD(Ctor, Ctor_Complete); // 必须指定构造类型

                        MC->mangleName(GD, OS); // 传入GlobalDecl而非Decl
                    }
                    else if (auto *Dtor = dyn_cast<CXXDestructorDecl>(VD))
                    {
                        // 处理析构函数同理
                        GlobalDecl GD(Dtor, Dtor_Complete);
                        MC->mangleName(GD, OS);
                    }
                    else
                    {
                        // 普通函数处理
                        MC->mangleName(VD, OS);
                    }
                }
                llvm::outs() << "Mangled name: " << MangledName << "\n";
                meta->mangledName = MangledName;
                meta->signature = demangle(MangledName);
                // 使用类型反糖化处理
                QualType T = VD->getType().getDesugaredType(Context);
                if (T->isReferenceType())
                {
                    T = T->getPointeeType();
                    meta->type = T.getAsString(Context.getPrintingPolicy()) + "&";
                }
                else
                {
                    meta->type = T.getAsString(Context.getPrintingPolicy());
                }

                externMeta.insert(meta);
            }
        }
        else if (FunctionDecl *FD = dyn_cast<FunctionDecl>(D))
        {
            // TestFunction(FD);
            ProcessFunctionDecl(FD);
        }
    }

    void
    HandleTranslationUnit(ASTContext &Context) override
    {
        // 正确的遍历方式：使用递归遍历所有声明
        RC.TraverseDecl(Context.getTranslationUnitDecl());
        for (auto *D : RC.getDecls_used())
        {
            TraverseDecl(D);
        }
        // 添加调试输出
        llvm::outs() << "Collected " << externMeta.size() << " symbols:\n";
        for (const auto &f : externMeta)
        {
            llvm::outs() << " - " << f->signature << "\n";
        }
        // 改用前端原始参数
        const auto &FrontendOpts = CI.getFrontendOpts();
        std::vector<std::string> opts;
        for (const auto &arg : FrontendOpts.Inputs)
        {
            opts.push_back(arg.getFile().str());
        }
        for (const auto &arg : FrontendOpts.LLVMArgs)
        {
            opts.push_back(arg);
        }

        // 获取编译诊断选项中的参数
        const auto &DiagOpts = CI.getDiagnosticOpts();
        for (const auto &inc : CI.getHeaderSearchOpts().UserEntries)
        {
            opts.push_back("-I" + inc.Path);
        }
        generateBinding(externMeta, opts, RC.getUser_defines()); // 使用正确的参数列表
    }
    // 修改后的TraverseDecl逻辑
    bool TraverseDecl(Decl *D)
    {
        // 判断这个Decl是否被主文件使用？
        // llvm::outs() << "Decl is used in main file\n";

        // if (!D)
        //     return true;

        // // 先处理模板特化
        // if (auto *FTD = dyn_cast<FunctionTemplateDecl>(D))
        // {
        //     // 处理显式特化
        //     for (auto *Spec : FTD->specializations())
        //     {
        //         TraverseDecl(Spec->getAsFunction());
        //     }
        //     // 处理隐式实例化
        //     if (auto *FD = FTD->getTemplatedDecl())
        //     {
        //         TraverseDecl(FD);
        //         // 处理所有实例化
        //         for (auto *Spec : FTD->specializations())
        //         {
        //             TraverseDecl(Spec->getAsFunction());
        //         }
        //     }
        // }else{
        //     // 原处理逻辑
        //     ProcessDeclaration(D);
        // }

        ProcessDeclaration(D);

        // 递归处理子声明
        // if (auto *Ctx = dyn_cast<DeclContext>(D))
        // {
        //     for (auto *Child : Ctx->decls())
        //     {
        //         TraverseDecl(Child);
        //     }
        // }
        return true;
    }
            // 启用模板实例化遍历
    bool shouldVisitTemplateInstantiations() const {
        return true;  // 关键设置点
    }
};

class ExternCollectorAction : public PluginASTAction
{
private:
    class preprocessor *pp;

protected:
    std::unique_ptr<ASTConsumer> CreateASTConsumer(
        CompilerInstance &CI, StringRef InFile) override
    {
        return std::make_unique<ExternalFunctionCollector>(
            CI.getASTContext(), CI);
    }

    bool ParseArgs(const CompilerInstance &CI,
                   const std::vector<std::string> &args) override
    {
        CI.getPreprocessor().addPPCallbacks(std::make_unique<preprocessor>(CI.getSourceManager()));
        // 可在此处理插件参数
        // 打印参数

        for (const auto &arg : args)
        {
            std::cout << "Plugin argument: " << arg << "\n";
        }
        if (args.size() == 0)
        {
            std::cout << "No arguments\n";
        }
        return true;
    }

    PluginASTAction::ActionType getActionType() override
    {
        return AddAfterMainAction;
    }
};

// 注册插件
static FrontendPluginRegistry::Add<ExternCollectorAction>
    X("extern-collector", "Collect external function declarations");
