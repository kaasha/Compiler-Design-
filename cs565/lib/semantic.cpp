//Semantic Analysis Pass
#include "llvm/ADT/Statistic.h"
#include "llvm/IR/Module.h"

#include "llvm/Analysis/CallGraphSCCPass.h"
#include "llvm/Pass.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/IR/Instruction.h"
#include "llvm/IR/Argument.h"
#include "llvm/IR/Value.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Attributes.h"
#include "llvm/IR/DebugInfo.h"
#include "llvm/IR/LegacyPassManager.h"

using namespace llvm;

#define DEBUG_TYPE "semantic"
//this function maps type id to type name
const char* getTypeVal(int tid, int f)
{
    if (!f) {
        if (tid == 0) {
            return "void";
        }
        else if (tid == 1) {
            return "float";
        }
        else if (tid == 2) {
            return "float";
        }
        else if (tid == 3) {
            return "double";
        }
        else if (tid == 10) {
            return "int";
        }
        else if (tid == 11) {
            return "function";
        }
        else if (tid == 12) {
            return "struct";
        }
        else if (tid == 13) {
            return "array";
        }
        else if (tid == 14) {
            return "pointer";
        }
        else if (tid == 15) {
            return "vector";
        }
    }
    else {
        if (tid == 0) {
            return "void *";
        }
        else if (tid == 1) {
            return "float *";
        }
        else if (tid == 2) {
            return "float *";
        }
        else if (tid == 3) {
            return "double *";
        }
        else if (tid == 10) {
            return "int *";
        }
        else if (tid == 11) {
            return "function *";
        }
        else if (tid == 12) {
            return "struct *";
        }
        else if (tid == 13) {
            return "array *";
        }
        else if (tid == 14) {
            return "pointer *";
        }
        else if (tid == 15) {
            return "vector *";
        }
    }
    return "unknown type";
}

namespace {
//for getting the name and type of each function and matching them
struct Semantic : public FunctionPass {
    static char ID;
    Semantic()
        : FunctionPass(ID)
    {
    }

