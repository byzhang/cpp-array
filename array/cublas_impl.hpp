/*
 * Copyright (©) 2014 Alejandro M. Aragón
 * Written by Alejandro M. Aragón <alejandro.aragon@fulbrightmail.org>
 * All Rights Reserved
 *
 * cpp-array is free  software: you can redistribute it and/or  modify it under
 * the terms  of the  GNU Lesser  General Public  License as  published by  the
 * Free Software Foundation, either version 3 of the License, or (at your option)
 * any later version.
 *
 * cpp-array is  distributed in the  hope that it  will be useful, but  WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A  PARTICULAR PURPOSE. See  the GNU  Lesser General  Public License  for
 * more details.
 *
 * You should  have received  a copy  of the GNU  Lesser General  Public License
 * along with cpp-array. If not, see <http://www.gnu.org/licenses/>.
 *
 */

/*! \file cublas_impl.hpp
 *
 * \brief This file defines the cublas interface.
 */

#ifndef CUBLAS_IMPL_HPP
#define CUBLAS_IMPL_HPP


#include <cuda_runtime.h>
#include <cublas_v2.h>
#include "cuda/helper_string.h"  // helper for shared functions common to CUDA SDK samples

static constexpr cublasOperation_t CblasNoTrans   = CUBLAS_OP_N;
static constexpr cublasOperation_t CblasTrans     = CUBLAS_OP_T;


__BEGIN_ARRAY_NAMESPACE__


using std::cout;
using std::endl;


class CUDA {
  
  int devID_;
  bool init_;
  
public:
  
  static CUDA& getInstance() {
    static CUDA instance;
    return instance;
  }
  
  int devID() const
  { return devID_; }
  
private:
  
  CUDA() : devID_(), init_() {}
  CUDA(CUDA const&) = delete;
  void operator=(CUDA const&) = delete;
  ~CUDA() { cudaDeviceReset(); }
  
public:
  
  bool initialized()
  { return init_; }
  
  void initialize(int argc, char **argv)
  {
    
    if (init_)
      return;
    
    // by default, we use device 0, otherwise we override the device ID based on what is provided at the command line
    cudaError_t error;
    
    if (checkCmdLineFlag(argc, (const char **)argv, "device")) {
      devID_ = getCmdLineArgumentInt(argc, (const char **)argv, "device");
      error = cudaSetDevice(devID_);
      
      if (error != cudaSuccess) {
        cout<<"cudaSetDevice returned error code "<<error<<", line "<<__LINE__<<endl;
        exit(EXIT_FAILURE);
      }
    }
    
    // get number of SMs on this GPU
    error = cudaGetDevice(&devID_);
    if (error != cudaSuccess) {
      cout<<"cudaGetDevice returned error code "<<error<<", line(%d)\n"<<__LINE__<<endl;
      exit(EXIT_FAILURE);
    }
    
    init_ = true;
  }
  
  int memory() {
    cudaDeviceProp props;
    cudaGetDeviceProperties(&props, devID_);
    return props.totalGlobalMem;
  }
  
  void info() const {
    cudaDeviceProp deviceProp;
    cudaError_t error = cudaGetDeviceProperties(&deviceProp, devID_);
    if (error != cudaSuccess) {
      cout<<"cudaGetDeviceProperties returned error code "<<error<<" , line "<<__LINE__<<endl;
      exit(EXIT_FAILURE);
    }
    
    cout<<"GPU Device "<<devID_<<": \""<<deviceProp.name<<"\" with compute capability "<<deviceProp.major<<"."<<deviceProp.minor<<endl;
  }
  
};



////////////////////////////////////////////////////////////////////////////////
// cublas functions



static cublasStatus_t cublasXscal(cublasHandle_t handle, int n, const float *alpha,
                                  float *x, int incx) {
  return cublasSscal(handle, n, alpha, x, incx);
}

static cublasStatus_t cublasXscal(cublasHandle_t handle, int n, const double *alpha,
                                  double *x, int incx) {
  return cublasDscal(handle, n, alpha, x, incx);
}


