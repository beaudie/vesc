#ifndef COMPILER_TRANSLATOR_CACHE_H_
#define COMPILER_TRANSLATOR_CACHE_H_

#include <tuple>
#include <unordered_map>

#include "compiler/translator/Types.h"
#include "compiler/translator/PoolAlloc.h"

class TCache {
  public:

    static void initialize();
    static void destroy();

    static const TType *getType(TBasicType basicType,
                                TPrecision precision)
    {
        return getType(basicType, precision, EvqTemporary,
                       1, 1);
    }
    static const TType *getType(TBasicType basicType,
                                unsigned char primarySize = 1,
                                unsigned char secondarySize = 1)
    {
        return getType(basicType, EbpUndefined, EvqGlobal,
                       primarySize, secondarySize);
    }
    static const TType *getType(TBasicType basicType,
                                TQualifier qualifier,
                                unsigned char primarySize = 1,
                                unsigned char secondarySize = 1)
    {
        return getType(basicType, EbpUndefined, qualifier,
                       primarySize, secondarySize);
    }
    static const TType *getType(TBasicType basicType,
                                TPrecision precision,
                                TQualifier qualifier,
                                unsigned char primarySize,
                                unsigned char secondarySize);

  private:
    TCache()
    {
    }

    typedef std::tuple<TBasicType,
                       TPrecision,
                       TQualifier,
                       unsigned char,
                       unsigned char> TypeKey;
    struct TypeKeyHash {
        size_t operator () (const TypeKey &key) const;
    };
    typedef std::unordered_map<TypeKey, const TType*, TypeKeyHash> TypeMap;

    TypeMap types;
    TPoolAllocator allocator;

    static TCache* cache;
};

#endif  // COMPILER_TRANSLATOR_CACHE_H_
