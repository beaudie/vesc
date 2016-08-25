#include "BreakVariableAliasingInInnerLoops.h"

#include "compiler/translator/IntermNode.h"

//TODO share
TIntermConstantUnion *constructConstUnionNode(const TType &type)
{
    TType myType = type;
    myType.clearArrayness();
    myType.setQualifier(EvqConst);
    size_t size          = myType.getObjectSize();
    TConstantUnion *u = new TConstantUnion[size];
    for (size_t ii = 0; ii < size; ++ii)
    {
        switch (type.getBasicType())
        {
            case EbtFloat:
                u[ii].setFConst(0.0f);
                break;
            case EbtInt:
                u[ii].setIConst(0);
                break;
            case EbtUInt:
                u[ii].setUConst(0u);
                break;
            default:
                UNREACHABLE();
                return nullptr;
        }
    }

    TIntermConstantUnion *node = new TIntermConstantUnion(u, myType);
    return node;
}

class AliasingBreaker : public TIntermTraverser
{
    public:
        AliasingBreaker() : TIntermTraverser(true, false, true)
        {
        }

    protected:
        bool visitBinary(Visit visit, TIntermBinary *binary)
        {
            if (visit != PreVisit)
            {
                return false;
            }

            if (mLoopLevel < 2 || !binary->isAssignment())
            {
                return true;
            }

            TIntermTyped *originalRight = binary->getRight();
            TType type = originalRight->getType();

            if (type.isArray() || type.isStruct())
            {
                return true;
            }

            TIntermBinary *newRight = new TIntermBinary(EOpAdd);
            newRight->setLeft(originalRight);
            newRight->setRight(constructConstUnionNode(type));

            return true;
        }

        bool visitLoop(Visit visit, TIntermLoop *loop)
        {
            if (visit == PreVisit)
            {
                mLoopLevel ++;
            }
            else
            {
                ASSERT(mLoopLevel > 0);
                mLoopLevel --;
            }

            return true;
        }

    private:
        int mLoopLevel = 0;
};

void BreakVariableAliasingInInnerLoops(TIntermNode *root)
{
    AliasingBreaker breaker;
    root->traverse(&breaker);
}
