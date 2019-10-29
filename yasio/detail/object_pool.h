//////////////////////////////////////////////////////////////////////////////////////////
// A cross platform socket APIs, support ios & android & wp8 & window store
// universal app
//////////////////////////////////////////////////////////////////////////////////////////
/*
The MIT License (MIT)

Copyright (c) 2012-2019 halx99

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/
// object_pool.h: a simple & high-performance object pool implementation v1.3
#ifndef YASIO__OBJECT_POOL_H
#define YASIO__OBJECT_POOL_H

#include <assert.h>
#include <stdlib.h>
#include <memory>
#include <mutex>

#define YASIO_OBJECT_POOL_HEADER_ONLY

#if defined(YASIO_OBJECT_POOL_HEADER_ONLY)
#  define OBJECT_POOL_DECL inline
#else
#  define OBJECT_POOL_DECL
#endif

#if defined(_MSC_VER)
#  pragma warning(push)
#  pragma warning(disable : 4200)
#endif

namespace yasio
{
namespace gc
{
#define YASIO_SZ_ALIGN(d, a) (((d) + ((a)-1)) & ~((a)-1))
#define YASIO_POOL_ESTIMATE_SIZE(element_type) YASIO_SZ_ALIGN(sizeof(element_type), sizeof(void*))

namespace detail
{
class object_pool
{
  typedef struct free_link_node
  {
    free_link_node* next;
  } * free_link;

  typedef struct chunk_link_node
  {
    chunk_link_node* next;
    char data[0];
  } * chunk_link;

  object_pool(const object_pool&) = delete;
  void operator=(const object_pool&) = delete;

public:
  OBJECT_POOL_DECL object_pool(size_t element_size, size_t element_count);

  OBJECT_POOL_DECL virtual ~object_pool(void);

  OBJECT_POOL_DECL void purge(void);

  OBJECT_POOL_DECL void cleanup(void);

  OBJECT_POOL_DECL void* get(void);
  OBJECT_POOL_DECL void release(void* _Ptr);

private:
  OBJECT_POOL_DECL void* allocate_from_chunk(void);
  OBJECT_POOL_DECL void* allocate_from_process_heap(void);

  OBJECT_POOL_DECL free_link_node* tidy_chunk(chunk_link chunk);

private:
  free_link free_link_; // link to free head
  chunk_link chunk_;    // chunk link
  const size_t element_size_;
  const size_t element_count_;

#if defined(_DEBUG)
  size_t allocated_count_; // allocated count
#endif
};

#define DEFINE_OBJECT_POOL_ALLOCATION(ELEMENT_TYPE, ELEMENT_COUNT)                                 \
public:                                                                                            \
  static void* operator new(size_t /*size*/) { return get_pool().get(); }                          \
                                                                                                   \
  static void* operator new(size_t /*size*/, std::nothrow_t) { return get_pool().get(); }          \
                                                                                                   \
  static void operator delete(void* p) { get_pool().release(p); }                                  \
                                                                                                   \
  static yasio::gc::detail::object_pool& get_pool()                                                \
  {                                                                                                \
    static yasio::gc::detail::object_pool s_pool(YASIO_POOL_ESTIMATE_SIZE(ELEMENT_TYPE),           \
                                                 ELEMENT_COUNT);                                   \
    return s_pool;                                                                                 \
  }

// The thread safe edition
#define DEFINE_CONCURRENT_OBJECT_POOL_ALLOCATION(ELEMENT_TYPE, ELEMENT_COUNT)                      \
public:                                                                                            \
  static void* operator new(size_t /*size*/) { return get_pool().allocate(); }                     \
                                                                                                   \
  static void* operator new(size_t /*size*/, std::nothrow_t) { return get_pool().allocate(); }     \
                                                                                                   \
  static void operator delete(void* p) { get_pool().deallocate(p); }                               \
                                                                                                   \
  static yasio::gc::object_pool<ELEMENT_TYPE, std::mutex>& get_pool()                              \
  {                                                                                                \
    static yasio::gc::object_pool<ELEMENT_TYPE, std::mutex> s_pool(ELEMENT_COUNT);                 \
    return s_pool;                                                                                 \
  }

#define DECLARE_OBJECT_POOL_ALLOCATION(ELEMENT_TYPE)                                               \
public:                                                                                            \
  static void* operator new(size_t /*size*/);                                                      \
  static void* operator new(size_t /*size*/, std::nothrow_t);                                      \
  static void operator delete(void* p);                                                            \
  static yasio::gc::detail::object_pool& get_pool();

#define IMPLEMENT_OBJECT_POOL_ALLOCATION(ELEMENT_TYPE, ELEMENT_COUNT)                              \
  void* ELEMENT_TYPE::operator new(size_t /*size*/) { return get_pool().get(); }                   \
                                                                                                   \
  void* ELEMENT_TYPE::operator new(size_t /*size*/, std::nothrow_t) { return get_pool().get(); }   \
                                                                                                   \
  void ELEMENT_TYPE::operator delete(void* p) { get_pool().release(p); }                           \
                                                                                                   \
  yasio::gc::detail::object_pool& ELEMENT_TYPE::get_pool()                                         \
  {                                                                                                \
    static yasio::gc::detail::object_pool s_pool(YASIO_POOL_ESTIMATE_SIZE(ELEMENT_TYPE),           \
                                                 ELEMENT_COUNT);                                   \
    return s_pool;                                                                                 \
  }
} // namespace detail

