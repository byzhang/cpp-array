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

//! \file expr.hpp
//
//  Created by Alejandro Aragón on 10/11/11.
//

#ifndef ARRAY_EXPR_HPP
#define ARRAY_EXPR_HPP


#include <iostream>
#include <typeinfo>

#include <boost/type_traits/is_arithmetic.hpp>
#include <boost/type_traits/is_reference.hpp>


extern "C"  {
#include <cblas.h>
}


#include "array-config.hpp"
#include "array.hpp"


__BEGIN_ARRAY_NAMESPACE__

using std::cout;
using std::endl;
using boost::is_arithmetic;

using typelist::NullType;


//! Expression identity, placeholder for a variable
template <typename T>
class ExprIdentity {
    
public:
    
    typedef T value_type;
    typedef T result_type;
    
    value_type operator()(T x) const {
#ifdef ARRAY_VERBOSE
        cout<<"1 Inside "<<typeid(value_type).name()<<" ExprIdentity::operator()("<<typeid(T).name()<<")"<<endl;
        cout<<"  value returned: "<<x<<endl;
#endif
        return x;
    }
};


//! Expression literal, represents a value that appears in the expression
template <typename T>
class ExprLiteral {
    
public:
    
    typedef T value_type;
    
    ExprLiteral(value_type value) : value_(value) {}
    
    template <typename... Args>
    value_type operator()(Args... params) const {
#ifdef ARRAY_VERBOSE
        cout<<"1 Inside "<<typeid(value_type).name()<<" ExprLiteral::operator()(Args...)"<<endl;
        cout<<"  value returned: "<<value_<<endl;
#endif
        return value_;
    }
    
    // provide implicit conversion
    operator value_type() const {
#ifdef ARRAY_VERBOSE
        cout<<"<"<<value_<<" imp conv to "<<typeid(value_type).name()<<">: ";
#endif
        return value_;
    }
    
private:
    value_type value_;
};




//! Expression wrapper class
template <class A>
class Expr {
    
private:
    A a_;
    
public:
    
    typedef typename A::value_type value_type;
    typedef typename A::result_type result_type;
    typedef typename A::left_type left_type;
    typedef typename A::right_type right_type;
    
    const left_type& left() const { return a_.left(); }
    const right_type& right() const { return a_.right(); }
    
    Expr() : a_() {}
    
    Expr(const A& x) : a_(x) {}
    
    result_type operator()() const {
        return a_();
    }
    
    result_type operator()(double x) const {
        //    result_type operator()(double x) const {
        return a_(x);
    }
    
    friend inline std::ostream& operator<<(std::ostream& os, const Expr<A>& expr) {
        return print<A>(os,expr);
    }
    
};

template <class>
struct Expr_traits;

template <class A>
struct Expr_traits<Expr<A> > {
    typedef Expr<A> type;
};

template <typename T>
struct Expr_traits<ExprLiteral<T> > {
    typedef ExprLiteral<T> type;
};

template <int d, typename T>
struct Expr_traits<Array<d,T> > {
    typedef const Array<d,T>& type;
};

template <>
struct Expr_traits<EmptyType > {
    typedef EmptyType type;
};


template<class A, class B, class Op>
class BinExprOp { 
    
    typename Expr_traits<A>::type a_;
    typename Expr_traits<B>::type b_;
    
public:
    
    typedef A left_type;
    typedef B right_type;
    typedef Op operator_type;
    typedef typename primitive<typename left_type::value_type, typename right_type::value_type>::result  value_type;
    typedef typename Return_type<left_type, right_type, operator_type>::result_type result_type;
    
    const left_type& left() const { 
        
#ifdef ARRAY_VERBOSE
        cout<<"    3 Inside BinExprOp::left()"<<endl;
        cout<<"      type of left:"<<typeid(left_type).name()<<endl;
#endif
        
        return a_; }
    
    const right_type& right() const { 
        
        
#ifdef ARRAY_VERBOSE
        cout<<"    3 Inside BinExprOp::right()"<<endl;
        cout<<"      type of right:"<<typeid(right_type).name()<<endl;
#endif
        return b_; }
    
    BinExprOp(const A& a, const B& b) : a_(a), b_(b) {
        
#ifdef ARRAY_VERBOSE
        cout<<"  2 Inside BinExprOp constructor"<<endl;
        cout<<"    type of A: "<<typeid(A).name()<<endl;
        cout<<"    type of B: "<<typeid(B).name()<<endl;
#endif
    }
    
    result_type operator()() const
    { return Op::apply(a_, b_); } 
    
    result_type operator()(double x) const
    { return Op::apply(a_(x), b_(x)); }
};




template<class A, class B, class Op>
class RefBinExprOp {
    
public:
    
    typedef A& reference_type;
    
private:
    
    reference_type a_;
    B b_;
    
public:
    
    typedef reference_type left_type;
    typedef B right_type;
    typedef Op operator_type;
    typedef typename A::value_type value_type;
    typedef reference_type result_type;
    
    RefBinExprOp(reference_type a, const B& b)
    : a_(a), b_(b) { 
#ifdef ARRAY_VERBOSE
        cout<<"Inside RefBinExprOp constructor"<<endl;
        cout<<"type of a:"<<typeid(A).name()<<endl;
        cout<<"type of b:"<<typeid(B).name()<<endl;
#endif
    }
    
    reference_type operator()() const { 
#ifdef ARRAY_VERBOSE
        cout<<"Inside RefBinExprOp::operator()()"<<endl;
#endif
        return Op::apply(a_, b_);
    }
    
    reference_type operator()(double x) const { 
#ifdef ARRAY_VERBOSE
        cout<<"Inside RefBinExprOp::operator()(double x)"<<endl;
        cout<<"result: "<< Op::apply(a_(x), b_(x))<<endl;
#endif
        return Op::apply(a_(x), b_(x));
    }
    
    friend std::ostream& operator<<(std::ostream& os, const RefBinExprOp& bop) {
#ifdef ARRAY_VERBOSE
        os<<"left: "<<bop.a_()<<endl;
        os<<"right: "<<bop.b_()<<endl;
#endif
        return os;
    }
};




////////////////////////////////////////////////////////////////////////////////
// blas functions

// level 1 blas xSCAL function: x <- alpha*x

static void cblas_scal(const int N, const double alpha, double *X, const int incX) {
    cblas_dscal(N, alpha, X, incX);
}

static void cblas_sscal(const int N, const float alpha, float *X, const int incX) {
    cblas_sscal(N, alpha, X, incX);
}


