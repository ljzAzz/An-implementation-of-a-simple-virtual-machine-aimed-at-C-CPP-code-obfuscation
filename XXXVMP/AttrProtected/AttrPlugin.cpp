#include "clang/AST/ASTContext.h"
#include "clang/AST/Attr.h"
#include "clang/Sema/ParsedAttr.h"
#include "clang/Sema/Sema.h"
#include "clang/Sema/SemaDiagnostic.h"
#include "llvm/IR/Attributes.h"
#include "clang/AST/Decl.h"
using namespace clang;
class ProtectAttrInfo : public ParsedAttrInfo {
public:
ProtectAttrInfo() {
    static constexpr Spelling S[] ={{ParsedAttr::AS_GNU,"protect"},
                        {ParsedAttr::AS_CXX11,"protect"}
    };
    Spellings = S;
  }
  AttrHandling handleDeclAttribute(Sema &S, Decl *D,
                                   const ParsedAttr &Attr) const override {
                                    if(isa<FunctionDecl>(D)){

                                            D->addAttr(AnnotateAttr::Create(S.Context, "protect", nullptr, 0,
                                                Attr.getRange()));
                                            return AttributeApplied;
                                        
                                    }
    return AttributeApplied;
  }
};
    
static clang::ParsedAttrInfoRegistry::Add<ProtectAttrInfo> Z("protect_attr","protect attribute description");