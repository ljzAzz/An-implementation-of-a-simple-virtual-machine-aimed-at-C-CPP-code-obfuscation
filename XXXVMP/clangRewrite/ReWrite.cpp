#include "clang/AST/AST.h"
#include "clang/AST/ASTConsumer.h"
#include "clang/AST/RecursiveASTVisitor.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Frontend/FrontendPluginRegistry.h"
#include "clang/Rewrite/Core/Rewriter.h"
#include "clang/Lex/Lexer.h"
#include "llvm/Support/raw_ostream.h"
#include "clang/AST/ASTContext.h"
#include "clang/AST/ParentMapContext.h"
#include <string>
#include <set>
#include <functional>
#include <sstream>
#include <iomanip>
#include <iostream>
using namespace clang;
using namespace llvm;

class FunctionCallVisitor : public RecursiveASTVisitor<FunctionCallVisitor> {
private:
    ASTContext *Context;
    Rewriter &TheRewriter;
    std::set<std::string> &ChangedFiles;

public:
    explicit FunctionCallVisitor(ASTContext *Context, Rewriter &R, std::set<std::string> &ChangedFiles)
        : Context(Context), TheRewriter(R), ChangedFiles(ChangedFiles) {}

    // 检查函数是否有 "protect" 属性
    bool hasProtectAttribute(const FunctionDecl *FD) {
        if (!FD)
            return false;

        for (const auto *Attr : FD->attrs()) {
            if (isa<AnnotateAttr>(Attr)) {
                const AnnotateAttr *AA = cast<AnnotateAttr>(Attr);
                StringRef AnnotationText = AA->getAnnotation();
                
                if (AnnotationText.equals("protect")) {
                    return true;
                }
            }
        }
        return false;
    }

    // 获取赋值语句的左侧变量
    std::string findAssignmentTarget(const CallExpr *CallE) {
        if (!CallE)
            return "";

        // 检查父节点
        const auto &Parents = Context->getParents(*CallE);
        if (Parents.empty())
            return "";

        // 检查是否是变量声明语句
        if (const VarDecl *VD = Parents[0].get<VarDecl>()) {
            if (VD->getInit() == CallE) {
                return VD->getNameAsString();
            }
        }
        // 检查是否是赋值语句
        else if (const BinaryOperator *BO = Parents[0].get<BinaryOperator>()) {
            if (BO->getOpcode() == BO_Assign && BO->getRHS() == CallE) {
                if (const DeclRefExpr *DRE = dyn_cast<DeclRefExpr>(BO->getLHS()->IgnoreParenImpCasts())) {
                    return DRE->getDecl()->getNameAsString();
                }
            }
        }

        return "";
    }
    
    std::string getSourceText(const SourceRange &Range) {
        if (Range.isInvalid())
            return "";

        SourceManager &SM = Context->getSourceManager();
        const LangOptions &LangOpts = Context->getLangOpts();

        // 获取开始和结束位置
        SourceLocation BeginLoc = Range.getBegin();
        SourceLocation EndLoc = Range.getEnd();

        // 处理宏展开
        if (BeginLoc.isMacroID())
            BeginLoc = SM.getSpellingLoc(BeginLoc);
        if (EndLoc.isMacroID())
            EndLoc = SM.getSpellingLoc(EndLoc);

        // 获取精确的结束位置
        EndLoc = Lexer::getLocForEndOfToken(EndLoc, 0, SM, LangOpts);

        // 确保位置有效
        if (BeginLoc.isInvalid() || EndLoc.isInvalid())
            return "";

        // 获取源代码文本
        bool Invalid = false;
        const char *Begin = SM.getCharacterData(BeginLoc, &Invalid);
        if (Invalid)
            return "";

        const char *End = SM.getCharacterData(EndLoc, &Invalid);
        if (Invalid)
            return "";

        // 计算文本长度
        unsigned Length = End - Begin;
        return std::string(Begin, Length);
    }
    