// level 1 blas xAXPY function: y <- alpha*x + y
static void cblas_axpy(const int N, const double alpha, const double *X,
                       const int incX, double *Y, const int incY) {
    cblas_daxpy(N, alpha, X, incX, Y, incY);
}

static void cblas_axpy(const int N, const float alpha, const float *X,
                       const int incX, float *Y, const int incY) {
    cblas_saxpy(N, alpha, X, incX, Y, incY);
}

// level 1 blas xDOT function: dot <- x'*y
static float cblas_dot(const int N, const float  *X, const int incX,
                       const float  *Y, const int incY) {
    return cblas_sdot(N, X, incX, Y, incY);
}

static double cblas_dot(const int N, const double *X, const int incX,
                        const double *Y, const int incY) {
    return cblas_ddot(N, X, incX, Y, incY);
}

// level 2 blas xGER function: A <- alpha*x*y' + A
static void cblas_ger(const int M, const int N,
                      const double alpha, const double *X, const int incX,
                      const double *Y, const int incY, double *A, const int lda) {
    cblas_dger(CblasColMajor, M, N, alpha, X, incX,
               Y, incY, A, lda);
}

static void cblas_ger(const int M, const int N,
                      const float alpha, const float *X, const int incX,
                      const float *Y, const int incY, float *A, const int lda) {
    cblas_sger(CblasColMajor, M, N, alpha, X, incX,
               Y, incY, A, lda);
}


// level 2 blas xGEMV function: Y <- alpha*A*x + beta*y
static void cblas_gemv(const enum CBLAS_TRANSPOSE TransA,
                       const int M, const int N,
                       const double alpha,
                       const double *A,
                       const int lda,
                       const double *X,
                       const int incX,
                       const double beta,
                       double *Y,
                       const int incY) {
    
    cblas_dgemv(CblasColMajor, TransA, M, N,
                alpha, A, lda, X, incX, beta, Y, incY);
}

static void cblas_gemv(const enum CBLAS_TRANSPOSE TransA,
                       const int M,
                       const int N,
                       const float alpha,
                       const float *A,
                       const int lda,
                       const float *X,
                       const int incX,
                       const float beta,
                       float *Y,
                       const int incY) {
    
    cblas_sgemv(CblasColMajor, TransA, M, N,
                alpha, A, lda, X, incX, beta, Y, incY);
}


// level 3 blas xGEMM function: C <- alpha*op(A)*op(B) = beta*C, op(X) = X, X'
/*! \f$ C \leftarrow \alpha op(A)op(B) + \beta C \f$
 *
 * Arguments:
 *
 *    transA   specifies the form of (op)A used in the multiplication:
 *             CblasNoTrans -> (op)A = A, CblasTrans -> (op)A = transpose(A)
 *    transB   specifies the form of (op)B used in the multiplication:
 *             CblasNoTrans -> (op)B = B, CblasTrans -> (op)B = transpose(B)
 *    M        the number of rows of the matrix (op)A and of the matrix C
 *    N        the number of columns of the matrix (op)B and of the matrix C
 *    K        the number of columns of the matrix (op)A and the number of rows of the matrix (op)B
 *    alpha    specifies the scalar alpha
 *    A        a two-dimensional array A
 *    lda      the first dimension of array A
 *    B        a two-dimensional array B
 *    ldb      the first dimension of array B
 *    beta     specifies the scalar beta
 *    C        a two-dimensional array
 *    ldc      the first dimension of array C
 */
static void cblas_gemm(const enum CBLAS_TRANSPOSE TransA,
                       const enum CBLAS_TRANSPOSE TransB, const int M, const int N,
                       const int K, const float alpha, const float *A,
                       const int lda, const float *B, const int ldb,
                       const float beta, float *C, const int ldc) {
    
    cblas_sgemm(CblasColMajor, TransA, TransB, M, N, K, alpha, A,
                lda, B, ldb, beta, C, ldc);
}


static void cblas_gemm(const enum CBLAS_TRANSPOSE TransA,
                       const enum CBLAS_TRANSPOSE TransB, const int M, const int N,
                       const int K, const double alpha, const double *A,
                       const int lda, const double *B, const int ldb,
                       const double beta, double *C, const int ldc) {
    
    cblas_dgemm(CblasColMajor, TransA, TransB, M, N, K, alpha, A,
                lda, B, ldb, beta, C, ldc);
}



// Applicative class for the addition operation
class ApAdd {
public:
    
    ////////////////////////////////////////////////////////////////////////////////
    // return references
    
    // array - scalar*array addition
    template <int d, typename T>
    static Array<d,T>& apply(Array<d,T>& a, 
                             const Expr< BinExprOp< ExprLiteral<T>, Array<d,T>, ApMul> > & y) { 
        
        const Array<d,T>& b = y.right();
        
        // size assertion
        for (size_t i=0; i<d; ++i)
            assert(a.n_[i] == b.n_[i]);
        
        cblas_axpy(a.size(), static_cast<T>(y.left()), b.data_, 1, a.data_, 1);
#ifdef ARRAY_VERBOSE
        cout<<"\n      4 Inside ApAdd::apply(array&, scalar*array)"<<endl;
#endif
        return a;
    }
    
    // array - array addition
    template <int d, typename T>
    static Array<d,T> apply(const Array<d,T>& a, const Array<d,T>& b) { 
        
        // size assertion
        for (size_t i=0; i<d; ++i)
            assert(a.n_[i] == b.n_[i]);
        
        Array<d,T> r(b);
        cblas_axpy(a.size(), T(1), a.data_, 1, r.data_, 1);
#ifdef ARRAY_VERBOSE
        cout<<"\n      4 Inside ApAdd::apply(array, array)"<<endl;
#endif
        return r;
    }
    
    // array - (scalar*array - scalar*array multiplication) addition
    template <typename T>
    static Array<2,T>& apply(Array<2,T>& c, 
                             const Expr< BinExprOp< 
                             Expr< BinExprOp< ExprLiteral<T>, Array<2,T>, ApMul > >, 
                             Expr< BinExprOp< ExprLiteral<T>, Array<2,T>, ApMul > >, 
                             ApMul > >& y) {
        
        // get matrix refernces
        const Array<2,T>& a = y.left().right();
        const Array<2,T>& b = y.right().right();
        
        // check size
        assert(a.columns() == b.rows());
        assert(c.rows() == a.rows());
        assert(c.columns() == b.columns());
        
        cblas_gemm(CblasNoTrans, CblasNoTrans, a.rows(), b.columns(),
                   a.columns(), static_cast<T>(y.left().left())*static_cast<T>(y.right().left()),
                   a.data_, a.rows(), b.data_, b.rows(), 1.0, c.data_, c.rows());
        
#ifdef ARRAY_VERBOSE
        cout<<"\n      4 Inside ApAdd::apply(matrix&, scalar*matrix*matrix)"<<endl;
#endif
        return c;
    }
    
