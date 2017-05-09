//
// Copyright 2017 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// ResourceManager11:
//   Centralized point of allocation for all D3D11 Resources.

#ifndef LIBANGLE_RENDERER_D3D_D3D11_RESOURCEFACTORY11_H_
#define LIBANGLE_RENDERER_D3D_D3D11_RESOURCEFACTORY11_H_

#include <array>
#include <memory>

#include "common/angleutils.h"
#include "common/debug.h"
#include "libANGLE/Error.h"

namespace rx
{
class Renderer11;
class ResourceManager11;
template <typename T>
class SharedResource11;
class TextureHelper11;

enum class ResourceType
{
    DepthStencilView,
    RenderTargetView,
    ShaderResourceView,
    Texture2D,
    Texture3D,
    Last
};

constexpr size_t ResourceTypeIndex(ResourceType resourceType)
{
    return static_cast<size_t>(resourceType);
}

constexpr size_t NumResourceTypes = ResourceTypeIndex(ResourceType::Last);

#define ANGLE_TYPE_HELPER_BEGIN(NAME, T) \
    template <T Param>                   \
    struct NAME;
#define ANGLE_TYPE_HELPER_END(NAME, T) \
    \
template<T Param> struct NAME          \
    {                                  \
    };                                 \
    \
template<T Param> using Get##NAME = typename NAME<Param>::Value;

#define ANGLE_TYPE_HELPER(HELPER, TYPE, ID, VALUE) \
    \
template<> struct HELPER<TYPE::ID>                 \
    {                                              \
        using Value = VALUE;                       \
    };

#define ANGLE_INV_TYPE_HELPER_BEGIN(NAME) \
    template <typename Param>             \
    struct NAME;
#define ANGLE_INV_TYPE_HELPER_END(NAME, T)       \
    \
template<typename Param> struct NAME             \
    {                                            \
    };                                           \
    \
template<typename Param> constexpr T Get##NAME() \
    {                                            \
        return NAME<Param>::Value;               \
    }

#define ANGLE_INV_TYPE_HELPER(HELPER, TYPE, ID, VALUE) \
    \
template<> struct HELPER<ID>                           \
    {                                                  \
        static constexpr TYPE Value = TYPE::VALUE;     \
    };

ANGLE_TYPE_HELPER_BEGIN(D3D11Type, ResourceType)
ANGLE_TYPE_HELPER(D3D11Type, ResourceType, DepthStencilView, ID3D11DepthStencilView)
ANGLE_TYPE_HELPER(D3D11Type, ResourceType, RenderTargetView, ID3D11RenderTargetView)
ANGLE_TYPE_HELPER(D3D11Type, ResourceType, ShaderResourceView, ID3D11ShaderResourceView)
ANGLE_TYPE_HELPER(D3D11Type, ResourceType, Texture2D, ID3D11Texture2D)
ANGLE_TYPE_HELPER(D3D11Type, ResourceType, Texture3D, ID3D11Texture3D)
ANGLE_TYPE_HELPER_END(D3D11Type, ResourceType)

ANGLE_TYPE_HELPER_BEGIN(DescType, ResourceType)
ANGLE_TYPE_HELPER(DescType, ResourceType, DepthStencilView, D3D11_DEPTH_STENCIL_VIEW_DESC)
ANGLE_TYPE_HELPER(DescType, ResourceType, RenderTargetView, D3D11_RENDER_TARGET_VIEW_DESC)
ANGLE_TYPE_HELPER(DescType, ResourceType, ShaderResourceView, D3D11_SHADER_RESOURCE_VIEW_DESC)
ANGLE_TYPE_HELPER(DescType, ResourceType, Texture2D, D3D11_TEXTURE2D_DESC)
ANGLE_TYPE_HELPER(DescType, ResourceType, Texture3D, D3D11_TEXTURE3D_DESC)
ANGLE_TYPE_HELPER_END(DescType, ResourceType)

ANGLE_TYPE_HELPER_BEGIN(InitDataType, ResourceType)
ANGLE_TYPE_HELPER(InitDataType, ResourceType, DepthStencilView, ID3D11Resource)
ANGLE_TYPE_HELPER(InitDataType, ResourceType, RenderTargetView, ID3D11Resource)
ANGLE_TYPE_HELPER(InitDataType, ResourceType, ShaderResourceView, ID3D11Resource)
ANGLE_TYPE_HELPER(InitDataType, ResourceType, Texture2D, const D3D11_SUBRESOURCE_DATA)
ANGLE_TYPE_HELPER(InitDataType, ResourceType, Texture3D, const D3D11_SUBRESOURCE_DATA)
ANGLE_TYPE_HELPER_END(InitDataType, ResourceType)

ANGLE_INV_TYPE_HELPER_BEGIN(ResourceTypeFromD3D11)
ANGLE_INV_TYPE_HELPER(ResourceTypeFromD3D11, ResourceType, ID3D11DepthStencilView, DepthStencilView)
ANGLE_INV_TYPE_HELPER(ResourceTypeFromD3D11, ResourceType, ID3D11RenderTargetView, RenderTargetView)
ANGLE_INV_TYPE_HELPER(ResourceTypeFromD3D11,
                      ResourceType,
                      ID3D11ShaderResourceView,
                      ShaderResourceView)
ANGLE_INV_TYPE_HELPER(ResourceTypeFromD3D11, ResourceType, ID3D11Texture2D, Texture2D)
ANGLE_INV_TYPE_HELPER(ResourceTypeFromD3D11, ResourceType, ID3D11Texture3D, Texture3D)
ANGLE_INV_TYPE_HELPER_END(ResourceTypeFromD3D11, ResourceType)

ANGLE_INV_TYPE_HELPER_BEGIN(ResourceTypeFromDesc)
ANGLE_INV_TYPE_HELPER(ResourceTypeFromDesc,
                      ResourceType,
                      D3D11_DEPTH_STENCIL_VIEW_DESC,
                      DepthStencilView)
ANGLE_INV_TYPE_HELPER(ResourceTypeFromDesc,
                      ResourceType,
                      D3D11_RENDER_TARGET_VIEW_DESC,
                      RenderTargetView)
ANGLE_INV_TYPE_HELPER(ResourceTypeFromDesc,
                      ResourceType,
                      D3D11_SHADER_RESOURCE_VIEW_DESC,
                      ShaderResourceView)
ANGLE_INV_TYPE_HELPER(ResourceTypeFromDesc, ResourceType, D3D11_TEXTURE2D_DESC, Texture2D)
ANGLE_INV_TYPE_HELPER(ResourceTypeFromDesc, ResourceType, D3D11_TEXTURE3D_DESC, Texture3D)
ANGLE_INV_TYPE_HELPER_END(ResourceTypeFromDesc, ResourceType)

template <typename T>
using GetDescFromD3D11 = GetDescType<ResourceTypeFromD3D11<T>::Value>;

template <typename T>
using GetInitDataFromD3D11 = GetInitDataType<ResourceTypeFromD3D11<T>::Value>;

template <typename DescT>
using GetD3D11FromDesc = GetD3D11Type<ResourceTypeFromDesc<DescT>::Value>;

template <typename T>
constexpr size_t ResourceTypeIndex()
{
    return static_cast<size_t>(GetResourceTypeFromD3D11<T>());
}

template <typename T>
struct TypedData
{
    TypedData() {}
    ~TypedData();