    // 访问函数调用表达式
    bool VisitCallExpr(CallExpr *CallE) {
        // 获取被调用的函数
        FunctionDecl *CalledFunc = CallE->getDirectCallee();
        if (!CalledFunc)
            return true;  // 跳过间接调用
    
        // 跳过系统头文件中的调用
        SourceLocation Loc = CallE->getBeginLoc();
        if (Loc.isInvalid() || Context->getSourceManager().isInSystemHeader(Loc))
            return true;


        // 检查是否有 protect 属性
        if (!hasProtectAttribute(CalledFunc))
            return true;
    
        // 获取返回类型是否为 void
        bool isVoid = CalledFunc->getReturnType()->isVoidType();
    
        // 获取函数名
        std::string FuncName = CalledFunc->getNameInfo().getAsString();
    
        // 跳过 InitVMPnative 函数本身的调用
        if (FuncName == "InitVMPnative")
            return true;
    
        // 查找可能的赋值目标
        std::string VarName = findAssignmentTarget(CallE);
    
        // 构建新的函数调用字符串
        std::string NewCall = "InitVMPnative(" + std::string(isVoid ? "true" : "false");
    
        // 添加返回值地址作为第二个参数
        if (!isVoid && !VarName.empty()) {
            NewCall += ", (void*)&" + VarName;
        } else {
            NewCall += ", nullptr";
        }
    
        // 添加原始参数
        for (unsigned i = 0; i < CallE->getNumArgs(); ++i) {
            NewCall += ", ";
            std::string ArgText = getSourceText(CallE->getArg(i)->getSourceRange());
            if (ArgText.empty()) {
                // 如果无法获取源文本，使用一个安全的默认值
                NewCall.pop_back(); // 删除最后一个逗号
                NewCall.pop_back(); 
                break;
            } else {
                NewCall += ArgText;
            }
        }
        NewCall += ")";
        std::cout << "Rewriting function call: " << FuncName << " to " << NewCall << "\n";
        
        // 获取函数调用的源位置
        SourceManager &SM = Context->getSourceManager();
        const LangOptions &LangOpts = Context->getLangOpts();

        SourceLocation BeginLoc = CallE->getBeginLoc();
        SourceLocation EndLoc = CallE->getEndLoc();
    
        if (BeginLoc.isInvalid() || EndLoc.isInvalid()) {
            llvm::errs() << "Invalid source locations for function call\n";
            return true;
        }
    
        // 处理宏展开
        if (BeginLoc.isMacroID()) {
            BeginLoc = SM.getSpellingLoc(BeginLoc);
        }
        if (EndLoc.isMacroID()) {
            EndLoc = SM.getSpellingLoc(EndLoc);
        }
    
        // 获取结束位置
        EndLoc = Lexer::getLocForEndOfToken(EndLoc, 0, SM, LangOpts);
    
        // 确保位置有效
        if (BeginLoc.isInvalid() || EndLoc.isInvalid() || 
            SM.getFileID(BeginLoc) != SM.getFileID(EndLoc)) {
            llvm::errs() << "Invalid source locations after processing\n";
            return true;
        }
    
        // 计算字符范围长度
        unsigned Length = SM.getFileOffset(EndLoc) - SM.getFileOffset(BeginLoc);
        if (Length == 0) {
            llvm::errs() << "Zero-length range\n";
            return true;
        }
        
        std::cout << "Replacing text from " << SM.getFileOffset(BeginLoc) 
                  << " to " << SM.getFileOffset(EndLoc) << "\n";
                  
        // 替换文本
        bool Success = false;
        try {
            if(TheRewriter.isRewritable(BeginLoc)) {
                TheRewriter.setSourceMgr(SM, LangOpts);
                std::cout << "FileName: " << SM.getFilename(BeginLoc).str() << "\n";
                std::cout << "Source location is rewritable\n";
            } else {
                std::cout << "Source location is not rewritable\n";
                abort();
            }
            // 使用字符偏移替代 CharSourceRange
            Success = TheRewriter.RemoveText(BeginLoc, Length);
            std::cout << "Text removed successfully\n";
            Success = TheRewriter.InsertText(BeginLoc, NewCall);
            std::cout << "Text inserted successfully\n";
            std::cout << "Original Text: " << getSourceText(CallE->getSourceRange()) << "\n";
            
            // 记录被修改的文件
            std::string Filename = SM.getFilename(BeginLoc).str();
            if (!Filename.empty()) {
                ChangedFiles.insert(Filename);
                llvm::errs() << "Modified file: " << Filename << "\n";
                auto Range = CalledFunc->getSourceRange();
                //移除该函数声明
                TheRewriter.RemoveText(Range);
            }
            
            return true;
        } catch (...) {
            llvm::errs() << "Unknown exception during text replacement\n";
            return true;
        }
    
        return true;
    }
};