    // array - expr addition
    template<int d, typename T, class B>
    static Array<d,T>& apply(Array<d,T>& a, const Expr<B>& b) {
        
#ifdef ARRAY_VERBOSE
        cout<<"\n*** INFO *** Applying general ApAdd::apply(array&, expr)"<<endl;
        cout<<"               left: "<<typeid(a).name()<<endl;
        cout<<"               right: "<<typeid(b).name()<<endl;
#endif
        return a =+ b();
    }
    
    
    ////////////////////////////////////////////////////////////////////////////////
    // return new objects
    
    // scalar*array - scalar*array addition
    template <int d, typename T>
    static Array<d, T> apply(const Expr< BinExprOp< ExprLiteral<T>, Array<d,T>, ApMul> >& x,
                             const Expr< BinExprOp< ExprLiteral<T>, Array<d,T>, ApMul> >& y) {
        
        // get matrix refernces
        const Array<d, T>& a = x.right();
        const Array<d, T>& b = y.right();
        
        // size assertion
        for (size_t i=0; i<d; ++i)
            assert(a.n_[i] == b.n_[i]);
        
        // initialize result to first product
        Array<d, T> r = (static_cast<T>(x.left())*a)();
        
        // add second product
        cblas_axpy(a.size(), static_cast<T>(y.left()), b.data_, 1, r.data_, 1);
        
#ifdef ARRAY_VERBOSE
        cout<<"\n      4 Inside ApAdd::apply(scalar*array, scalar*array)"<<endl;
#endif
        return r;
    }
    
    // expr - expr addition
    template<class A, class B>
    static typename Return_type<Expr<A>, Expr<B>, ApMul>::result_type
    apply(const Expr<A>& a, const Expr<B>& b) {
#ifdef ARRAY_VERBOSE
        cout<<"\n*** INFO *** Applying general ApAdd::apply(expr, expr)"<<endl;
        cout<<"               left: "<<typeid(a).name()<<endl;
        cout<<"               right: "<<typeid(b).name()<<endl;
#endif
        return (a()+b())();
    }
};

// Applicative class for the subtraction operation
class ApSub {
public:
    
    // array - array addition
    template <int d, typename T>
    static Array<d,T> apply(const Array<d,T>& a, const Array<d,T>& b) { 
        
        // size assertion
        for (size_t i=0; i<d; ++i)
            assert(a.n_[i] == b.n_[i]);
        
        Array<d,T> r(a);
        cblas_axpy(r.size(), T(-1), b.data_, 1, r.data_, 1);
        return r;
    }
    // expr - expr addition
    template<class A, class B>
    static typename Return_type<Expr<A>, Expr<B>, ApMul>::result_type
    apply(const Expr<A>& a, const Expr<B>& b) {
#ifdef ARRAY_VERBOSE
        cout<<"\n*** INFO *** Applying general ApSub::apply(expr, expr)"<<endl;
        cout<<"               left: "<<typeid(a).name()<<endl;
        cout<<"               right: "<<typeid(b).name()<<endl;
#endif
        return (a()-b())();
    }
};

// Applicative class for the multiplication operation
class ApMul {
public:
    
    // scalar-array multiplication
    template <int d, typename T>
    static Array<d,T> apply(const ExprLiteral<T>& a, const Array<d,T>& b) {
        
        // \todo this could be replaced by combining the constructor and 
        // initialization using the scalar
        Array<d,T> r(b);
        cblas_scal(b.size(), static_cast<T>(a), r.data_, 1);
#ifdef ARRAY_VERBOSE
        cout<<"\n      4 Inside ApMul::apply(scalar, array)"<<endl;
#endif
        return r;
    }
    
    // scalar-transposed array multiplication
    template <int d, typename T>
    static Array<d,T> apply(const ExprLiteral<T>& a, const Expr< BinExprOp<Array<d,T>, EmptyType, ApTr> >& b) {
        
#ifdef ARRAY_VERBOSE
        cout<<"\n      4 Inside ApMul::apply(scalar, transposed array)"<<endl;
#endif
        return static_cast<T>(a) * b();
    }
    
    // transposed vector - scalar*vector multiplication
    template <typename T>
    static T apply(const Expr<BinExprOp<Array<1,T>, EmptyType, ApTr> >& a,
                   const Expr<BinExprOp<ExprLiteral<T>, Array<1,T>, ApMul> >& b) {
        
        const Array<1,T>& x = a.left();
        const Array<1,T>& y = b.right();
        
        assert(x.size() == y.size());
        
#ifdef ARRAY_VERBOSE
        cout<<"\n      4 Inside ApMul::apply(transposed vector, scalar*vector)"<<endl;
#endif
        return static_cast<T>(b.left())*cblas_dot(x.size(), x.data_, 1, y.data_, 1);
    }
    
    // scalar*vector - transposed vector multiplication
    template <typename T>
    static Array<2,T> apply(const Expr<BinExprOp<ExprLiteral<T>, Array<1,T>, ApMul> >& a,
                            const Expr<BinExprOp<Array<1,T>, EmptyType, ApTr> >& b) {
        
        const Array<1,T>& x = a.right();
        const Array<1,T>& y = b.left();
        
        Array<2,T> r(x.size(), y.size());
        
        cblas_ger(x.size(), y.size(), static_cast<T>(a.left()), x.data_, 1, y.data_, 1, r.data_, r.rows());
        
#ifdef ARRAY_VERBOSE
        cout<<"\n      4 Inside ApMul::apply(scalar*vector, transposed vector)"<<endl;
#endif
        return r;
    }
    
    // scalar*transposed vector - scalar*vector multiplication
    template <typename T>
    static T apply(const Expr<BinExprOp<ExprLiteral<T>, Expr<BinExprOp<Array<1,T>, EmptyType, ApTr> >, ApMul> >& a,
                   const Expr<BinExprOp<ExprLiteral<T>, Array<1,T>, ApMul> >& b) {
        
        const Array<1,T>& x = a.right().left();
        const Array<1,T>& y = b.right();
        
        assert(x.size() == y.size());
        
#ifdef ARRAY_VERBOSE
        cout<<"\n      4 Inside ApMul::apply(scalar*transposed vector, scalar*vector)"<<endl;
#endif
        return static_cast<T>(b.left())*static_cast<T>(a.left())*cblas_dot(x.size(), x.data_, 1, y.data_, 1);
    }
    
