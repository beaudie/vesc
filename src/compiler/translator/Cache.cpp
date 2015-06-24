#include "common/angleutils.h"
#include "compiler/translator/Cache.h"

namespace
{

class TScopedAllocator : angle::NonCopyable
{
  public:
    TScopedAllocator(TPoolAllocator* allocator)
        : previousAllocator(GetGlobalPoolAllocator())
    {
        SetGlobalPoolAllocator(allocator);
    }
    ~TScopedAllocator()
    {
        SetGlobalPoolAllocator(previousAllocator);
    }

  private:
    TPoolAllocator* previousAllocator;
};

}  // namespace

size_t TCache::TypeKeyHash::operator () (const TypeKey &key) const
{
    static_assert(std::tuple_size<TypeKey>::value == 5,
                  "not all TypeKey members are hashed");
    return  std::hash<int>()(std::get<0>(key)) ^
            std::hash<int>()(std::get<1>(key)) ^
            std::hash<int>()(std::get<2>(key)) ^
            std::hash<int>()(std::get<3>(key)) ^
            std::hash<int>()(std::get<4>(key));
}

TCache* TCache::cache = nullptr;

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
    TypeKey key = std::make_tuple(basicType, precision, qualifier,
                                  primarySize, secondarySize);
    auto it = cache->types.find(key);
    if (it != cache->types.end())
    {
        return it->second;
    }
    else
    {
        TScopedAllocator scopedAllocator(&cache->allocator);

        TType* type = new TType(basicType, precision, qualifier,
                                primarySize, secondarySize);
        type->realize();
        cache->types.insert({key, type});

        return type;
    }
}
