
#pragma once

#include <algorithm>
#include <stdexcept>

#include <vector>
#include <cstddef>
#include <cassert>

// The array storage
template<typename T>
class NDArray
{
public:

  // Max size in any one dimension of ~1e9
  static const int64_t MaxSize = 1 << 30;

  typedef T value_type;

  typedef const T& const_reference;

  typedef T& reference;

//   // RO iterator over one dimension (O) of an n-D array given an index
//   class ConstIterator
//   {
//   public:

//     ConstIterator(const NDArray<T>& a, int64_t orient, const std::vector<int64_t>& idx) : m_a(a), m_orient(orient), m_idx(idx)
//     {
//       // set index of orientation dimension to zero
//       m_idx[orient] = 0;
//     }

//     ~ConstIterator() { }

//     const std::vector<int64_t>& idx() const
//     {
//       return m_idx;
//     }

//     ConstIterator& operator++()
//     {
//       ++m_idx[m_orient];
//       return *this;
//     }

//     bool end() const
//     {
//       return (size_t)m_idx[m_orient] >= m_a.size(m_orient);
//     }

//     const_reference operator*() const
//     {
//       return m_a[m_idx];
//     }

//   private:
//     const NDArray& m_a;
//     int64_t m_orient;
//     std::vector<int64_t> m_idx;
//   };

//   // RW iterator over one dimension (O) of an n-D array given an index
//   template<size_t O>
//   class Iterator
//   {
//   public:

//     static const size_t Orient = O;

//     Iterator(NDArray<D, T>& a, size_t* idx) : m_a(a)
//     {
//       // copy indices
//       std::copy(idx, idx + D, m_idx);
//       // set index of orientation dimension to zero
//       m_idx[Orient] = 0;
//     }

//     ~Iterator() { }

//     const size_t* idx() const
//     {
//       return m_idx;
//     }

//     Iterator& operator++()
//     {
//       ++m_idx[Orient];
//       return *this;
//     }

//     bool end() const
//     {
//       return m_idx[Orient] >= m_a.size(Orient);
//     }

//     const_reference operator*() const
//     {
//       return m_a[m_idx];
//     }

//     reference operator*()
//     {
//       return m_a[m_idx];
//     }

//   private:
//     NDArray& m_a;
//     size_t m_idx[D];
//   };

  NDArray() : m_dim(0), m_sizes(), m_storageSize(0), m_data(0), m_owned(true)
  {
  }

  explicit NDArray(const std::vector<int64_t>& sizes) : m_dim(sizes.size()), m_sizes(sizes), m_storageSize(0), m_data(0), m_owned(true)
  {
    resize(sizes);
  }

  // Construct with storage managed by some other object
  NDArray(const std::vector<int64_t>& sizes, T* const storage)
    : m_dim(sizes.size()), m_sizes(sizes)
  {
    assert(m_sizes.size());
    m_storageSize = sizes[0];
    assert(m_storageSize < MaxSize);
    for (size_t i = 1; i < m_dim; ++i)
    {
      assert(sizes[i] < MaxSize);
      m_storageSize *= sizes[i];
    }
    computeOffsets();
    
    m_data = storage;
    m_owned = false;
  }

  // Disallow copy
  NDArray(const NDArray&) = delete;
  NDArray& operator=(const NDArray&) = delete;

  // Copying is strongly discouraged for efficiency reasons, however there will always be times when a copy is unavoidable...
  // By explictly providing a copy function we avoid sloppy/inefficient coding where implicit copies are (inadvertently) taken
  static void copy(const NDArray<T>& src, NDArray<T>& dest) 
  {
    dest.resize(src.m_sizes);
    std::copy(src.m_data, src.m_data + src.m_storageSize, dest.m_data);
  }

  // But allow move
  NDArray(NDArray&& a)
  {
    m_dim = a.m_dim;
    m_sizes.swap(a.m_sizes);
    m_offsets.swap(a.m_offsets);
    m_storageSize = a.m_storageSize;
    m_data = a.m_data;
    m_owned = a.m_owned;
    a.m_owned = false;
  }

  ~NDArray()
  {
    if (m_owned)
      deallocate(m_data);
  }

  size_t dim() const
  {
    return m_dim;
  }

  size_t size(size_t dim) const
  {
    assert(dim < m_dim);
    return m_sizes[dim];
  }

  const std::vector<int64_t>& sizes() const
  {
    return m_sizes;
  }

  size_t storageSize() const
  {
    return m_storageSize;
  }

  const T* rawData() const
  {
    return m_data;
  }

  void resize(const std::vector<int64_t>& sizes)
  {
    if (m_owned)
    {
      m_dim = sizes.size();
      size_t oldStorageSize = m_storageSize;

      m_sizes = sizes;
      m_storageSize = sizes[0];
      assert(m_storageSize < MaxSize);
      for (size_t i = 1; i < m_dim; ++i)
      {
        assert(sizes[i] < MaxSize);
        m_storageSize *= sizes[i];
      }

      // no realloc if storageSize already big enough
      if (m_storageSize > oldStorageSize)
      {
        deallocate(m_data);
        m_data = allocate(m_storageSize);
      }
      computeOffsets();
    }
    else
    {
      throw std::runtime_error("resizing not permitted when memory is not owned");
    }
  }

  void assign(T val) const
  {
    std::fill(m_data, m_data + m_storageSize, val);
  }

  reference operator[](const std::vector<int64_t>& index)
  {
    return m_data[offset(index)];
  }

  const_reference operator[](const std::vector<int64_t>& index) const
  {
    return m_data[offset(index)];
  }

  reference operator[](const std::vector<int64_t*>& index)
  {
    return m_data[offset(index)];
  }

  const_reference operator[](const std::vector<int64_t*>& index) const
  {
    return m_data[offset(index)];
  }

  value_type* begin() const
  {
    return m_data;
  }

  value_type* end() const
  {
    return m_data + m_storageSize;
  }

  // relinqish ownership (caller must delete)
  void release()
  {
    m_owned = false;
  }

private:

  void computeOffsets()
  {
    m_offsets.resize(m_dim);
    size_t mult = m_storageSize;
    for (size_t i = 0; i < m_dim; ++i)
    {
      mult /= m_sizes[i];
      m_offsets[i] = mult;
    }
  }

  size_t offset(const std::vector<int64_t>& idx) const
  {
    // TODO this is pretty horrible, but it works.
    size_t ret = 0;
    for (size_t i = 0; i < m_dim; ++i)
    {
      ret += m_offsets[i] * idx[i];
    }
    return ret;
  }

  size_t offset(const std::vector<int64_t*>& idx) const
  {
    // TODO this is pretty horrible, but it works.
    size_t ret = 0;
    for (size_t i = 0; i < m_dim; ++i)
    {
      ret += m_offsets[i] * *idx[i];
    }
    return ret;
  }

  T* allocate(size_t size) const
  {
    return new T[size];
  }

  void deallocate(T* p) const
  {
    delete [] p;
  }

private:

  size_t m_dim;
  std::vector<int64_t> m_sizes;
  std::vector<int64_t> m_offsets;
  size_t m_storageSize;
  T* m_data;
  bool m_owned;
};

