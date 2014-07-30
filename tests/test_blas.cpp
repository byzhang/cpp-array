/*
 * Copyright (©) 2014 Alejandro M. Aragón
 * Written by Alejandro M. Aragón <alejandro.aragon@fulbrightmail.org>
 * All Rights Reserved
 *
 * cpp-array is free  software: you can redistribute it and/or  modify it under
 * the terms  of the  GNU Lesser  General Public  License as  published by  the
 * Free Software Foundation, either version 3 of the License, or (at your
 * option) any later version.
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

/*! \file test_operations.cpp
 *
 * \brief This function tests some operations with the algebraic objects.
 */

#include "array.hpp"

using std::cout;
using std::endl;

int main(int argc, char **argv) {

#ifdef HAVE_CUBLAS_H
  array::CUDA::getInstance().initialize(argc, argv);
#endif

  typedef array::vector_type<double> vector_type;
  typedef array::matrix_type<double> matrix_type;
  typedef array::tensor_type<double> tensor_type;

  size_t m = 3, n = 3;

  vector_type X(m), Y(m);

  for (size_t i = 0; i < m; ++i) {
    X(i) = i;
    Y(i) = m - i;
  }

  cout << "X -> " << X << endl;
  cout << "Y -> " << Y << endl;

  cout << "X + Y = " << (X + Y) << endl;
  cout << "X - Y = " << (X - Y) << endl;

  cout << "2.*X = " << (2. * X) << endl;
  cout << "(2.*X)+Y = " << ((2. * X) + Y) << endl;
  cout << "X+(2*Y) = " << (X + (2 * Y)) << endl;
  cout << "(X+(2*Y))+Y = " << ((X + (2 * Y)) + Y) << endl;
  cout << "Y+(X+(2*Y)) = " << (Y + (X + (2 * Y))) << endl;

  cout << "(2.*X)+(3*Y) = " << ((2. * X) + (3 * Y)) << endl;

  cout << "transpose(X)* Y / (transpose(X)*X) = " << (transpose(X)* Y / (transpose(X)*X)) << endl;

  cout << "(transpose(X) * Y) = " << (transpose(X) * Y) << endl;
  cout << "(X * transpose(Y)) = " << (X * transpose(Y)) << endl;
  cout << "((2. * X) * transpose(Y)) = " << ((2. * X) * transpose(Y)) << endl;
  cout << "(X * (2 * transpose(Y))) = " << (X * (2 * transpose(Y))) << endl;

  cout << "(X * (2. * (2. * transpose(Y)))) = "
       << (X * (2. * (2. * transpose(Y)))) << endl;
  cout << "(X * ((transpose(Y) * 2.) * 2.)) = "
       << (X * ((transpose(Y) * 2.) * 2.)) << endl;

  matrix_type A(m, m);
  matrix_type B(m, m);
  matrix_type D(m, 1);
  matrix_type E(1, m);

  int k = 0;
  int l = -1;
  for (size_t i = 0; i < m; ++i) {
    D(i, 0UL) = k;
    E(0UL, i) = l;
    for (size_t j = 0; j < m; ++j) {
      A(i, j) = ++k;
      B(i, j) = --l;
    }
  }

  cout << " A " << A << endl;
  matrix_type Atr = transpose(A);
  cout << " Atr = A' = " << Atr << endl;

  // matrix addition
  cout << "A+A: " << (A + A) << endl;
  cout << "A+(2.*A): " << (A + (2. * A)) << endl;
  cout << "(2.*A)+A: " << ((2. * A) + A) << endl;
  cout << "(2.*A)+(2.*A): " << ((2. * A) + (2. * A)) << endl;

  // matrix subtraction
  cout << "A-A: " << (A - A) << endl;
  cout << "A-(2.*A): " << (A - (2. * A)) << endl;
  cout << "(2.*A)-A: " << ((2. * A) - A) << endl;
  cout << "(2.*A)-(2.*A): " << ((2. * A) - (2. * A)) << endl;

  // matrix multiplication
  cout << "A*A: " << (A * A) << endl;
  cout << "A*(2.*A): " << (A * (2. * A)) << endl;
  cout << "(2.*A)*A: " << ((2. * A) * A) << endl;
  cout << "(2.*A)*(2.*A): " << ((2. * A) * (2. * A)) << endl;

  // matrix - vector multiplication
  cout << "A*X: " << (A * X) << endl;
  cout << "A*(2.*X): " << (A * (2. * X)) << endl;
  cout << "(2.*A)*X): " << ((2. * A) * X) << endl;
  cout << "(2.*A)*(3.*X): " << (2. * A) * (3. * X) << endl;

  cout << " E " << E << endl;
  cout << "2.*X*E: " << (2. * X * E) << endl;

  // support for reference operators
  matrix_type C = A;

  cout << "A: " << A << endl;
  cout << "B: " << B << endl;
  cout << "C: " << C << endl;
  C += A;
  cout << "C += A = " << C << endl;
  C += 2. * A;
  cout << "C += 2.*A = " << C << endl;
  C += 2. * A * B;
  cout << "C += 2.*A*B = " << C << endl;

  C -= 2. * A * B;
  cout << "C -= 2.*A*B = " << C << endl;
  C -= 2. * A;
  cout << "C -= 2*A = " << C << endl;
  C -= A;
  cout << "C -= A = " << C << endl;

  // support for transposed objects

  m = 5;
  n = 3;
  A = matrix_type(m, n);
  B = matrix_type(n, m);
  k = 0;
  l = -1;
  for (size_t i = 0; i < m; ++i)
    for (size_t j = 0; j < n; ++j) {
      A(i, j) = ++k;
      B(j, i) = --l;
    }

  cout << "A: " << A << endl;
  cout << "B: " << B << endl;
  cout << "C: " << C << endl;

  cout << "transpose(B)*C: " << (transpose(B) * C) << endl;
  cout << "(2.*transpose(B))*C: " << (2. * transpose(B) * C) << endl;
  cout << "transpose(B)*(2.*C): " << (transpose(B) * (2. * C)) << endl;
  cout << "(4.*transpose(B))*(0.5*C): " << ((4. * transpose(B)) * (0.5 * C))
       << endl;

  cout << "C*transpose(A): " << (C * transpose(A)) << endl;
  cout << "(2.*C)*transpose(A): " << ((2. * C) * transpose(A)) << endl;
  cout << "C*(2.*transpose(A)): " << (C * (2. * transpose(A))) << endl;
  cout << "(4.*C)*(0.5*transpose(A)): " << ((4. * C) * (0.5 * transpose(A)))
       << endl;

  cout << "transpose(B)*transpose(A): " << (transpose(B) * transpose(A))
       << endl;
  cout << "(2.*transpose(B))*transpose(A): "
       << ((2. * transpose(B)) * transpose(A)) << endl;
  cout << "transpose(B)*(2.*transpose(A)): "
       << (transpose(B) * (2. * transpose(A))) << endl;
  cout << "(4.*transpose(B))*(0.5*transpose(A)): "
       << ((4. * transpose(B)) * (0.5 * transpose(A))) << endl;

  cout << " D " << D << endl;
  cout << "2.*D*transpose(X): " << (2. * D * transpose(X)) << endl;

  return 0;
}