template <class T>
void cblas_scal(const int N, const T alpha, T *X, const int incX) {
  
  cudaError_t cudaStat;
  cublasStatus_t stat;
  cublasHandle_t handle;
  
  // make sure CUDA is initialized
  if (!CUDA::getInstance().initialized()) {
    cout<<"*** ERROR *** cuda not initialized"<<endl;
    cout<<"              Call array::CUDA::getInstance().initialize(argc, argv);"<<endl;
    exit(EXIT_FAILURE);
  }
  
  stat = cublasCreate(&handle);
  if (stat != CUBLAS_STATUS_SUCCESS) {
    cout<<"*** ERROR *** CUBLAS initialization failed"<<endl;
    exit(EXIT_FAILURE);
  }
  
  // allocate device memory
  T *d_X;
  cudaStat = cudaMalloc((void **) &d_X, N*sizeof(T));
  if (cudaStat != cudaSuccess) {
    cout<<"*** ERROR *** cudaMalloc d_X returned error code "<<cudaStat<<", line "<<__LINE__<<endl;
    exit(EXIT_FAILURE);
  }
  
  // copy host memory to device
  cudaStat = cudaMemcpy(d_X, X, N*sizeof(T), cudaMemcpyHostToDevice);
  if (cudaStat != cudaSuccess) {
    cout<<"*** ERROR *** cudaMemcpy d_X X returned error code "<<cudaStat<<", line "<<__LINE__<<endl;
    exit(EXIT_FAILURE);
  }
  
  stat = cublasXscal(handle, N, &alpha, d_X, incX);
  if (stat != CUBLAS_STATUS_SUCCESS) {
    cout<<"*** ERROR *** cublasXscal failed"<<endl;
    exit(EXIT_FAILURE);
  }
  
  // copy result from device to host
  cudaStat = cudaMemcpy(X, d_X, N*sizeof(T), cudaMemcpyDeviceToHost);
  if (cudaStat != cudaSuccess) {
    cout<<"*** ERROR *** cudaMemcpy X d_X returned error code "<<cudaStat<<", line "<<__LINE__<<endl;
    exit(EXIT_FAILURE);
  }
  
  cudaFree (d_X);
}






static cublasStatus_t cublasXaxpy(cublasHandle_t handle, int n, const float *alpha,
                                  const float *x, int incx, float *y, int incy) {
  return cublasSaxpy(handle, n, alpha, x, incx, y, incy);
}

static cublasStatus_t cublasXaxpy(cublasHandle_t handle, int n, const double *alpha,
                                  const double *x, int incx, double *y, int incy) {
  return cublasDaxpy(handle, n, alpha, x, incx, y, incy);
}


template <typename T>
static void cblas_axpy(const int N, const T alpha, const T *X,
                       const int incX, T *Y, const int incY)
{
  cudaError_t cudaStat;
  cublasStatus_t stat;
  cublasHandle_t handle;
  
  // make sure CUDA is initialized
  if (!CUDA::getInstance().initialized()) {
    cout<<"*** ERROR *** cuda not initialized"<<endl;
    cout<<"              Call array::CUDA::getInstance().initialize(argc, argv);"<<endl;
    exit(EXIT_FAILURE);
  }
  
  stat = cublasCreate(&handle);
  if (stat != CUBLAS_STATUS_SUCCESS) {
    cout<<"*** ERROR *** CUBLAS initialization failed"<<endl;
    exit(EXIT_FAILURE);
  }
  
  // allocate device memory
  T *d_X, *d_Y;
  cudaStat = cudaMalloc((void **) &d_X, N*sizeof(T));
  if (cudaStat != cudaSuccess) {
    cout<<"*** ERROR *** cudaMalloc d_X returned error code "<<cudaStat<<", line "<<__LINE__<<endl;
    exit(EXIT_FAILURE);
  }
  cudaStat = cudaMalloc((void **) &d_Y, N*sizeof(T));
  if (cudaStat != cudaSuccess) {
    cout<<"*** ERROR *** cudaMalloc d_Y returned error code "<<cudaStat<<", line "<<__LINE__<<endl;
    exit(EXIT_FAILURE);
  }
  
  // copy host memory to device
  stat = cublasSetVector(N, sizeof(T), X, incX, d_X, 1);
  if (cudaStat != cudaSuccess) {
    cout<<"*** ERROR *** cublasSetVector returned error code "<<stat<<", line "<<__LINE__<<endl;
    exit(EXIT_FAILURE);
  }
  
  stat = cublasSetVector(N, sizeof(T), Y, incY, d_Y, 1);
  if (cudaStat != cudaSuccess) {
    cout<<"*** ERROR *** cublasSetVector returned error code "<<stat<<", line "<<__LINE__<<endl;
    exit(EXIT_FAILURE);
  }
  
  stat = cublasXaxpy(handle, N, &alpha, d_X, 1, d_Y, 1);
  if (stat != CUBLAS_STATUS_SUCCESS) {
    cout<<"*** ERROR *** cublasXscal failed"<<endl;
    exit(EXIT_FAILURE);
  }
  
  // copy result from device to host
  stat = cublasGetVector(N, sizeof(T), d_Y, 1, Y, incY);
  if (cudaStat != cudaSuccess) {
    cout<<"*** ERROR *** cublasGetVector returned error code "<<stat<<", line "<<__LINE__<<endl;
    exit(EXIT_FAILURE);
  }
  
  cudaFree (d_X);
  cudaFree (d_Y);
}





