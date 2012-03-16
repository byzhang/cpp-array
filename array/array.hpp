/*
 * Copyright (C) 2011 by Alejandro M. Aragón
 * Written by Alejandro M. Aragón <alejandro.aragon@gmail.com>
 * All Rights Reserved
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

//! \file array.hpp
//
//  Created by Alejandro Aragón on 10/11/11.
//

#ifndef ARRAY_HPP
#define ARRAY_HPP

#include <iostream>
#include <cassert>
#include <iomanip>

#include "return_type.hpp"

__BEGIN_ARRAY_NAMESPACE__

using std::cout;
using std::endl;


template <int d>
struct Print;

template <int d, class Array>
struct Array_proxy;


template <bool B, class T = void>
struct enable_if { typedef T type; };

template <class T>
struct enable_if<false, T> {};


template <bool B, class T = void>
struct disable_if { typedef T type; };

template <class T>
struct disable_if<true, T> {};






template <int d, typename T>
class Array_base {
  
public:
  typedef T value_type;
  
  Array_base() : n_(), data_(0) {}
  
protected:
  size_t n_[d];
  value_type* data_;
};

template <typename T>
class Array_base<2,T> {
  
public:
  typedef T value_type;
  
  Array_base() : n_(), data_(0) {}
  
  size_t rows() const
  { return n_[0]; }
  
  size_t columns() const
  { return n_[1]; }
  
protected:
  size_t n_[2];
  value_type* data_;
};

template <class A, class B>
struct SameClass {
  enum { result = false }; 
};

template <class A>
struct SameClass<A,A> {
  enum { result = true }; 
};


/*! \tparam n - Dimension of array
 */
template <int d, typename T>
struct Array : public Array_base <d,T>  {
  //struct Array : public Array_base <d,T>, public enabled<Array<d,T> >  {
  
  typedef T* pointer_type;
  typedef T& reference_type;
  typedef T value_type;
  
  static const int d_ = d;
  
  typedef Array_base <d,T> base_type;
  
  using base_type::n_;
  using base_type::data_;
  
  // default constructor
  Array() : base_type() {}
  
  // parameter constructor
  template <typename... Args>
  explicit Array(const Args&... args) : base_type() {
    
#ifdef ARRAY_VERBOSE
    cout<<"Inside constructor template <typename... Args> explicit Array(const Args&... args)"<<endl;
#endif
    
    static_assert(sizeof...(Args) <= d , "*** ERROR *** Number of arguments exceeded array dimension");
    
    size_t sizes[] = { args... };
    
    size_t size = 1;
    for (size_t i=0; i<d; ++i) {
      n_[i] = i < sizeof...(Args) ? sizes[i] : sizes[sizeof...(Args) - 1];
      assert(n_[i] != 0);
      size *= n_[i];
    }
    
    data_ = new value_type[size];
    for (size_t i=0; i<size; ++i)
      data_[i] = value_type();
  }
  
  
  Array(const Array& a) : base_type() {
    
#ifdef ARRAY_VERBOSE
    cout<<"Inside constructor Array(const Array& a)"<<endl;
#endif
    
    for (size_t i=0; i<d; ++i)
      n_[i] = a.n_[i];
    
    if(a.data_) {
      size_t s = size();
      data_ = new value_type[s];
      for(size_t i=0; i<s; ++i)
        data_[i] = a.data_[i];
    } else
      data_ = 0;
  }
  
  Array& operator=(const Array& a) {
    
#ifdef ARRAY_VERBOSE
    cout<<"inside Array& operator=(const Array& a)"<<endl;
#endif
    
    // check for self-assignment
    if(this != &a) {
      
      for (int i=0; i<d; ++i) {
        n_[i] = a.n_[i];
        assert(n_[i] != 0);
      }
      
      // delete allocated memory
      delete[] data_;
      
      // check for null pointer
      if(a.data_) {
        size_t s = a.size();
        data_ = new value_type[s];
        for(size_t i=0; i<s; ++i)
          data_[i] = a.data_[i];
      } else
        data_ = 0;
    }
    return *this;
  }
  