class FunctionCallRewriterConsumer : public ASTConsumer {
private:
    Rewriter TheRewriter;
    FunctionCallVisitor Visitor;
    std::set<std::string> ChangedFiles;
    std::hash<std::string> hashString;

public:
    explicit FunctionCallRewriterConsumer(CompilerInstance &CI)
        : TheRewriter(CI.getSourceManager(), CI.getLangOpts()), 
          Visitor(&CI.getASTContext(), TheRewriter, ChangedFiles) {}

    
    void HandleTranslationUnit(ASTContext &Context) override {
        
        std::cout << "Visiting translation unit...\n";
        Visitor.TraverseDecl(Context.getTranslationUnitDecl());
        std::cout << "Finished visiting translation unit\n";
        
        rewriteToNewFile();
    }
    
    // 将修改后的源代码写入文件
    void rewriteToNewFile() {
        // 遍历所有缓冲区，处理所有被修改的文件
        for (auto it = TheRewriter.buffer_begin(); it != TheRewriter.buffer_end(); ++it) {
            const FileID &FileID = it->first;
            const RewriteBuffer &RewriteBuf = it->second;
            
            // 获取文件条目
            const FileEntry *FileEntry = TheRewriter.getSourceMgr().getFileEntryForID(FileID);
            if (!FileEntry) {
                llvm::errs() << "Failed to get file entry\n";
                continue;
            }
            
            // 获取原始文件名
            std::string OriginalFilename = TheRewriter.getSourceMgr().getFilename(
                TheRewriter.getSourceMgr().getLocForStartOfFile(FileID)).str();
            if(OriginalFilename.find("VMPInterface") != std::string::npos) {
                llvm::outs() << "Skipping VMPInterface file: " << OriginalFilename << "\n";
                continue;
            }
            if (OriginalFilename.empty()) {
                llvm::errs() << "Empty filename\n";
                continue;
            }
            RewriteBuf.write(llvm::outs());
            llvm::outs()<<"\n";
            // 生成新文件名，使用哈希值作为后缀
            auto suffix_n = hashString(OriginalFilename);
            std::ostringstream oss;
            oss << std::hex << std::setw(16) << std::setfill('0') << suffix_n;
            
            std::string newFileName = OriginalFilename.substr(0, OriginalFilename.rfind(".")) + 
                                     "_" + oss.str() + OriginalFilename.substr(OriginalFilename.rfind("."));
            
            // 创建输出文件
            std::error_code EC;
            raw_fd_ostream outFile(newFileName, EC, sys::fs::OF_None);
            if (EC) {
                llvm::errs() << "Could not open output file " << newFileName << ": " << EC.message() << "\n";
                continue;
            }
            
            // 写入修改后的内容
            RewriteBuf.write(outFile);
            outFile.close();
            
            llvm::outs() << "Modified source written to: " << newFileName << "\n";
        }
    }
};

// 前端动作，创建 AST 消费者
class FunctionCallRewriterAction : public PluginASTAction {
private:


protected:
    // 创建 AST 消费者
    std::unique_ptr<ASTConsumer> CreateASTConsumer(CompilerInstance &CI, 
                                                 StringRef file) override {
        return std::make_unique<FunctionCallRewriterConsumer>(CI);
    }

    // 插件参数解析
    bool ParseArgs(const CompilerInstance &CI,
                 const std::vector<std::string> &args) override {
        return true;
    }
    
    PluginASTAction::ActionType getActionType() override {
        return AddAfterMainAction;
    }
};

// 注册插件
static FrontendPluginRegistry::Add<FunctionCallRewriterAction>
X("function-call-rewriter", "Rewrites function calls with protect attribute to InitVMPnative");
