#include "common/angleutils.h"
#include "compiler/translator/Cache.h"

namespace
{

class TScopedAllocator : angle::NonCopyable
{
  public:
    TScopedAllocator(TPoolAllocator *allocator)
        : previousAllocator(GetGlobalPoolAllocator())
    {
        SetGlobalPoolAllocator(allocator);
    }
    ~TScopedAllocator()
    {
        SetGlobalPoolAllocator(previousAllocator);
    }

  private:
    TPoolAllocator *previousAllocator;
};

}  // namespace

TCache *TCache::cache = nullptr;

void TCache::initialize()
{
    if (cache == nullptr)
    {
        cache = new TCache();
    }
}

void TCache::destroy()
{
    delete cache;
    cache = nullptr;
}

const TType *TCache::getType(TBasicType basicType,
                             TPrecision precision,
                             TQualifier qualifier,
                             unsigned char primarySize,
                             unsigned char secondarySize)
{
    TypeKey key = {basicType, precision, qualifier,
                   primarySize, secondarySize};
    auto it = cache->types.find(key);
    if (it != cache->types.end())
    {
        return it->second;
    }
    else
    {
        TScopedAllocator scopedAllocator(&cache->allocator);

        TType *type = new TType(basicType, precision, qualifier,
                                primarySize, secondarySize);
        type->realize();
        cache->types.insert({key, type});

        return type;
    }
}