    T *object                  = nullptr;
    ResourceManager11 *manager = nullptr;
};

// Smart pointer type. Wraps the resource and a factory for safe deletion.
template <typename T, template <class> class Pointer, typename DataT>
class Resource11Base : angle::NonCopyable
{
  public:
    T *get() const { return mData->object; }

    void setDebugName(const char *name) { d3d11::SetDebugName(mData->object, name); }

    void set(T *object)
    {
        ASSERT(!valid());
        mData->object = object;
    }

    bool valid() const { return (mData->object != nullptr); }

    // Note: this function is not safety-checked.
    template <typename DescT>
    void getDesc(DescT *descOut) const
    {
        return reinterpret_cast<GetD3D11FromDesc<DescT> *>(mData->object)->GetDesc(descOut);
    }

    void reset() { mData.reset(new DataT()); }

  protected:
    friend class TextureHelper11;

    Resource11Base() : mData(new DataT()) {}

    Resource11Base(Resource11Base &&movedObj) : mData(new DataT())
    {
        std::swap(mData, movedObj.mData);
    }

    virtual ~Resource11Base() { mData.reset(); }

    Resource11Base &operator=(Resource11Base &&movedObj)
    {
        std::swap(mData, movedObj.mData);
        return *this;
    }

    Pointer<DataT> mData;
};

template <typename T>
using UniquePtr = typename std::unique_ptr<T, std::default_delete<T>>;

template <typename ResourceT>
class Resource11 : public Resource11Base<ResourceT, UniquePtr, TypedData<ResourceT>>
{
  public:
    Resource11() {}
    Resource11(Resource11 &&other) : Resource11Base(std::move(other)) {}
    Resource11 &operator=(Resource11 &&other)
    {
        std::swap(mData, other.mData);
        return *this;
    }