    // scalar*vector - scalar*transposed vector multiplication
    template <typename T>
    static Array<2,T> apply(const Expr<BinExprOp<ExprLiteral<T>, Array<1,T>, ApMul> >& a,
                            const Expr<BinExprOp<ExprLiteral<T>, Expr<BinExprOp<Array<1,T>, EmptyType, ApTr> >, ApMul> >& b) {
        
        const Array<1,T>& x = a.right();
        const Array<1,T>& y = b.right().left();
        
        Array<2,T> r(x.size(), y.size());
        
        cblas_ger(x.size(), y.size(), static_cast<T>(a.left())*static_cast<T>(b.left()), x.data_, 1, y.data_, 1, r.data_, r.rows());
        
#ifdef ARRAY_VERBOSE
        cout<<"\n      4 Inside ApMul::apply(scalar*vector, scalar*transposed vector)"<<endl;
#endif
        return r;
    }
    
    // scalar*matrix - scalar*matrix multiplication
    template <typename T>
    static Array<2, T>
    apply(const Expr< BinExprOp< ExprLiteral<T>, Array<2,T>, ApMul> >& x,
          const Expr< BinExprOp< ExprLiteral<T>, Array<2,T>, ApMul> >& y) {
        
        // get matrix refernces
        const Array<2, T>& a = x.right();
        const Array<2, T>& b = y.right();
        
        // check size
        assert(a.columns() == b.rows());
        
        Array<2, T> r(a.rows(), b.columns());
        cblas_gemm(CblasNoTrans, CblasNoTrans, r.rows(), r.columns(),
                   a.columns(), static_cast<T>(x.left())*static_cast<T>(y.left()),
                   a.data_, a.rows(), b.data_, b.rows(), 1.0, r.data_, r.rows());
        
#ifdef ARRAY_VERBOSE
        cout<<"\n      4 Inside ApMul::apply(scalar*matrix, scalar*matrix)"<<endl;
#endif
        return r;
    }
    
    // scalar*matrix - scalar*vector multiplication
    template <typename T>
    static Array<1, T> apply(const Expr< BinExprOp< ExprLiteral<T>, Array<2,T>, ApMul> >& x,
                             const Expr< BinExprOp< ExprLiteral<T>, Array<1,T>, ApMul> >& y) {
        
        // get matrix refernces
        const Array<2, T>& a = x.right();
        const Array<1, T>& b = y.right();
        
        // check size
        assert(a.columns() == b.size());
        
        Array<1, T> r(b.size());
        cblas_gemv(CblasNoTrans, a.rows(), a.columns(), static_cast<T>(x.left() * y.left()),
                   a.data_, a.rows(), b.data_, 1, 1., r.data_, 1);
#ifdef ARRAY_VERBOSE
        cout<<"\n      4 Inside ApMul::apply(scalar*matrix, scalar*vector)"<<endl;
#endif
        
        return r;
    }
    
    // scalar*transposed matrix - scalar*matrix multiplication
    template <typename T>
    static Array<2, T> apply(const Expr< BinExprOp< ExprLiteral<T>, Expr< BinExprOp< Array<2,T>, EmptyType, ApTr> >, ApMul> > &x,
                             const Expr< BinExprOp< ExprLiteral<T>, Array<2,T>, ApMul> > &y) {
        
        // get matrix refernces
        const Array<2, T>& a = x.right().left();
        const Array<2, T>& b = y.right();
        
        // check size
        assert(a.rows() == b.rows());
        
        Array<2, T> r(a.columns(), b.columns());
        cblas_gemm(CblasTrans, CblasNoTrans, r.rows(), r.columns(),
                   a.rows(), static_cast<T>(x.left())*static_cast<T>(y.left()),
                   a.data_, a.rows(), b.data_, b.rows(), 1.0, r.data_, r.rows());
        
#ifdef ARRAY_VERBOSE
        cout<<"\n      4 Inside ApMul::apply(scalar*transposed matrix, scalar*matrix)"<<endl;
#endif
        return r;
    }
    
    
    // scalar*matrix - scalar*transposed matrix multiplication
    template <typename T>
    static Array<2, T> apply(const Expr< BinExprOp< ExprLiteral<T>, Array<2,T>, ApMul> > &x,
                             const Expr< BinExprOp< ExprLiteral<T>, Expr< BinExprOp< Array<2,T>, EmptyType, ApTr> >, ApMul> > &y) {
        
        // get matrix refernces
        const Array<2, T>& a = x.right();
        const Array<2, T>& b = y.right().left();
        
        // check size
        assert(a.columns() == b.columns());
        
        Array<2, T> r(a.rows(), b.rows());
        cblas_gemm(CblasNoTrans, CblasTrans, r.rows(), r.columns(),
                   a.columns(), static_cast<T>(x.left())*static_cast<T>(y.left()),
                   a.data_, a.rows(), b.data_, b.rows(), 1.0, r.data_, r.rows());
        
#ifdef ARRAY_VERBOSE
        cout<<"\n      4 Inside ApMul::apply(scalar*matrix, scalar*transposed matrix)"<<endl;
#endif
        return r;
    }
    
    // scalar*transposed matrix - scalar*transposed matrix multiplication
    template <typename T>
    static Array<2, T> apply(const Expr< BinExprOp< ExprLiteral<T>, Expr< BinExprOp< Array<2,T>, EmptyType, ApTr> >, ApMul> > &x,
                             const Expr< BinExprOp< ExprLiteral<T>, Expr< BinExprOp< Array<2,T>, EmptyType, ApTr> >, ApMul> > &y) {
        
        // get matrix refernces
        const Array<2, T>& a = x.right().left();
        const Array<2, T>& b = y.right().left();
        
        // check size
        assert(a.rows() == b.columns());
        
        Array<2, T> r(a.columns(), b.rows());
        cblas_gemm(CblasTrans, CblasTrans, r.rows(), r.columns(),
                   a.rows(), static_cast<T>(x.left())*static_cast<T>(y.left()),
                   a.data_, a.rows(), b.data_, b.rows(), 1.0, r.data_, r.rows());
        
#ifdef ARRAY_VERBOSE
        cout<<"\n      4 Inside ApMul::apply(scalar*transposed matrix, scalar*transposed matrix)"<<endl;
#endif
        return r;
    }
    