static cublasStatus_t cublasXdot(cublasHandle_t handle, int n,
                                 const float *x, int incx,
                                 const float *y, int incy,
                                 float *result)
{  return cublasSdot(handle, n, x, incx, y, incy, result); }

static cublasStatus_t cublasXdot(cublasHandle_t handle, int n,
                                 const double *x, int incx,
                                 const double *y, int incy,
                                 double *result)
{  return cublasDdot(handle, n, x, incx, y, incy, result); }


template <typename T>
static T cblas_dot(const int N, const T *X, const int incX,
                   const T *Y, const int incY)
{
  cudaError_t cudaStat;
  cublasStatus_t stat;
  cublasHandle_t handle;
  
  // make sure CUDA is initialized
  if (!CUDA::getInstance().initialized()) {
    cout<<"*** ERROR *** cuda not initialized"<<endl;
    cout<<"              Call array::CUDA::getInstance().initialize(argc, argv);"<<endl;
    exit(EXIT_FAILURE);
  }
  
  stat = cublasCreate(&handle);
  if (stat != CUBLAS_STATUS_SUCCESS) {
    cout<<"*** ERROR *** CUBLAS initialization failed"<<endl;
    exit(EXIT_FAILURE);
  }
  
  // allocate device memory
  T *d_X, *d_Y;
  cudaStat = cudaMalloc((void **) &d_X, N*sizeof(T));
  if (cudaStat != cudaSuccess) {
    cout<<"*** ERROR *** cudaMalloc d_X returned error code "<<cudaStat<<", line "<<__LINE__<<endl;
    exit(EXIT_FAILURE);
  }
  cudaStat = cudaMalloc((void **) &d_Y, N*sizeof(T));
  if (cudaStat != cudaSuccess) {
    cout<<"*** ERROR *** cudaMalloc d_Y returned error code "<<cudaStat<<", line "<<__LINE__<<endl;
    exit(EXIT_FAILURE);
  }
  
  // copy host memory to device
  stat = cublasSetVector(N, sizeof(T), X, incX, d_X, 1);
  if (cudaStat != cudaSuccess) {
    cout<<"*** ERROR *** cublasSetVector returned error code "<<stat<<", line "<<__LINE__<<endl;
    exit(EXIT_FAILURE);
  }
  
  stat = cublasSetVector(N, sizeof(T), Y, incY, d_Y, 1);
  if (cudaStat != cudaSuccess) {
    cout<<"*** ERROR *** cublasSetVector returned error code "<<stat<<", line "<<__LINE__<<endl;
    exit(EXIT_FAILURE);
  }
  
  T r;
  
  stat = cublasXdot(handle, N, d_X, 1, d_Y, 1, &r);
  if (stat != CUBLAS_STATUS_SUCCESS) {
    cout<<"*** ERROR *** cublasXdot failed"<<endl;
    exit(EXIT_FAILURE);
  }
  
  cudaFree (d_X);
  cudaFree (d_Y);
  
  return r;
}



// level 2 blas xGER function: A <- alpha*x*y' + A

static cublasStatus_t cublasXger(cublasHandle_t handle, int m, int n,
                                 const float *alpha, const float *x, int incx,
                                 const float *y, int incy, float *A, int lda)
{ return cublasSger(handle, m, n, alpha, x, incx, y, incy, A, lda); }


static cublasStatus_t cublasXger(cublasHandle_t handle, int m, int n,
                                 const double *alpha, const double *x, int incx,
                                 const double *y, int incy, double *A, int lda)
{ return cublasDger(handle, m, n, alpha, x, incx, y, incy, A, lda); }