  //    // source is a fnresult
  //    Array(fnresult<Array> src) {
  //        
  //#ifdef ARRAY_VERBOSE
  //        cout<<"inside Array(fnresult<Array>)"<<endl;
  //#endif
  //        data_ = src.data_;
  //        src.data_ = NULL;
  //        
  //        for (int i=0; i<d; ++i)
  //            n_[i] = src.n_[i];
  //    }
  //    
  //    // source is a temporary
  //    Array(temporary<Array> src) {
  //        
  //#ifdef ARRAY_VERBOSE
  //        cout<<"inside Array(temporary<Array>)"<<endl;
  //#endif
  //
  //        Array& rhs = src.get();
  //        data_ = rhs.data_;
  //        rhs.data_= NULL;
  //
  //        for (int i=0; i<d; ++i)
  //            n_[i] = rhs.n_[i];
  //    }
  
  
  // constructor taking an expression with transpose operator    
  Array(const Expr<BinExprOp<Array, EmptyType, ApTr> >& atr) {
    
#ifdef ARRAY_VERBOSE
    cout<<"inside constructor Array(const Expr<BinExprOp<Array, EmptyType, ApTr> >& atr)"<<endl;
#endif
    
    Array& a = *this;
    a = atr();
  }
  
  
  // constructor taking an arbitrary expression
  template <class A>
  Array(const Expr<A>& expr) {
    
#ifdef ARRAY_VERBOSE
    cout<<"inside template <class A> Array(const Expr<A>& expr)"<<endl;
#endif
    static_assert(SameClass<Array, typename A::result_type>::result, "*** ERROR *** Resulting expression is not of type array.");
    Array& a = *this;
    a = expr();
    
    //        execute(expr);
  }
  
  template <class A>
  typename enable_if<SameClass<Array, typename A::result_type>::result, void>::type
  execute(const Expr<A>& expr) {
    cout<<"INSIDE EXECUTE FOR MATRICES!"<<endl;
    
    Array& a = *this;
    a = expr();
  }
  
  template <class A>
  typename enable_if<!SameClass<Array, typename A::result_type>::result, void>::type
  execute(const Expr<A>& expr) {
    cout<<"INSIDE EXECUTE FOR NON MATRICES!"<<endl;
    
    static_assert(SameClass<Array, typename A::result_type>::result, "*** ERROR *** Resulting expression is not of type Array.");
  }
  
  //    
  //    template <class A>
  //    enable_if<!IsArray<typename A::result_type> >::result, void>::type
  //    execute(const Expr<A>& expr) {
  //        
  //        cout<<"INSIDE EXECUTE FOR OTHER STUFF!"<<endl;
  //        
  //        Array& a = *this;
  //        a = expr();
  //    }
  
  
  //    // unary operator-(any)
  //    template <class A>
  //    typename enable_if<!is_arithmetic<A>::value, Expr<BinExprOp<ExprLiteral<int>, A, ApMul> > >::type
  //    operator-(const A& a) {
  //        
  //#ifdef ARRAY_VERBOSE
  //        cout<<"1 Inside unary operator-(any), file "<<__FILE__<<", line "<<__LINE__<<endl;
  //        typedef BinExprOp<ExprLiteral<int>, A, ApMul> ExprT;
  //        cout<<"  expression type: "<<typeid(ExprT).name()<<endl;
  //#endif
  //        return (-1 *a);
  //    }
  
  ~Array() { delete data_; }
  
  size_t size() const {
    size_t n = 1;
    for (size_t i=0; i<d; ++i) {
      assert(n_[i] != 0);
      n *= n_[i];
    }
    return n;
  }
  
  
  template <typename... Args>
  reference_type operator()(Args... params) {
    
    // check that the number of parameters corresponds to the size of the array
    static_assert(sizeof...(Args) == d , "*** ERROR *** Number of arguments does not match array dimension.");
    
    // unpack parameters
    size_t indices[] = { params... };
    
    // return reference
    return data_[index(indices)];
  }
  
  
  template <typename... Args>
  value_type operator()(Args... params) const {
    
    // check that the number of parameters corresponds to the size of the array
    static_assert(sizeof...(Args) == d , "*** ERROR *** Number of arguments does not match array dimension.");
    
    // unpack parameters
    size_t indices[] = { params... };
    
    // return reference
    return data_[index(indices)];
  }
  
  friend std::ostream& operator<<(std::ostream& os, const Array& a) {
    Print<Array::d_>::print(os, a.n_, a.data_);
    return os;
  }
  