    // expr - expr multiplication
    template<class A, class B>
    static typename Return_type<Expr<A>, Expr<B>, ApMul>::result_type
    apply(const Expr<A>& a, const Expr<B>& b) {
#ifdef ARRAY_VERBOSE
        cout<<"\n*** INFO *** Applying general ApMul::apply(expr, expr)"<<endl;
        cout<<"               left: "<<typeid(a).name()<<endl;
        cout<<"               right: "<<typeid(b).name()<<endl;
#endif
        return (a()*b())();
    }
    
};

// Applicative class for the division operation
class ApDiv {
public:
    
    // expr - expr divition
    template<class A, class B>
    static typename Return_type<Expr<A>, Expr<B>, ApMul>::result_type
    apply(const Expr<A>& a, const Expr<B>& b) {
#ifdef ARRAY_VERBOSE
        cout<<"\n*** INFO *** Applying general ApDiv::apply(expr, expr)"<<endl;
        cout<<"               left: "<<typeid(a).name()<<endl;
        cout<<"               right: "<<typeid(b).name()<<endl;
#endif
        return (a()/b())();
    }
};


// ApTr -- transpose
struct ApTr {
    
    //    // this function should never be called
    //    template <typename T>
    //    static inline Array<1,T> apply(const Array<1,T>& a, EmptyType) {
    //        cout<<"*** ERROR *** Cannot return the transpose of a vector"<<endl;
    //        exit(1);
    //    }
    
    template <typename T>
    static inline Array<2,T> apply(const Array<2,T>& a, EmptyType) {
        
        Array<2,T> r(a.columns(), a.rows());
        for(size_t i=0; i<r.rows(); ++i)
            for(size_t j=0; j<r.columns(); ++j)
                r(i,j) = a(j,i);
        return r;
    }
    
    //    static inline double apply(const M& A, size_t i, size_t j) {
    //#ifdef CPPUTILS_ARRAY_VERBOSE
    //        cout<<"Inside TrOp::apply(M,i,j)"<<endl;
    //#endif
    //        
    //        return A(j,i);
    //    }
};


////////////////////////////////////////////////////////////////////////////////
// unary operator+

// unary operator+(any)
template <class A>
typename enable_if<!is_arithmetic<A>::value, A>::type
operator+(const A& a) {
    cout<<"copy!"<<endl;
    return a;
}

////////////////////////////////////////////////////////////////////////////////
// unary operator-

// unary operator-(any)
template <class A>
typename enable_if<!is_arithmetic<A>::value, Expr<BinExprOp<ExprLiteral<int>, A, ApMul> > >::type
operator-(const A& a) {
    
#ifdef ARRAY_VERBOSE
    cout<<"1 Inside unary operator-(any)"<<endl;
    typedef BinExprOp<ExprLiteral<int>, A, ApMul> ExprT;
    cout<<"  expression type: "<<typeid(ExprT).name()<<endl;
#endif
    return (-1 *a);
}

////////////////////////////////////////////////////////////////////////////////
// operator+

// operator+(expr, expr)
template<class A, class B>
Expr<BinExprOp<Expr<A>, Expr<B>, ApAdd> >
operator+(const Expr<A>& a, const Expr<B>& b) {
    
    typedef BinExprOp<Expr<A>, Expr<B>, ApAdd> ExprT;
#ifdef ARRAY_VERBOSE
    cout<<"1 Inside operator+(expr, expr)"<<endl;
    cout<<"  expression type: "<<typeid(ExprT).name()<<endl;
#endif
    return Expr<ExprT>(ExprT(a,b));
}

// operator+(array, array)
template <int d, typename T>
Expr<BinExprOp<Array<d,T>, Array<d,T>, ApAdd> >
operator+(const Array<d,T>& a, const Array<d,T>& b) {
    
    typedef BinExprOp<Array<d,T>, Array<d,T>, ApAdd> ExprT;
#ifdef ARRAY_VERBOSE
    cout<<"1 Inside operator+(array, array)"<<endl;
    cout<<"  expression type: "<<typeid(ExprT).name()<<endl;
#endif
    return Expr<ExprT>(ExprT(a,b));
}

// operator+(expr, array)
template<int d, typename T, class A>
Expr< BinExprOp<
Expr<A>,
Expr< BinExprOp< ExprLiteral<T>, Array<d,T>, ApMul> >, 
ApAdd> >
operator+(const Expr<A>& a, const Array<d,T>& b) {
    
    typedef BinExprOp<
    Expr<A>, 
    Expr< BinExprOp< ExprLiteral<T>, Array<d,T>, ApMul> >, 
    ApAdd> ExprT;
#ifdef ARRAY_VERBOSE
    cout<<"1 Inside operator+(expr, array)"<<endl;
    cout<<"  expression type: "<<typeid(ExprT).name()<<endl;
#endif
    return Expr<ExprT>(ExprT(a, T(1)*b));
}

// operator+(array, expr)
template<int d, typename T, class B>
Expr< BinExprOp<
Expr< BinExprOp< ExprLiteral<T>, Array<d,T>, ApMul> >, 
Expr<B>,
ApAdd> >
operator+(const Array<d,T>& a, const Expr<B>& b) {
    
    typedef BinExprOp<
    Expr< BinExprOp< ExprLiteral<T>, Array<d,T>, ApMul> >, 
    Expr<B>, 
    ApAdd> ExprT;
#ifdef ARRAY_VERBOSE
    cout<<"1 Inside operator+(array, expr)"<<endl;
    cout<<"  expression type: "<<typeid(ExprT).name()<<endl;
#endif
    return Expr<ExprT>(ExprT(T(1)*a, b));
}


////////////////////////////////////////////////////////////////////////////////
// operator-

// operator-(expr, expr)
template<class A, class B>
Expr<BinExprOp<Expr<A>, Expr<B>, ApSub> >
operator-(const Expr<A>& a, const Expr<B>& b) {
    
    typedef BinExprOp<Expr<A>, Expr<B>, ApSub> ExprT;
#ifdef ARRAY_VERBOSE
    cout<<"1 Inside operator-(expr, expr)"<<endl;
    cout<<"  expression type: "<<typeid(ExprT).name()<<endl;
#endif
    return Expr<ExprT>(ExprT(a,b));
}