template <typename T>
static void cblas_ger(const int M, const int N,
                      const T alpha, const T *X, const int incX,
                      const T *Y, const int incY, T *A, const int lda)
{
  
  cudaError_t cudaStat;
  cublasStatus_t stat;
  cublasHandle_t handle;
  
  // make sure CUDA is initialized
  if (!CUDA::getInstance().initialized()) {
    cout<<"*** ERROR *** cuda not initialized"<<endl;
    cout<<"              Call array::CUDA::getInstance().initialize(argc, argv);"<<endl;
    exit(EXIT_FAILURE);
  }
  
  stat = cublasCreate(&handle);
  if (stat != CUBLAS_STATUS_SUCCESS) {
    cout<<"*** ERROR *** CUBLAS initialization failed"<<endl;
    exit(EXIT_FAILURE);
  }
  
  // allocate device memory
  T *d_A, *d_X, *d_Y;
  
  cudaStat = cudaMalloc((void**)&d_A, M*N*sizeof(T));
  if (cudaStat != cudaSuccess) {
    cout<<"*** ERROR *** cudaMalloc d_A returned error code "<<cudaStat<<", line "<<__LINE__<<endl;
  }
  
  stat = cublasSetMatrix(M, N, sizeof(T), A, M, d_A, M);
  if (stat != CUBLAS_STATUS_SUCCESS) {
    cout<<"*** ERROR *** cublasSetMatrix returned error code "<<stat<<", line "<<__LINE__<<endl;
    exit(EXIT_FAILURE);
  }
  
  cudaStat = cudaMalloc((void**)&d_X, M*sizeof(T));
  if (cudaStat != cudaSuccess) {
    cout<<"*** ERROR *** cudaMalloc d_X returned error code "<<cudaStat<<", line "<<__LINE__<<endl;
  }
  
  stat = cublasSetVector(M, sizeof(T), X, incX, d_X, 1);
  if (stat != CUBLAS_STATUS_SUCCESS) {
    cout<<"*** ERROR *** cublasSetVector returned error code "<<stat<<", line "<<__LINE__<<endl;
    exit(EXIT_FAILURE);
  }
  
  cudaStat = cudaMalloc((void **) &d_Y, sizeof(T)*N);
  if (cudaStat != cudaSuccess) {
    cout<<"*** ERROR *** cudaMalloc d_Y returned error code "<<cudaStat<<", line "<<__LINE__<<endl;
    exit(EXIT_FAILURE);
  }
  
  stat = cublasSetVector(N, sizeof(T), Y, incY, d_Y, 1);
  if (stat != CUBLAS_STATUS_SUCCESS) {
    cout<<"*** ERROR *** cublasSetVector returned error code "<<stat<<", line "<<__LINE__<<endl;
    exit(EXIT_FAILURE);
  }
  
  stat = cublasXger(handle, M, N, &alpha, d_X, 1, d_Y, 1, d_A, M);
  if (stat != CUBLAS_STATUS_SUCCESS) {
    cout<<"*** ERROR *** cublasXdot failed"<<endl;
    exit(EXIT_FAILURE);
  }
  
  // copy result from device to host
  stat = cublasGetMatrix(M,N, sizeof(T), d_A, M, A, M);
  if (cudaStat != cudaSuccess) {
    cout<<"*** ERROR *** cublasGetVector returned error code "<<stat<<", line "<<__LINE__<<endl;
    exit(EXIT_FAILURE);
  }
  
  cudaFree (d_A);
  cudaFree (d_X);
  cudaFree (d_Y);
}





// level 2 blas xGEMV function: Y <- alpha*A*x + beta*y
static cublasStatus_t cublasXgemv(cublasHandle_t handle, cublasOperation_t trans,
                                  int m, int n, const float *alpha, const float *A, int lda,
                                  const float *x, int incx,
                                  const float *beta, float *y, int incy)
{
  return cublasSgemv(handle, trans, m, n, alpha, A, lda, x, incx, beta, y, incy);
}

static cublasStatus_t cublasXgemv(cublasHandle_t handle, cublasOperation_t trans,
                                  int m, int n, const double *alpha, const double *A, int lda,
                                  const double *x, int incx,
                                  const double *beta, double *y, int incy)
{
  return cublasDgemv(handle, trans, m, n, alpha, A, lda, x, incx, beta, y, incy);
}