  typedef typename Array_proxy<d, Array>::reference_type proxy_reference_type;
  typedef typename Array_proxy<d, Array>::value_type proxy_value_type;
  
  proxy_reference_type operator[](size_t i)
  { return proxy_reference_type(*this, i); }
  
  proxy_value_type operator[](size_t i) const
  { return proxy_value_type(const_cast<Array&>(*this), i); }

  
private:
  
  size_t index(size_t indices[]) const {
    
    size_t i = 0, s = 1;
    for (int j=0; j<d_; ++j) {
      assert(indices[j] < n_[j]);
      i += s * indices[j];
      s *= n_[j];
    }
    return i;
  }
  
  template <int, class>
  friend struct Array_proxy;
  
  friend class DApMul;
  friend class DApDivide;
};

template <int d, class Array>
struct Array_proxy {
  
  typedef const Array_proxy<d-1, Array> value_type;
  typedef Array_proxy<d-1, Array> reference_type;
  
  explicit Array_proxy (const Array& a, size_t i) : a_(a), i_(i) {}
  
  template <int c>
  Array_proxy (const Array_proxy<c, Array>& a, size_t i) : a_(a.a_), i_(a.i_) {
    
    size_t s = 1;
    for (int j=0; j<Array::d_ - d - 1; ++j)
      s *= a_.n_[j];
    i_ += i*s;
  }

  reference_type operator[](size_t i)
  { return reference_type(*this, i); }
  
  value_type operator[](size_t i) const
  { return value_type(*this, i); }
  
  const Array& a_;
  size_t i_;
};


template <class Array>
struct Array_proxy<0, Array> {
  
  typedef typename Array::reference_type reference_type;
  typedef typename Array::value_type value_type;
  
  explicit Array_proxy (const Array& a, size_t i) : a_(a), i_(i) {}
  
  template <int d>
  Array_proxy (const Array_proxy<d, Array>& a, size_t i) : a_(a.a_), i_(a.i_) {
    
    size_t s = 1;
    for (int j=0; j<Array::d_ - 1; ++j)
      s *= a_.n_[j];
    i_ += i*s;
  }
  
  reference_type operator=(value_type v) {
    a_.data_[i_] = v;
    return a_.data_[i_];
  }
  reference_type operator+=(value_type v) {
    a_.data_[i_] += v;
    return a_.data_[i_];
  }
  reference_type operator-=(value_type v) {
    a_.data_[i_] -= v;
    return a_.data_[i_];
  }
  reference_type operator*=(value_type v) {
    a_.data_[i_] *= v;
    return a_.data_[i_];
  }
  reference_type operator/=(value_type v) {
    a_.data_[i_] /= v;
    return a_.data_[i_];
  }
  
  operator reference_type()
  { return a_.data_[i_]; }
  
  operator value_type() const
  { return a_.data_[i_]; }
  
  const Array& a_;
  size_t i_;
};


template <>
struct Print<1> {
  
  static std::ostream& print(std::ostream& os, const size_t size[], const double* data) {
    
    const size_t m = size[0];
    os<<"Array<1> ("<<m<<")"<<endl;
    for (size_t i=0; i<m; ++i)
      os<<" "<<data[i]<<endl;
    return os;
  }
};


template <>
struct Print<2> {
  
  static std::ostream& print(std::ostream& os, const size_t size[], const double* data) {
    
    const size_t m = size[0];
    const size_t n = size[1];
    
    os<<"Array<2> ("<<m<<"x"<<n<<")"<<endl;
    for (size_t i=0; i<m; ++i) {
      for (size_t j=0; j<n; ++j)
        os<<" "<<data[i + j*m];
      cout<<endl;
    }
    return os;
  }
};

template <int d>
struct Print {
  
  static std::ostream& print(std::ostream& os, const size_t size[], const double* data) {
    
    size_t s = 1;
    for (int i=0; i<d-1; ++i)
      s *= size[i];
    
    for (size_t i=0; i<size[d-1]; ++i) {
      os<<"Dim "<<d<<": "<<i<<", ";
      Print<d-1>::print(os, size, data + i*s);
    }
    return os;
  }
};

__END_ARRAY_NAMESPACE__

#endif /* ARRAY_HPP */