// operator-(array, array)
template <int d, typename T>
Expr<BinExprOp<Array<d,T>, Array<d,T>, ApSub> >
operator-(const Array<d,T>& a, const Array<d,T>& b) {
    
    typedef BinExprOp<Array<d,T>, Array<d,T>, ApSub> ExprT;
#ifdef ARRAY_VERBOSE
    cout<<"1 Inside operator-(array, array)"<<endl;
    cout<<"  expression type: "<<typeid(ExprT).name()<<endl;
#endif
    return Expr<ExprT>(ExprT(a,b));
}

// operator-(expr, array)
template<int d, typename T, class A>
Expr< BinExprOp<
Expr<A>,
Expr< BinExprOp< ExprLiteral<T>, Array<d,T>, ApMul> >, 
ApAdd> >
operator-(const Expr<A>& a, const Array<d,T>& b) {
    
    typedef BinExprOp<
    Expr<A>, 
    Expr< BinExprOp< ExprLiteral<T>, Array<d,T>, ApMul> >, 
    ApAdd> ExprT;
#ifdef ARRAY_VERBOSE
    cout<<"1 Inside operator-(expr, array)"<<endl;
    cout<<"  expression type: "<<typeid(ExprT).name()<<endl;
#endif
    return Expr<ExprT>(ExprT(a, T(-1)*b));
}


// operator-(array, scalar*array)
template <int d, class T>
Expr<
BinExprOp<
Expr< BinExprOp< ExprLiteral<T>, Array<d,T>, ApMul> >, 
Expr< BinExprOp< ExprLiteral<T>, Array<d,T>, ApMul> >,
ApAdd>
>
operator-(const Array<d,T>& a, const Expr<BinExprOp< ExprLiteral<T>, Array<d,T>, ApMul> >& b) {
    
    typedef Expr< BinExprOp< ExprLiteral<T>, Array<d,T>, ApMul> > inner_expression;
    
    typedef BinExprOp< inner_expression, inner_expression, ApAdd> ExprT;
#ifdef ARRAY_VERBOSE
    cout<<"1 Inside operator-(array, scalar*array)"<<endl;
    cout<<"  expression type: "<<typeid(ExprT).name()<<endl;
#endif
    
    T factor = T(-1) * static_cast<T>(b.left());
    return Expr<ExprT>(ExprT(T(1)*a, factor*b.right()));
}


////////////////////////////////////////////////////////////////////////////////
// operator*

// operator*(expr, expr)
template<class A, class B>
Expr<BinExprOp<Expr<A>, Expr<B>, ApMul> >
operator*(const Expr<A>& a, const Expr<B>& b) {
    
    typedef BinExprOp<Expr<A>, Expr<B>, ApMul> ExprT;
#ifdef ARRAY_VERBOSE
    cout<<"1 Inside operator*(expr, expr)"<<endl;
    cout<<"  expression type: "<<typeid(ExprT).name()<<endl;
#endif
    return Expr<ExprT>(ExprT(a,b));
}

// operator*(array, array)
template<int d1, int d2, typename T>
Expr< BinExprOp<
Expr< BinExprOp< ExprLiteral<T>, Array<d1,T>, ApMul> >, 
Expr< BinExprOp< ExprLiteral<T>, Array<d2,T>, ApMul> >, 
ApMul> >
operator*(const Array<d1,T>& a, const Array<d2,T>& b) {
    
    typedef BinExprOp<
    Expr< BinExprOp< ExprLiteral<T>, Array<d1,T>, ApMul> >, 
    Expr< BinExprOp< ExprLiteral<T>, Array<d2,T>, ApMul> >, 
    ApMul> ExprT;
#ifdef ARRAY_VERBOSE
    cout<<"1 Inside operator*(array, array)"<<endl;
    cout<<"  expression type: "<<typeid(ExprT).name()<<endl;
#endif
    
    return Expr<ExprT>(ExprT(T(1)*a, T(1)*b));
}

// operator*(scalar, expr)
template <typename S, class B>
typename enable_if<is_arithmetic<S>::value, Expr<BinExprOp< ExprLiteral<typename Expr<B>::value_type >, Expr<B>, ApMul> > >::type
operator*(S a, const Expr<B>& b) {
    
    typedef typename Expr<B>::value_type value_type;
    typedef BinExprOp< ExprLiteral<value_type>, Expr<B>, ApMul> ExprT;
#ifdef ARRAY_VERBOSE
    cout<<"1 Inside operator*(scalar, expr), with scalar="<<a<<""<<endl;
    cout<<"  expression type: "<<typeid(ExprT).name()<<endl;
    cout<<"  value type: "<<typeid(value_type).name()<<endl;
#endif
    return Expr<ExprT>(ExprT(ExprLiteral<value_type>(a),b));
}

// operator*(expr, scalar)
template <typename S, class A>
typename enable_if<is_arithmetic<S>::value, Expr<BinExprOp< ExprLiteral<typename Expr<A>::value_type>, Expr<A>, ApMul> > >::type
operator*(const Expr<A>& a, S b) {
    
    typedef typename Expr<A>::value_type value_type;
    typedef BinExprOp< ExprLiteral<value_type>, Expr<A>, ApMul> ExprT;
#ifdef ARRAY_VERBOSE
    cout<<"1 Inside operator*(expr, scalar), with scalar="<<b<<""<<endl;
    cout<<"  expression type: "<<typeid(ExprT).name()<<endl;
    cout<<"  value type: "<<typeid(value_type).name()<<endl;
#endif
    return Expr<ExprT>(ExprT(ExprLiteral<value_type>(b),a));
}

// operator*(scalar, scalar*expr)
template <typename S, class T, class B>
typename enable_if<is_arithmetic<S>::value, Expr<BinExprOp< ExprLiteral<T>, Expr<B>, ApMul> > >::type
operator*(S a, const Expr<BinExprOp< ExprLiteral<T>, Expr<B>, ApMul> >& b) {
    
    typedef BinExprOp< ExprLiteral<T>, Expr<B>, ApMul> ExprT;
#ifdef ARRAY_VERBOSE
    cout<<"1 Inside operator*(scalar, scalar*expr), with scalar="<<a<<""<<endl;
    cout<<"  expression type: "<<typeid(ExprT).name()<<endl;
#endif
    return Expr<ExprT>(ExprT(ExprLiteral<T>(a*b.left()),b.right()));
}