  private:
    template <typename T>
    friend class SharedResource11;
    friend class ResourceManager11;

    Resource11(ResourceT *object, ResourceManager11 *manager)
    {
        mData->object  = object;
        mData->manager = manager;
    }
};

template <typename T>
class SharedResource11 : public Resource11Base<T, std::shared_ptr, TypedData<T>>
{
  public:
    SharedResource11() {}
    SharedResource11(SharedResource11 &&movedObj) : Resource11Base(std::move(movedObj)) {}

    SharedResource11 &operator=(SharedResource11 &&other)
    {
        std::swap(mData, other.mData);
        return *this;
    }

    SharedResource11(const SharedResource11 &sharedObj) { mData = sharedObj.mData; }

    SharedResource11 &operator=(const SharedResource11 &sharedObj)
    {
        mData = sharedObj.mData;
        return *this;
    }

  private:
    friend class ResourceManager11;
    SharedResource11(Resource11<T> &&obj) : Resource11Base()
    {
        std::swap(mData->manager, obj.mData->manager);

        // Can't use std::swap because of ID3D11Resource.
        auto temp         = mData->object;
        mData->object     = obj.mData->object;
        obj.mData->object = static_cast<T *>(temp);
    }
};

class ResourceManager11 final : angle::NonCopyable
{
  public:
    ResourceManager11();
    ~ResourceManager11();

    template <typename T>
    gl::Error allocate(Renderer11 *renderer,
                       const GetDescFromD3D11<T> *desc,
                       GetInitDataFromD3D11<T> *initData,
                       Resource11<T> *resourceOut);

    template <typename T>
    gl::Error allocate(Renderer11 *renderer,
                       const GetDescFromD3D11<T> *desc,
                       GetInitDataFromD3D11<T> *initData,
                       SharedResource11<T> *sharedRes)
    {
        Resource11<T> res;
        ANGLE_TRY(allocate(renderer, desc, initData, &res));
        *sharedRes = std::move(res);
        return gl::NoError();
    }

    template <typename T>
    void onRelease(T *resource);
    void onReleaseResource(ResourceType resourceType, ID3D11Resource *resource);

  private:
    void incrResource(ResourceType resourceType, size_t memorySize);
    void decrResource(ResourceType resourceType, size_t memorySize);

    std::array<size_t, NumResourceTypes> mAllocatedResourceCounts;
    std::array<size_t, NumResourceTypes> mAllocatedResourceDeviceMemory;
};

template <typename ResourceT>
TypedData<ResourceT>::~TypedData()
{
    if (object)
    {
        // We can have a nullptr factory when holding passed-in resources.
        if (manager)
        {
            manager->onRelease(object);
        }
        object->Release();
    }
}

namespace d3d11
{
using DepthStencilView   = Resource11<ID3D11DepthStencilView>;
using RenderTargetView   = Resource11<ID3D11RenderTargetView>;
using ShaderResourceView = Resource11<ID3D11ShaderResourceView>;
using Texture2D          = Resource11<ID3D11Texture2D>;
using Texture3D          = Resource11<ID3D11Texture3D>;

using SharedSRV = SharedResource11<ID3D11ShaderResourceView>;
}  // namespace d3d11

}  // namespace rx

#endif  // LIBANGLE_RENDERER_D3D_D3D11_RESOURCEFACTORY11_H_
