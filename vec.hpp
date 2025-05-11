#pragma once

#include "header.hpp"

#define DEFAULT_VEC_CAP ((size_t)64)

namespace sf
{
template <typename T> class Vec
{
private:
  T *vals;
  size_t size;
  size_t cap;

public:
  Vec ();
  Vec (size_t);
  Vec (Vec &);
  Vec (Vec &&);
  Vec (const Vec &);
  Vec (std::initializer_list<T>);

  struct Iterator
  {
    using iterator_category = std::forward_iterator_tag;
    using difference_type = std::ptrdiff_t;
    using value_type = T;
    using pointer = value_type *;
    using reference = value_type &;

    Iterator (pointer ptr) : m_ptr (ptr) {}

    reference
    operator* () const
    {
      return *m_ptr;
    }

    pointer
    operator->()
    {
      return m_ptr;
    }

    Iterator &
    operator++ ()
    {
      m_ptr++;
      return *this;
    }

    Iterator
    operator++ (int)
    {
      Iterator tmp = *this;
      ++(*this);
      return tmp;
    }

    friend bool
    operator== (const Iterator &a, const Iterator &b)
    {
      return a.m_ptr == b.m_ptr;
    }

    friend bool
    operator!= (const Iterator &a, const Iterator &b)
    {
      return a.m_ptr != b.m_ptr;
    }

  private:
    pointer m_ptr;
  };

  Vec &operator= (const Vec &rhs);
  Vec &operator= (Vec &&rhs) noexcept;

  inline size_t
  get_size () const
  {
    return size;
  }

  inline size_t
  get_cap () const
  {
    return cap;
  }

  inline T *&
  get ()
  {
    return vals;
  }

  inline const T *
  get () const
  {
    return vals;
  }

  inline T &
  back ()
  {
    if (!size)
      throw "vec_empty_array";

    return vals[size - 1];
  }

  inline T
  back () const
  {
    if (!size)
      throw "vec_empty_array";

    return vals[size - 1];
  }

  inline T &
  front ()
  {
    if (!size)
      throw "vec_empty_array";

    return vals[0];
  }

  inline T
  front () const
  {
    if (!size)
      throw "vec_empty_array";

    return vals[0];
  }

  void push_back (T &);
  void push_back (T &&);
  T &pop_back ();

  T &
  operator[] (size_t i)
  {
    if (i >= size)
      throw "vec_index_out_of_bounds";

    return vals[i];
  }

  const T
  operator[] (size_t i) const
  {
    if (i >= size)
      throw "vec_index_out_of_bounds";

    return vals[i];
  }

  void resize (size_t);
  void insert (size_t, T &);
  void insert (size_t, T &&);

  Iterator
  begin ()
  {
    return Iterator (&vals[0]);
  }

  Iterator
  end ()
  {
    return Iterator (&vals[size]);
  }

  void
  clear ()
  {
    size = 0;
  };

  void reverse ();

  ~Vec ();
};

} // namespace sf

namespace sf
{
template <typename T> Vec<T>::Vec ()
{
  size = 0;
  cap = DEFAULT_VEC_CAP;
  vals = new T[cap];
}

template <typename T> Vec<T>::Vec (size_t _Size)
{
  size = _Size;
  cap = (size / DEFAULT_VEC_CAP + 1) * DEFAULT_VEC_CAP;
  vals = new T[cap];
}

template <typename T> Vec<T>::Vec (Vec<T> &rhs)
{
  size = rhs.size;
  cap = rhs.cap;

  vals = new T[cap];

  for (size_t i = 0; i < size; i++)
    vals[i] = rhs.vals[i];
}

template <typename T> Vec<T>::Vec (Vec<T> &&rhs)
{
  size = rhs.size;
  cap = rhs.cap;
  std::swap (vals, rhs.vals);
}

template <typename T> Vec<T>::Vec (const Vec &rhs)
{
  size = rhs.size;
  cap = rhs.cap;

  vals = new T[cap];

  for (size_t i = 0; i < size; i++)
    vals[i] = rhs.vals[i];
}

template <typename T> Vec<T>::Vec (std::initializer_list<T> lst)
{
  size = lst.size ();
  cap = (size / DEFAULT_VEC_CAP + 1) * DEFAULT_VEC_CAP;
  vals = new T[cap];
  size_t i = 0;

  for (const T &p : lst)
    vals[i++] = p;
}

template <typename T>
Vec<T> &
Vec<T>::operator= (const Vec<T> &rhs)
{
  if (this == &rhs)
    return *this;
  delete[] vals;

  size = rhs.size;
  cap = rhs.cap;
  vals = new T[cap];

  for (size_t i = 0; i < size; ++i)
    vals[i] = rhs.vals[i];

  return *this;
}

template <typename T>
Vec<T> &
Vec<T>::operator= (Vec<T> &&rhs) noexcept
{
  if (this != &rhs)
    {
      delete[] vals;

      vals = rhs.vals;
      size = rhs.size;
      cap = rhs.cap;

      rhs.vals = nullptr;
      rhs.size = 0;
      rhs.cap = 0;
    }
  return *this;
}

template <typename T>
void
Vec<T>::resize (size_t _Cap)
{
  if (_Cap < cap)
    return;

  cap = _Cap;
  T *vp = vals;
  vals = new T[cap];

  for (size_t i = 0; i < size; i++)
    vals[i] = vp[i];

  delete[] vp;
}

template <typename T>
void
Vec<T>::push_back (T &v)
{
  if (size == cap)
    resize (2 * cap + 1);

  vals[size++] = v;
}

template <typename T>
void
Vec<T>::push_back (T &&v)
{
  if (size == cap)
    resize (2 * cap + 1);

  vals[size++] = std::move (v);
}

template <typename T>
T &
Vec<T>::pop_back ()
{
  if (!size)
    throw "vec_empty_array";

  return vals[--size];
}

template <typename T> Vec<T>::~Vec () { delete[] vals; }

template <typename T>
void
Vec<T>::insert (size_t i, T &v)
{
  if (i > size)
    {
      push_back (v);
      return;
    }

  T pres = vals[i];
  size_t j = i;
  vals[j++] = v;

  while (j < size)
    {
      T tmp = vals[j];
      vals[j] = pres;
      pres = tmp;
      j++;
    }

  push_back (pres);
}
template <typename T>
void
Vec<T>::insert (size_t i, T &&v)
{
  if (i > size)
    {
      push_back (v);
      return;
    }

  T pres = vals[i];
  size_t j = i;
  vals[j++] = std::move (v);

  while (j < size)
    {
      T tmp = vals[j];
      vals[j] = pres;
      pres = tmp;
      j++;
    }

  push_back (pres);
}

template <typename T>
void
Vec<T>::reverse ()
{
  int i = 0, j = get_size () - 1;

  while (i < j)
    {
      T tmp = vals[i];
      vals[i] = vals[j];
      vals[j] = tmp;

      i++;
      j--;
    }
}
} // namespace sf