    virtual bool runOnFunction(Function& F)
    {
        for (Function::iterator bb = F.begin(), e = F.end(); bb != e; ++bb) {
            for (BasicBlock::iterator i = bb->begin(), e = bb->end(); i != e; ++i) {
                CallInst* callInst = dyn_cast<CallInst>(&*i);

                //int line = i->getDebugLoc().getLine();
                //errs() << "he line number is : " << line;

                if (isa<CallInst>(&*i)) {

                    MDNode* N = callInst->getMetadata("dbg");
                    DILocation Loc(N); // DILocation is in DebugInfo.h
                    unsigned int Line = Loc.getLineNumber();
                    //check type and argument
                    Value* v = callInst->getCalledValue();
                    Function* fn = dyn_cast<Function>(v->stripPointerCasts());

                    if (!(fn->isDeclaration())) { //to ignore type checking for functions with variable number of arguments and for functions that are called with debug mode
                        const char* fname = fn->getName().data();
                        //this gives the num of params to be received, which is their in declaration
                        size_t expectedParams = fn->getFunctionType()->getNumParams();
                        //this gives the num of arguments that are sent in the call instruction, which is their in invocation
                        size_t sentArg = callInst->getNumArgOperands();
                        //checking
                        //errs() << "sent args : " << sentArg << " expectedParams : "<<expectedParams << "\n";
                        if (sentArg != expectedParams) {
                            //MISMATCH Number ARGUMENTS
                            errs() << "error: Function " << fname << " call on line " << Line << ": expected " << expectedParams << " arguments but " << sentArg << " are/is present.\n";
                        }
                        else {
                            //MATCH Number ARGUMENTS
                            //support for float, int, double, char, void, pointer, function, struct, vector

                            for (size_t i = 0; i < sentArg; i++) {
                                size_t paramType = fn->getFunctionType()->getParamType(i)->getTypeID();
                                size_t argType = callInst->getArgOperand(i)->getType()->getTypeID();
                                if (paramType != argType) {
                                    //errs() << "sent args type: " << callInst->getArgOperand(i)->getType()->isIntegerTy(32) << " expectedParams type : "<<fn->getFunctionType()->getParamType(i)->isIntegerTy(32) << "\n";
                                    //errs() << "sent args typeid: " << argType << " expectedParams typeid : "<<paramType << "\n";
                                    const char *ptype, *atype;
                                    //std:string a;
                                    ptype = getTypeVal(paramType, 0);
                                    atype = getTypeVal(argType, 0);
                                    int f = 0;
                                    if (paramType == 14 || argType == 14) {
                                        PointerType* pt = dyn_cast<PointerType>(fn->getFunctionType()->getParamType(i));
                                        if (pt != NULL) {
                                            paramType = pt->getElementType()->getTypeID();
                                            if (paramType == 12) {
                                                ptype = (pt->getElementType()->getStructName()).data();
                                            }
                                            else {
                                                if (pt->getElementType()->isIntegerTy(8)) {
                                                    ptype = "char *(pointer)";
                                                    f = 1;
                                                }
                                                else {
                                                    ptype = getTypeVal(paramType, 1);
                                                    f = 1;
                                                }
                                            }
                                        }
                                        PointerType* pt1 = dyn_cast<PointerType>(callInst->getArgOperand(i)->getType());
                                        if (pt1 != NULL) {
                                            argType = pt1->getElementType()->getTypeID();
                                            if (argType == 12) {
                                                atype = (pt1->getElementType()->getStructName()).data();
                                            }
                                            else {
                                                if (pt1->getElementType()->isIntegerTy(8)) {
                                                    atype = "char *(pointer)";
                                                    f = 1;
                                                }
                                                else
                                                    atype = getTypeVal(argType, 1);
                                                f = 1;
                                            }
                                        }
                                    }

                                    if (fn->getFunctionType()->getParamType(i)->isIntegerTy(8)) {
                                        ptype = "char";
                                    }
                                    if (callInst->getArgOperand(i)->getType()->isIntegerTy(8)) {
                                        atype = "char";
                                    }
                                    if ((paramType == 1 || paramType == 2 || paramType == 3 || paramType == 10) && (argType == 1 || argType == 2 || argType == 3 || argType == 10) && !f) {
                                        //warning
                                        errs() << "warning: Function " << fname << " on line " << Line << ":argument type mismatch. Expected " << ptype << " but arguments is of type " << atype << "\n";
                                    }
                                    else {
                                        //error
                                        errs() << "error: Function " << fname << " on line " << Line << ":argument type mismatch. Expected " << ptype << " but arguments is of type " << atype << "\n";
                                    }
                                }
                                else {
                                    if (paramType == 10 && argType == 10) {
                                        if (fn->getFunctionType()->getParamType(i)->isIntegerTy(8) && callInst->getArgOperand(i)->getType()->isIntegerTy(32)) {
                                            errs() << "warning: Function " << fname << " on line " << Line << ":argument type mismatch. Expected char but arguments is of type int\n";
                                        }
                                        else if (fn->getFunctionType()->getParamType(i)->isIntegerTy(32) && callInst->getArgOperand(i)->getType()->isIntegerTy(8)) {
                                            errs() << "warning: Function " << fname << " on line " << Line << ":argument type mismatch. Expected int but arguments is of type char\n";
                                        }
                                    }
                                    if (paramType == 14 && argType == 14) {
                                        const char *ptype, *atype;
                                        PointerType* pt = dyn_cast<PointerType>(fn->getFunctionType()->getParamType(i));
                                        if (pt != NULL) {
                                            paramType = pt->getElementType()->getTypeID();
                                            if (paramType == 12) {
                                                ptype = (pt->getElementType()->getStructName()).data();
                                            }
                                            else {
                                                if (pt->getElementType()->isIntegerTy(8))
                                                    ptype = "char *";
                                                else
                                                    ptype = getTypeVal(paramType, 1);
                                            }
                                        }
                                        PointerType* pt1 = dyn_cast<PointerType>(callInst->getArgOperand(i)->getType());
                                        if (pt1 != NULL) {
                                            argType = pt1->getElementType()->getTypeID();
                                            if (argType == 12) {
                                                atype = (pt1->getElementType()->getStructName()).data();
                                            }
                                            else {
                                                if (pt1->getElementType()->isIntegerTy(8))
                                                    atype = "char *";
                                                else
                                                    atype = getTypeVal(argType, 1);
                                            }
                                        }
                                        // errs() << "ptype is : " << atype << argType;
                                        if (paramType != argType) {

                                            if ((paramType == 1 || paramType == 2 || paramType == 3 || paramType == 10) && (argType == 1 || argType == 2 || argType == 3 || argType == 10)) {
                                                errs() << "warning: Function " << fname << " on line " << Line << ":argument type mismatch. Expected " << ptype << " but arguments is of type " << atype << "\n";
                                            }
                                            else
                                                errs() << "error: Function " << fname << " on line " << Line << ":argument type mismatch. Expected " << ptype << " but arguments is of type " << atype << "\n";
                                        }
                                        else {
                                            //errs() << "equal : ";
                                            if (paramType == 12 && (ptype != atype)) {
                                                errs() << "error: Function " << fname << " on line " << Line << ":argument type mismatch. Expected " << ptype << " but arguments is of type " << atype << "\n";
                                            }
                                            if (pt1->getElementType()->isIntegerTy(8) && pt->getElementType()->isIntegerTy(32)) {
                                                errs() << "warning: Function " << fname << " on line " << Line << ":argument type mismatch. Expected int * but arguments is of type char *\n";
                                            }
                                            else if (pt->getElementType()->isIntegerTy(8) && pt1->getElementType()->isIntegerTy(32)) {
                                                errs() << "warning: Function " << fname << " on line " << Line << ":argument type mismatch. Expected char * but arguments is of type int *\n";
                                            }
                                        }
                                    }

                                    //no problem
                                }
                            }
                        }
                    }
                    else {
                        //errs() << " function with variable number of args\n";
                    }
                }
                else { //errs() << "   Not A function/call instruction\n";
                }
            }
        }
        return false;
    }
};
}