// operator*(scalar, scalar*expr*expr)
template <typename S, class T, class A, class B>
typename enable_if<is_arithmetic<S>::value, Expr<BinExprOp< Expr<BinExprOp<ExprLiteral<T>, A , ApMul> >, Expr<B>, ApMul> > >::type
operator*(S a, const Expr<BinExprOp< Expr<BinExprOp<ExprLiteral<T>, A , ApMul> >, Expr<B>, ApMul> >& b) {
    
    typedef BinExprOp< Expr<BinExprOp<ExprLiteral<T>, A , ApMul> >, Expr<B>, ApMul> ExprT;
#ifdef ARRAY_VERBOSE
    cout<<"1 Inside operator*(scalar, scalar*expr*expr), with scalar="<<a<<""<<endl;
    cout<<"  expression type: "<<typeid(ExprT).name()<<endl;
#endif
    T scalar = a*b.left().left();
    const A& left_expr = b.left().right();
    const Expr<B>& right_expr = b.right();
    
    return Expr<ExprT>(ExprT(scalar*left_expr, right_expr));
}


// operator*(scalar*expr, scalar)
template <typename S, class T, class A>
typename enable_if<is_arithmetic<S>::value, Expr<BinExprOp< ExprLiteral<T>, Expr<A>, ApMul> > >::type
operator*(const Expr<BinExprOp< ExprLiteral<T>, Expr<A>, ApMul> >& a, S b) {
    
    typedef BinExprOp< ExprLiteral<T>, Expr<A>, ApMul> ExprT;
#ifdef ARRAY_VERBOSE
    cout<<"1 Inside operator*(scalar*expr, scalar), with scalar="<<b<<""<<endl;
    cout<<"  expression type: "<<typeid(ExprT).name()<<endl;
#endif
    return Expr<ExprT>(ExprT(ExprLiteral<T>(a.left()*b),a.right()));
}

// operator*(scalar, array)
template <int d, typename S, typename T>
typename enable_if<is_arithmetic<S>::value, Expr<BinExprOp< ExprLiteral<T>, Array<d,T>, ApMul> > >::type
operator*(S a, const Array<d,T>& b) {
    
    typedef BinExprOp< ExprLiteral<T>, Array<d,T>, ApMul> ExprT;
#ifdef ARRAY_VERBOSE
    cout<<"1 Inside operator*(scalar, array), with scalar="<<a<<""<<endl;
    cout<<"  expression type: "<<typeid(ExprT).name()<<endl;
#endif
    return Expr<ExprT>(ExprT(ExprLiteral<T>(a),b));
}

// operator*(array, scalar)
template <int d, typename S, typename T>
typename enable_if<is_arithmetic<S>::value, Expr<BinExprOp< ExprLiteral<T>, Array<d,T>, ApMul> > >::type
operator*(const Array<d,T>& a, S b) {
    
    typedef BinExprOp< ExprLiteral<T>, Array<d,T>, ApMul> ExprT;
#ifdef ARRAY_VERBOSE
    cout<<"1 Inside operator*(array, scalar), with scalar="<<b<<""<<endl;
    cout<<"  expression type: "<<typeid(ExprT).name()<<endl;
#endif
    return Expr<ExprT>(ExprT(ExprLiteral<T>(b),a));
}

// operator*(scalar*array, scalar)
template <int d, typename S, typename T>
typename enable_if<is_arithmetic<S>::value, Expr<BinExprOp< ExprLiteral<T>, Array<d,T>, ApMul> > >::type
operator*(const Expr<BinExprOp< ExprLiteral<T>, Array<d,T>, ApMul> >& a, S b) {
    
    typedef BinExprOp< ExprLiteral<T>, Array<d,T>, ApMul> ExprT;
#ifdef ARRAY_VERBOSE
    cout<<"1 Inside operator*(scalar*array, scalar), with scalar="<<b<<""<<endl;
    cout<<"  expression type: "<<typeid(ExprT).name()<<endl;
#endif
    return Expr<ExprT>(ExprT(ExprLiteral<T>(a.left()*b),a.right()));
}

// operator*(scalar, scalar*array)
template <int d, typename S, typename T>
typename enable_if<is_arithmetic<S>::value, Expr<BinExprOp< ExprLiteral<T>, Array<d,T>, ApMul> > >::type
operator*(S a, const Expr<BinExprOp< ExprLiteral<T>, Array<d,T>, ApMul> >& b) {
    
    typedef BinExprOp< ExprLiteral<T>, Array<d,T>, ApMul> ExprT;
#ifdef ARRAY_VERBOSE
    cout<<"1 Inside operator*(scalar, scalar*array), with scalar="<<a<<""<<endl;
    cout<<"  expression type: "<<typeid(ExprT).name()<<endl;
#endif
    return Expr<ExprT>(ExprT(ExprLiteral<T>(a*b.left()),b.right()));
}

// operator*(expr, array)
template<int d, typename T, class A>
Expr< BinExprOp<
Expr<A>, 
Expr< BinExprOp< ExprLiteral<T>, Array<d,T>, ApMul> >, 
ApMul> >
operator*(const Expr<A>& a, const Array<d,T>& b) {
    
    typedef BinExprOp<
    Expr<A>, 
    Expr< BinExprOp< ExprLiteral<T>, Array<d,T>, ApMul> >, 
    ApMul> ExprT;
#ifdef ARRAY_VERBOSE
    cout<<"1 Inside operator*(expr, array)"<<endl;
    cout<<"  expression type: "<<typeid(ExprT).name()<<endl;
#endif
    return Expr<ExprT>(ExprT(a, T(1)*b));
}

// operator*(array, expr)
template<int d, typename T, class B>
Expr< BinExprOp<
Expr< BinExprOp< ExprLiteral<T>, Array<d,T>, ApMul> >, 
Expr<B>,
ApMul> >
operator*(const Array<d,T>& a, const Expr<B>& b) {
    
    typedef BinExprOp<
    Expr< BinExprOp< ExprLiteral<T>, Array<d,T>, ApMul> >, 
    Expr<B>,
    ApMul> ExprT;
#ifdef ARRAY_VERBOSE
    cout<<"1 Inside operator*(array, expr)"<<endl;
    cout<<"  expression type: "<<typeid(ExprT).name()<<endl;
#endif
    return Expr<ExprT>(ExprT(T(1)*a, b));
}


// operator*(scalar, transposed object)
template <int d, typename S, typename T>
typename enable_if<is_arithmetic<S>::value, 
Expr<
BinExprOp< 
ExprLiteral<T>,
Expr<BinExprOp<Array<d,T>, EmptyType, ApTr> >,
ApMul> 
> 
>::type
operator*(S a, const Expr<BinExprOp<Array<d,T>, EmptyType, ApTr> >& b) {
        
    typedef BinExprOp< ExprLiteral<T>, Expr<BinExprOp<Array<d,T>, EmptyType, ApTr> >, ApMul> ExprT;
#ifdef ARRAY_VERBOSE
    cout<<"1 Inside operator*(scalar, transposed object), with scalar="<<a<<""<<endl;
    cout<<"  expression type: "<<typeid(ExprT).name()<<endl;
#endif
    return Expr<ExprT>(ExprT(ExprLiteral<T>(a),b));
}
// \todo // operator*(transposed object, scalar)