template <typename _Ty, typename _Mutex = void> class object_pool : public detail::object_pool
{
  object_pool(const object_pool&) = delete;
  void operator=(const object_pool&) = delete;

public:
  object_pool(size_t _ElemCount = 512)
      : detail::object_pool(YASIO_POOL_ESTIMATE_SIZE(_Ty), _ElemCount)
  {}

  template <typename... _Args> _Ty* construct(const _Args&... args)
  {
    return new (allocate()) _Ty(args...);
  }

  void destroy(void* _Ptr)
  {
    ((_Ty*)_Ptr)->~_Ty(); // call the destructor
    release(_Ptr);
  }

  void* allocate() { return get(); }

  void deallocate(void* _Ptr) { release(_Ptr); }
};

template <typename _Ty> class object_pool<_Ty, std::mutex> : public detail::object_pool
{
public:
  object_pool(size_t _ElemCount = 512)
      : detail::object_pool(YASIO_POOL_ESTIMATE_SIZE(_Ty), _ElemCount)
  {}

  template <typename... _Args> _Ty* construct(const _Args&... args)
  {
    return new (allocate()) _Ty(args...);
  }

  void destroy(void* _Ptr)
  {
    ((_Ty*)_Ptr)->~_Ty(); // call the destructor
    release(_Ptr);
  }

  void* allocate()
  {
    std::lock_guard<std::mutex> lk(this->mutex_);
    return get();
  }

  void deallocate(void* _Ptr)
  {
    std::lock_guard<std::mutex> lk(this->mutex_);
    release(_Ptr);
  }

  std::mutex mutex_;
};

//////////////////////// allocator /////////////////
// TEMPLATE CLASS object_pool_allocator, can't used by std::vector, DO NOT use at non-msvc compiler.
template <class _Ty, size_t _ElemCount = 8192 / sizeof(_Ty)> class object_pool_allocator
{ // generic allocator for objects of class _Ty
public:
  typedef _Ty value_type;

  typedef value_type* pointer;
  typedef value_type& reference;
  typedef const value_type* const_pointer;
  typedef const value_type& const_reference;

  typedef size_t size_type;
#ifdef _WIN32
  typedef ptrdiff_t difference_type;
#else
  typedef long difference_type;
#endif

  template <class _Other> struct rebind
  { // convert this type to _ALLOCATOR<_Other>
    typedef object_pool_allocator<_Other> other;
  };

  pointer address(reference _Val) const
  { // return address of mutable _Val
    return ((pointer) & (char&)_Val);
  }

  const_pointer address(const_reference _Val) const
  { // return address of nonmutable _Val
    return ((const_pointer) & (const char&)_Val);
  }

  object_pool_allocator() throw()
  { // construct default allocator (do nothing)
  }

  object_pool_allocator(const object_pool_allocator<_Ty>&) throw()
  { // construct by copying (do nothing)
  }

  template <class _Other> object_pool_allocator(const object_pool_allocator<_Other>&) throw()
  { // construct from a related allocator (do nothing)
  }

  template <class _Other>
  object_pool_allocator<_Ty>& operator=(const object_pool_allocator<_Other>&)
  { // assign from a related allocator (do nothing)
    return (*this);
  }

  void deallocate(pointer _Ptr, size_type)
  { // deallocate object at _Ptr, ignore size
    _Mempool.release(_Ptr);
  }

  pointer allocate(size_type count)
  { // allocate array of _Count elements
    assert(count == 1);
    (void)count;
    return static_cast<pointer>(_Mempool.get());
  }

  pointer allocate(size_type count, const void*)
  { // allocate array of _Count elements, not support, such as std::vector
    return allocate(count);
  }

  void construct(_Ty* _Ptr)
  { // default construct object at _Ptr
    ::new ((void*)_Ptr) _Ty();
  }

  void construct(pointer _Ptr, const _Ty& _Val)
  { // construct object at _Ptr with value _Val
    new (_Ptr) _Ty(_Val);
  }

  void construct(pointer _Ptr, _Ty&& _Val)
  { // construct object at _Ptr with value _Val
    new ((void*)_Ptr) _Ty(std::forward<_Ty>(_Val));
  }

  template <class _Other> void construct(pointer _Ptr, _Other&& _Val)
  { // construct object at _Ptr with value _Val
    new ((void*)_Ptr) _Ty(std::forward<_Other>(_Val));
  }

  template <class _Objty, class... _Types> void construct(_Objty* _Ptr, _Types&&... _Args)
  { // construct _Objty(_Types...) at _Ptr
    ::new ((void*)_Ptr) _Objty(std::forward<_Types>(_Args)...);
  }

  template <class _Uty> void destroy(_Uty* _Ptr)
  { // destroy object at _Ptr, do nothing
    _Ptr->~_Uty();
  }

  size_type max_size() const throw()
  { // estimate maximum array size
    size_type _Count = (size_type)(-1) / sizeof(_Ty);
    return (0 < _Count ? _Count : 1);
  }

  // private:
  static object_pool<_Ty, void> _Mempool;
};

template <class _Ty, class _Other>
inline bool operator==(const object_pool_allocator<_Ty>&,
                       const object_pool_allocator<_Other>&) throw()
{ // test for allocator equality
  return (true);
}

template <class _Ty, class _Other>
inline bool operator!=(const object_pool_allocator<_Ty>& _Left,
                       const object_pool_allocator<_Other>& _Right) throw()
{ // test for allocator inequality
  return (!(_Left == _Right));
}

template <class _Ty, size_t _ElemCount>
object_pool<_Ty, void> object_pool_allocator<_Ty, _ElemCount>::_Mempool(_ElemCount);

} // namespace gc
} // namespace yasio

#if defined(YASIO_OBJECT_POOL_HEADER_ONLY)
#  include "object_pool.cpp"
#endif

#if defined(_MSC_VER)
#  pragma warning(pop)
#endif

#endif