char Semantic::ID = 0;
static RegisterPass<Semantic> X("proj1", "Semantic Analysis Pass");

namespace {
//for getting call invocations
struct Semantic1b : public CallGraphSCCPass {
    static char ID;
    int desc = 0;
    std::map<std::string, int> funcCount;
    Semantic1b()
        : CallGraphSCCPass(ID)
    {
    }

    virtual bool runOnSCC(CallGraphSCC& cgscc)
    {
        if (!desc) {
            errs() << "List of function calls:"
                   << "\n";
            desc = 1;
        }
        for (CallGraphSCC::iterator it = cgscc.begin(); it != cgscc.end(); it++) {

            CallGraphNode* node = *it;
            if (node->getFunction()) {
                if (strncmp((node->getFunction()->getName()).data(), "llvm.", strlen("llvm."))) {
                    errs() << node->getFunction()->getName();
                    errs() << "(";

                    int numArgs = node->getFunction()->getArgumentList().size();
                    for (Function::ArgumentListType::iterator bb = node->getFunction()->getArgumentList().begin(), e = node->getFunction()->getArgumentList().end(); bb != e; ++bb) {
                        numArgs--;
                        size_t typeId = bb->getType()->getTypeID();
                        const char* printType = getTypeVal(typeId, 0);

                        if (typeId == 14 || typeId == 14) {
                            PointerType* pt = dyn_cast<PointerType>(bb->getType());
                            if (pt != NULL) {
                                typeId = pt->getElementType()->getTypeID();
                                if (typeId == 12) {
                                    printType = "struct";
                                }
                                else {
                                    if (pt->getElementType()->isIntegerTy(8))
                                        printType = "char *";
                                    else
                                        printType = getTypeVal(typeId, 1);
                                }
                            }
                        }
                        if (bb->getType()->isIntegerTy(8)) {
                            printType = "char";
                        }

                        errs() << printType;
                        if (numArgs)
                            errs() << ",";
                    }

                    errs() << ") : ";
                    errs() << node->getNumReferences() - 1;
                    errs() << "\n";
                }
            }
        }
        return false;
    }
};
}

char Semantic1b::ID = 0;
static RegisterPass<Semantic1b> Y("proj1b", "semantic2 Analysis Pass");