template <typename T>
static void cblas_gemv(cublasOperation_t trans, int m, int n, T alpha,
                       const T *A, int lda, const T *x, int incx,
                       T beta, T *y, int incy)
{
  
  cudaError_t cudaStat;
  cublasStatus_t stat;
  cublasHandle_t handle;
  
  // make sure CUDA is initialized
  if (!CUDA::getInstance().initialized()) {
    cout<<"*** ERROR *** cuda not initialized"<<endl;
    cout<<"              Call array::CUDA::getInstance().initialize(argc, argv);"<<endl;
    exit(EXIT_FAILURE);
  }
  
  stat = cublasCreate(&handle);
  if (stat != CUBLAS_STATUS_SUCCESS) {
    cout<<"*** ERROR *** CUBLAS initialization failed"<<endl;
    exit(EXIT_FAILURE);
  }
  
  
  // allocate device memory
  T *d_A, *d_X, *d_Y;
  
  cudaStat = cudaMalloc((void**)&d_A, m*n*sizeof(T));
  if (cudaStat != cudaSuccess) {
    cout<<"*** ERROR *** cudaMalloc d_A returned error code "<<cudaStat<<", line "<<__LINE__<<endl;
  }
  
  stat = cublasSetMatrix(m, n, sizeof(T), A, m, d_A, m);
  if (stat != CUBLAS_STATUS_SUCCESS) {
    cout<<"*** ERROR *** cublasSetMatrix returned error code "<<stat<<", line "<<__LINE__<<endl;
    exit(EXIT_FAILURE);
  }
  
  cudaStat = cudaMalloc((void**)&d_X, n*sizeof(T));
  if (cudaStat != cudaSuccess) {
    cout<<"*** ERROR *** cudaMalloc d_X returned error code "<<cudaStat<<", line "<<__LINE__<<endl;
  }
  
  stat = cublasSetVector(n, sizeof(T), x, incx, d_X, 1);
  if (stat != CUBLAS_STATUS_SUCCESS) {
    cout<<"*** ERROR *** cublasSetVector returned error code "<<stat<<", line "<<__LINE__<<endl;
    exit(EXIT_FAILURE);
  }
  
  cudaStat = cudaMalloc((void **) &d_Y, sizeof(T)*n);
  if (cudaStat != cudaSuccess) {
    cout<<"*** ERROR *** cudaMalloc d_Y returned error code "<<cudaStat<<", line "<<__LINE__<<endl;
    exit(EXIT_FAILURE);
  }
  
  stat = cublasXgemv(handle, trans, m, n, &alpha, d_A, m, d_X, 1, &beta, d_Y, 1);
  if (stat != CUBLAS_STATUS_SUCCESS) {
    cout<<"*** ERROR *** cublasXdot failed"<<endl;
    exit(EXIT_FAILURE);
  }
  
  // copy result from device to host
  cudaStat = cudaMemcpy(y, d_Y, sizeof(T)*n, cudaMemcpyDeviceToHost);
  if (cudaStat != cudaSuccess) {
    cout<<"*** ERROR *** cudaMemcpy y d_Y returned error code "<<cudaStat<<", line "<<__LINE__<<endl;
    exit(EXIT_FAILURE);
  }
  
  cudaFree (d_A);
  cudaFree (d_X);
  cudaFree (d_Y);
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

static cublasStatus_t cublasXgemm(cublasHandle_t& handle, cublasOperation_t transa, cublasOperation_t transb, int m, int n, int k,
                                  float alpha, const float *A, int lda,
                                  const float *B, int ldb, float beta, float *C,
                                  int ldc) {
  return cublasSgemm(handle, transa, transb, m, n, k, &alpha, A, lda, B, ldb, &beta, C, m);
}

static cublasStatus_t cublasXgemm(cublasHandle_t& handle, cublasOperation_t transa, cublasOperation_t transb, int m, int n, int k,
                                  double alpha, const double *A, int lda,
                                  const double *B, int ldb, double beta, double *C,
                                  int ldc) {
  return cublasDgemm(handle, transa, transb, m, n, k, &alpha, A, lda, B, ldb, &beta, C, m);
}


template <class T>
void cblas_gemm(cublasOperation_t transa, cublasOperation_t transb, int m, int n, int k,
                T alpha, const T *A, int lda, const T *B, int ldb, T beta, T *C, int ldc) {
  
  cudaDeviceProp deviceProp;
  cudaError_t error;
  
  // make sure CUDA is initialized
  if (!CUDA::getInstance().initialized()) {
    cout<<"*** ERROR *** cuda not initialized"<<endl;
    cout<<"              Call array::CUDA::getInstance().initialize(argc, argv);"<<endl;
    exit(EXIT_FAILURE);
  }
  
  int devID = CUDA::getInstance().devID();
  
  error = cudaGetDeviceProperties(&deviceProp, devID);
  
  if (error != cudaSuccess) {
    cout<<"*** ERROR *** cudaGetDeviceProperties returned error code "<<error<<", line "<<__LINE__<<endl;
    exit(EXIT_FAILURE);
  }
  
  // use a larger block size for Fermi and above
  int block_size = (deviceProp.major < 2) ? 16 : 32;
  
  // allocate device memory
  T *d_A, *d_B, *d_C;
  
  unsigned int mem_size_A = sizeof(T)*m*k;
  error = cudaMalloc((void **) &d_A, mem_size_A);
  if (error != cudaSuccess) {
    cout<<"*** ERROR *** cudaMalloc d_A returned error code "<<error<<", line "<<__LINE__<<endl;
    exit(EXIT_FAILURE);
  }
  
  unsigned int mem_size_B = sizeof(T)*ldb*n;
  error = cudaMalloc((void **) &d_B, mem_size_B);
  if (error != cudaSuccess) {
    cout<<"*** ERROR *** cudaMalloc d_B returned error code "<<error<<", line "<<__LINE__<<endl;
    exit(EXIT_FAILURE);
  }
  
  // copy host memory to device
  error = cudaMemcpy(d_A, A, mem_size_A, cudaMemcpyHostToDevice);
  if (error != cudaSuccess) {
    cout<<"*** ERROR *** cudaMemcpy d_A A returned error code "<<error<<", line "<<__LINE__<<endl;
    exit(EXIT_FAILURE);
  }
  
  error = cudaMemcpy(d_B, B, mem_size_B, cudaMemcpyHostToDevice);
  if (error != cudaSuccess) {
    cout<<"*** ERROR *** cudaMemcpy d_B B returned error code "<<error<<", line "<<__LINE__<<endl;
    exit(EXIT_FAILURE);
  }
  
  unsigned int mem_size_C = sizeof(T)*m*n;
  error = cudaMalloc((void **) &d_C, mem_size_C);
  if (error != cudaSuccess) {
    cout<<"*** ERROR *** cudaMalloc d_C returned error code "<<error<<", line "<<__LINE__<<endl;
    exit(EXIT_FAILURE);
  }
  
  error = cudaMemcpy(d_C, C, mem_size_C, cudaMemcpyHostToDevice);
  if (error != cudaSuccess) {
    cout<<"*** ERROR *** cudaMemcpy d_B B returned error code "<<error<<", line "<<__LINE__<<endl;
    exit(EXIT_FAILURE);
  }
  
  // setup execution parameters
  dim3 threads(block_size, block_size);
  dim3 grid(n / threads.x, m / threads.y);
  
  // CUBLAS version 2.0
  {
    cublasHandle_t handle;
    
    cublasStatus_t ret = cublasCreate(&handle);
    
    if (ret != CUBLAS_STATUS_SUCCESS) {
      cout<<"*** ERROR *** cublasCreate returned error code "<<ret<<", line "<<__LINE__<<endl;
      exit(EXIT_FAILURE);
    }
    
    ret = cublasXgemm(handle, transa, transb, m, n, k, alpha, d_A, lda, d_B, ldb, beta, d_C, m);
    if (ret != CUBLAS_STATUS_SUCCESS) {
      cout<<"*** ERROR *** cublasSgemm returned error code "<<ret<<", line "<<__LINE__<<endl;
      exit(EXIT_FAILURE);
    }
    
    // copy result from device to host
    error = cudaMemcpy(C, d_C, mem_size_C, cudaMemcpyDeviceToHost);
    if (error != cudaSuccess) {
      cout<<"*** ERROR *** cudaMemcpy h_CUBLAS d_C returned error code "<<error<<", line "<<__LINE__<<endl;
      exit(EXIT_FAILURE);
    }
  }
  
  cudaFree(d_A);
  cudaFree(d_B);
  cudaFree(d_C);
}

__END_ARRAY_NAMESPACE__


#endif /* CUBLAS_IMPL_HPP */