////////////////////////////////////////////////////////////////////////////////
// operator/

// operator/(expr, expr)
template<class A, class B>
Expr<BinExprOp<Expr<A>, Expr<B>, ApDiv> >
operator/(const Expr<A>& a, const Expr<B>& b) {
    
    typedef BinExprOp<Expr<A>, Expr<B>, ApDiv> ExprT;
#ifdef ARRAY_VERBOSE
    cout<<"1 Inside operator/(expr, expr)"<<endl;
    cout<<"  expression type: "<<typeid(ExprT).name()<<endl;
#endif
    return Expr<ExprT>(ExprT(a,b));
}


// operator/(array, scalar)
template <int d, typename S, typename T>
typename enable_if<is_arithmetic<S>::value, Expr<BinExprOp< ExprLiteral<T>, Array<d,T>, ApMul> > >::type
operator/(const Array<d,T>& a, S b) {
    
    typedef BinExprOp< ExprLiteral<T>, Array<d,T>, ApMul> ExprT;
#ifdef ARRAY_VERBOSE
    cout<<"1 Inside operator/(array, scalar), with scalar="<<b<<""<<endl;
    cout<<"  expression type: "<<typeid(ExprT).name()<<endl;
#endif
    return Expr<ExprT>(ExprT(ExprLiteral<T>(1/static_cast<T>(b)),a));
}


////////////////////////////////////////////////////////////////////////////////
// operator transpose

template <class A>
Expr< BinExprOp< ExprLiteral<typename A::value_type>, Expr<BinExprOp<A, EmptyType, ApTr> >, ApMul> > 
transpose(const A& a) {
    
    typedef typename A::value_type value_type;
    typedef BinExprOp<A, EmptyType, ApTr> TrExprT;
    typedef BinExprOp< ExprLiteral<value_type>, Expr< TrExprT>, ApMul> ExprT;
#ifdef ARRAY_VERBOSE
    cout<<"1 Inside transpose(any)"<<endl;
    cout<<"  expression type: "<<typeid(ExprT).name()<<endl;
#endif
    return Expr<ExprT>(ExprT(value_type(1), Expr<TrExprT>(TrExprT(a, EmptyType()))));
}

template <class A>
Expr< BinExprOp< ExprLiteral<typename A::value_type>, A, ApMul> >
transpose(const Expr< BinExprOp< ExprLiteral<typename A::value_type>, Expr<BinExprOp<A, EmptyType, ApTr> >, ApMul> >& a) {
    
    typedef typename A::value_type value_type;
    typedef BinExprOp< ExprLiteral<typename A::value_type>, A, ApMul> ExprT;

#ifdef ARRAY_VERBOSE
    cout<<"1 Inside transpose(scalar*transpose(any))"<<endl;
#endif
    
    return Expr<ExprT>(ExprT(a.left(), a.right().left()));
}

template <class A>
Expr <BinExprOp< ExprLiteral<typename A::value_type>, A , ApMul> >
transpose(const Expr< BinExprOp< ExprLiteral<typename A::value_type>, Expr< BinExprOp< Expr< BinExprOp< ExprLiteral<typename A::value_type>, A , ApMul> >, EmptyType, ApTr> >, ApMul> > & a) {
    
    typedef typename A::value_type value_type;

    typedef BinExprOp< ExprLiteral<typename A::value_type>, A , ApMul> ExprT;

#ifdef ARRAY_VERBOSE
    cout<<"1 Inside transpose(scalar*(scalar*transpose(any)))"<<endl;
#endif
    value_type s = static_cast<value_type>(a.left()) * static_cast<value_type>(a.right().left().left());
    
    return Expr<ExprT>(ExprT(s, a.right().left().right()));
}


////////////////////////////////////////////////////////////////////////////////
// operator+=


// operator+=(array, array)
template <int d, typename T>
Array<d,T>&
operator+=(Array<d,T>& a, const Array<d,T>& b) {
    
#ifdef ARRAY_VERBOSE
    cout<<"1 Inside operator+=(array, array)"<<endl;
#endif
    return a += T(1)*b;
}

// operator+=(any, any)
template <class A, class B>
typename enable_if<!is_arithmetic<A>::value && !is_arithmetic<B>::value, A& >::type
operator+=(A& a, const B& b) {
    typedef RefBinExprOp<A, B, ApAdd> ExprT;
#ifdef ARRAY_VERBOSE
    cout<<"1 Inside operator+=(any&, const any&)"<<endl;
    cout<<"  expression type: "<<typeid(ExprT).name()<<endl;
#endif
    return Expr<ExprT>(ExprT(a,b))();
}

////////////////////////////////////////////////////////////////////////////////
// operator-=


// operator-=(array, array)
template <int d, typename T>
Array<d,T>&
operator-=(Array<d,T>& a, const Array<d,T>& b) {
    
#ifdef ARRAY_VERBOSE
    cout<<"1 Inside operator-=(array, array)"<<endl;
#endif
    return a += T(-1)*b;
}

// operator-=(any, any)
template <class A, class B>
A& operator-=(A& a, const B& b) {
    
    typedef typename primitive<typename A::value_type, typename B::value_type>::result T;
    return a += T(-1)*b;
}



////////////////////////////////////////////////////////////////////////////////
// operator<<


template <class A>
inline std::ostream& print(std::ostream& os, const Expr<A>& e) {
    os<<e();
    return os;
}

template <>
inline std::ostream& print<BinExprOp< ExprLiteral<double>, Expr< BinExprOp< Array<1>, EmptyType, ApTr> >, ApMul> > (std::ostream& os, const Expr<BinExprOp< ExprLiteral<double>, Expr< BinExprOp< Array<1>, EmptyType, ApTr> >, ApMul> >& y) {
    
    const Array<1>& v = y.right().left();
    double s = static_cast<double>(y.left());
    
    for(size_t i=0; i<v.size(); ++i)
        os<<" "<<s*v[i];
    os<<endl;
    return os;
}

__END_ARRAY_NAMESPACE__


#endif /* ARRAY_EXPR_HPP */
