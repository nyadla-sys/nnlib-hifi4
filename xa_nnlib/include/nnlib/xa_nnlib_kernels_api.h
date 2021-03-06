/*******************************************************************************
* Copyright (c) 2018-2020 Cadence Design Systems, Inc.
* 
* Permission is hereby granted, free of charge, to any person obtaining
* a copy of this software and associated documentation files (the
* "Software"), to use this Software with Cadence processor cores only and 
* not with any other processors and platforms, subject to
* the following conditions:
* 
* The above copyright notice and this permission notice shall be included
* in all copies or substantial portions of the Software.
* 
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
* IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
* CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
* TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
* SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

******************************************************************************/
#ifndef __XA_NNLIB_KERNELS_API_H__
#define __XA_NNLIB_KERNELS_API_H__

/**
 * @file xa_nnlib_kernels_api.h
 * @brief This file gives the API definition for the HiFi NNLIB
 *
 * matXvec KERNELS API NAMING CONVENTION <br>
 * <br>
 * xa_nn_matXvec_<batch>_[m]x[n]_[p]_<activation>, where
 * - <batch>: Optional 'batch' tag to indicate time batching routine
 * - [m]: Matrix precision in bits
 * - [n]: Vector (and bias for non-activation routines) precision in bits
 * - [p]: Output precision in bits
 * - <activation>: optional activation tag 'sigmoid' / 'tanh'
 *
 * These set of kernels perform dual matXvec followed by optional
 * activation function. There are several variants based on the input,
 * output precision and use of activation functions.
 *
 * Restriction,
 * - All pointers (p_out, p_mat1, p_mat2, p_vec1, p_vec2, p_bias, p_scratch) must
 * be SIMD (64-bit) aligned and should not overlap.
 * - p_mat2, p_vec2 can be 'NULL', but other pointers cannot be 'NULL'
 * - Variables cols1, cols2, row_stride1, row_stride2 must be multiple of 4
 *
 * Usage of few critical variables,
 * - acc_shift:
 *   -# In case of valid activation tag i.e. <activation>: shift to be
 *   applied on accumulator to match accumulator's Q format with activation
 *   function's input's Q format
 *   -# In case of bypass i.e. no activation tag: shift to be applied on
 *   accumulator.
 *   -# Positive value denotes left shift, and negative value denotes right shift.
 * - bias_shift: shift which is to be applied on bias to match bias's
 *   Q format with accumulator's Q format. Positive value denotes left shift,
 *   and negative value denotes right shift.
 * - bias_precision: This represents bias precision
 *   -# For 16x16, and 8x16 apis, valid values are '16' and '64'
 *   -# For 8x8 apis, valid values are '8' and '32'
 *
 * Output 8b, 16b, 32b of fixed point apis (only for bypass variants) is
 * extracted from 64b accumulator with symmetric rounding. Output 64b of fixed
 * point apis (only for bypass variants) is extracted from 64b accumulator.
 * Output 8b, 16b of fixed point apis (only for activation variants) is 
 * symmetrically rounded.
 *
 * matXvec 16x16 Kernels,
 * - Bypass kernels with 16, 32, 64 bit output: 3
 * - Fused kernel with 2 activation variants:   2
 * - Time batching kernel:                      1 (Not implemented)
 * - Total:                                     6
 *
 * matXvec 8x16 Kernels,
 * - Bypass kernels with 16, 32, 64 bit output: 3
 * - Fused kernel with 2 activation variants:   2
 * - Time batching kernel:                      1 (Not implemented)
 * - Total:                                     6
 *
 * matXvec 8x8 Kernels,
 * - Bypass kernels with 8, 16, 32 bit output: 3
 * - Fused kernel with 2 activation variants:  2
 * - Time batching kernel:                     1 (Not implemented)
 * - Total:                                    6
 *
 * matXvec float32 x float32 Kernels,
 * - Bypass kernels 32 bit output:            1
 * - Fused kernel with 2 activation variants: 2
 * - Time batching kernel:                    1 (Not implemented)
 * - Total:                                   4
 *
 * ACTIVATION KERNELS API NAMING CONVENTION <br>
 * <br>
 * xa_nn_vec_[activation]_[n]_[p] for fixed point <br>
 * xa_nn_vec_[activation]_f32_f32 for floating point, where
 * - [activation]: One of activations - sigmoid/tanh/relu/relu1/relu6/softmax
 * - [n]:          Input precision in bits
 * - [p]:          Output precision in bits
 *
 * Possible values,
 * - 'n' takes value '32', and expects input in Q6.25 format.
 * - 'p' takes values '32' and '16', gives output in Q16.15 and Q0.15 formats
 * respectively.
 *
 * There is WORD32 datatype variable 'threshold' for 'relu' related apis, which
 * expects value in Q16.15 format.
 *
 * Restriction,
 * - All pointers (p_out, p_vec) must be 32-bit aligned and should not overlap.
 *
 * activation 32_32 kernels,
 * - Vector activation kernels: 6
 * - Total:                     6
 *
 * activation f32_f32 kernels,
 * - Vector activation kernels: 6
 * - Total:                     6
 *
 * activation 32_16 kernels,
 * - Vector activation kernels: 2
 * - Total:                     2
 */

#if defined(__cplusplus)
extern "C"
{
#endif

WORD32 xa_nn_matXvec_16x16_16(
         WORD16 * __restrict__ p_out,                /*!< [out] 16b result: rows x 1 */
         WORD16 * __restrict__ p_mat1,               /*!< [in] 16b mat1: rows x cols1 */
         WORD16 * __restrict__ p_mat2,               /*!< [in] 16b mat2: rows x cols2 */
         WORD16 * __restrict__ p_vec1,               /*!< [in] 16b vec1: cols1 x 1 */
         WORD16 * __restrict__ p_vec2,               /*!< [in] 16b vec2: cols2 x 1 */
         WORD16 * __restrict__ p_bias,               /*!< [in] 16b bias: rows x 1 */
         WORD32 rows,                                /*!< [in] number of rows */
         WORD32 cols1,                               /*!< [in] number of columns of mat1 */
         WORD32 cols2,                               /*!< [in] number of columns of mat2 */
         WORD32 row_stride1,                         /*!< [in] row stride for mat1 */
         WORD32 row_stride2,                         /*!< [in] row stride for mat2 */
         WORD32 acc_shift,                             /*!< [in] out accumulator left shift amount */
         WORD32 bias_shift                             /*!< [in] bias left shift amount */
  );

WORD32 xa_nn_matXvec_16x16_32(
         WORD32 * __restrict__ p_out,                /*!< [out] 32b result: rows x 1 */
         WORD16 * __restrict__ p_mat1,               /*!< [in] 16b mat1: rows x cols1 */
         WORD16 * __restrict__ p_mat2,               /*!< [in] 16b mat2: rows x cols2 */
         WORD16 * __restrict__ p_vec1,               /*!< [in] 16b vec1: cols1 x 1 */
         WORD16 * __restrict__ p_vec2,               /*!< [in] 16b vec2: cols2 x 1 */
         WORD16 * __restrict__ p_bias,               /*!< [in] 16b bias: rows x 1 */
         WORD32 rows,                                /*!< [in] number of rows */
         WORD32 cols1,                               /*!< [in] number of columns of mat1 */
         WORD32 cols2,                               /*!< [in] number of columns of mat2 */
         WORD32 row_stride1,                         /*!< [in] row stride for mat1 */
         WORD32 row_stride2,                         /*!< [in] row stride for mat2 */
         WORD32 acc_shift,                             /*!< [in] out accumulator left shift amount */
         WORD32 bias_shift                             /*!< [in] bias left shift amount */
  );

WORD32 xa_nn_matXvec_16x16_64(
         WORD64 * __restrict__ p_out,                /*!< [out] 64b result: rows x 1 */
         WORD16 * __restrict__ p_mat1,               /*!< [in] 16b mat1: rows x cols1 */
         WORD16 * __restrict__ p_mat2,               /*!< [in] 16b mat2: rows x cols2 */
         WORD16 * __restrict__ p_vec1,               /*!< [in] 16b vec1: cols1 x 1 */
         WORD16 * __restrict__ p_vec2,               /*!< [in] 16b vec2: cols2 x 1 */
         WORD16 * __restrict__ p_bias,               /*!< [in] 16b bias: rows x 1 */
         WORD32 rows,                                /*!< [in] number of rows */
         WORD32 cols1,                               /*!< [in] number of columns of mat1 */
         WORD32 cols2,                               /*!< [in] number of columns of mat2 */
         WORD32 row_stride1,                         /*!< [in] row stride for mat1 */
         WORD32 row_stride2,                         /*!< [in] row stride for mat2 */
         WORD32 acc_shift,                             /*!< [in] out accumulator left shift amount */
         WORD32 bias_shift                             /*!< [in] bias left shift amount */
  );

WORD32 xa_nn_matXvec_16x16_16_tanh(
         WORD16 * __restrict__ p_out,                /*!< [out] 16b result: rows x 1 */
         WORD16 * __restrict__ p_mat1,               /*!< [in] 16b mat1: rows x cols1 */
         WORD16 * __restrict__ p_mat2,               /*!< [in] 16b mat2: rows x cols2 */
         WORD16 * __restrict__ p_vec1,               /*!< [in] 16b vec1: cols1 x 1 */
         WORD16 * __restrict__ p_vec2,               /*!< [in] 16b vec2: cols2 x 1 */
         VOID   * __restrict__ p_bias,               /*!< [in] bias: rows x 1 */
         WORD32 rows,                                /*!< [in] number of rows */
         WORD32 cols1,                               /*!< [in] number of columns of mat1 */
         WORD32 cols2,                               /*!< [in] number of columns of mat2 */
         WORD32 row_stride1,                         /*!< [in] row stride for mat1 */
         WORD32 row_stride2,                         /*!< [in] row stride for mat2 */
         WORD32 acc_shift,                             /*!< [in] out accumulator left shift amount */
         WORD32 bias_shift,                            /*!< [in] bias left shift amount */
         WORD32 bias_precision,                      /*!< [in] bias precision */
         VOID   * __restrict__ p_scratch             /*!< [in,out] scratch: rows x 4 bytes */
  );

WORD32 xa_nn_matXvec_16x16_16_sigmoid(

         WORD16 * __restrict__ p_out,                /*!< [out] 16b result: rows x 1 */
         WORD16 * __restrict__ p_mat1,               /*!< [in] 16b mat1: rows x cols1 */
         WORD16 * __restrict__ p_mat2,               /*!< [in] 16b mat2: rows x cols2 */
         WORD16 * __restrict__ p_vec1,               /*!< [in] 16b vec1: cols1 x 1 */
         WORD16 * __restrict__ p_vec2,               /*!< [in] 16b vec2: cols2 x 1 */
         VOID   * __restrict__ p_bias,               /*!< [in] bias: rows x 1 */
         WORD32 rows,                                /*!< [in] number of rows */
         WORD32 cols1,                               /*!< [in] number of columns of mat1 */
         WORD32 cols2,                               /*!< [in] number of columns of mat2 */
         WORD32 row_stride1,                         /*!< [in] row stride for mat1 */
         WORD32 row_stride2,                         /*!< [in] row stride for mat2 */
         WORD32 acc_shift,                             /*!< [in] out accumulator left shift amount */
         WORD32 bias_shift,                            /*!< [in] bias left shift amount */
         WORD32 bias_precision,                      /*!< [in] bias precision */
         VOID   * __restrict__ p_scratch             /*!< [in,out] scratch: rows x 4 bytes */
  );
         
WORD32 xa_nn_matXvec_batch_16x16_64(
         WORD64 ** __restrict__ p_out,               /*!< [out] array of result: rows x 1 pointers */
         WORD16 *  __restrict__ p_mat1,              /*!< [in] 16b mat1: rows x cols1 */
         WORD16 ** __restrict__ p_vec1,              /*!< [in] 16b vec1: cols1 x 1 */
         WORD16 *  __restrict__ p_bias,              /*!< [in] 16b bias TBD: Need array? */
         WORD32 rows,                                /*!< [in] number of rows */
         WORD32 cols1,                               /*!< [in] number of columns of mat1 */
         WORD32 row_stride1,                         /*!< [in] row stride for mat1 */
         WORD32 acc_shift,                             /*!< [in] out accumulator left shift amount */
         WORD32 bias_shift,                            /*!< [in] bias left shift amount */
         WORD32 vec_count                            /*!< [in] number of vectors: 2, 4, 2n */
  );

WORD32 xa_nn_matXvec_8x16_16(
         WORD16 * __restrict__ p_out,                /*!< [out] 16b result: rows x 1 */
         WORD8  * __restrict__ p_mat1,               /*!< [in] 8b mat1: rows x cols1 */
         WORD8  * __restrict__ p_mat2,               /*!< [in] 8b mat2: rows x cols2 */
         WORD16 * __restrict__ p_vec1,               /*!< [in] 16b vec1: cols1 x 1 */
         WORD16 * __restrict__ p_vec2,               /*!< [in] 16b vec2: cols2 x 1 */
         WORD16 * __restrict__ p_bias,               /*!< [in] 16b bias: rows x 1 */
         WORD32 rows,                                /*!< [in] number of rows */
         WORD32 cols1,                               /*!< [in] number of columns of mat1 */
         WORD32 cols2,                               /*!< [in] number of columns of mat2 */
         WORD32 row_stride1,                         /*!< [in] row stride for mat1 */
         WORD32 row_stride2,                         /*!< [in] row stride for mat2 */
         WORD32 acc_shift,                             /*!< [in] out accumulator left shift amount */
         WORD32 bias_shift                             /*!< [in] bias left shift amount */
  );

WORD32 xa_nn_matXvec_8x16_32(
         WORD32 * __restrict__ p_out,                /*!< [out] 32b result: rows x 1 */
         WORD8  * __restrict__ p_mat1,               /*!< [in] 8b mat1: rows x cols1 */
         WORD8  * __restrict__ p_mat2,               /*!< [in] 8b mat2: rows x cols2 */
         WORD16 * __restrict__ p_vec1,               /*!< [in] 16b vec1: cols1 x 1 */
         WORD16 * __restrict__ p_vec2,               /*!< [in] 16b vec2: cols2 x 1 */
         WORD16 * __restrict__ p_bias,               /*!< [in] 16b bias: rows x 1 */
         WORD32 rows,                                /*!< [in] number of rows */
         WORD32 cols1,                               /*!< [in] number of columns of mat1 */
         WORD32 cols2,                               /*!< [in] number of columns of mat2 */
         WORD32 row_stride1,                         /*!< [in] row stride for mat1 */
         WORD32 row_stride2,                         /*!< [in] row stride for mat2 */
         WORD32 acc_shift,                             /*!< [in] out accumulator left shift amount */
         WORD32 bias_shift                             /*!< [in] bias left shift amount */
  );

WORD32 xa_nn_matXvec_8x16_64(
         WORD64 * __restrict__ p_out,                /*!< [out] 64b result: rows x 1 */
         WORD8  * __restrict__ p_mat1,               /*!< [in] 8b mat1: rows x cols1 */
         WORD8  * __restrict__ p_mat2,               /*!< [in] 8b mat2: rows x cols2 */
         WORD16 * __restrict__ p_vec1,               /*!< [in] 16b vec1: cols1 x 1 */
         WORD16 * __restrict__ p_vec2,               /*!< [in] 16b vec2: cols2 x 1 */
         WORD16 * __restrict__ p_bias,               /*!< [in] 16b bias: rows x 1 */
         WORD32 rows,                                /*!< [in] number of rows */
         WORD32 cols1,                               /*!< [in] number of columns of mat1 */
         WORD32 cols2,                               /*!< [in] number of columns of mat2 */
         WORD32 row_stride1,                         /*!< [in] row stride for mat1 */
         WORD32 row_stride2,                         /*!< [in] row stride for mat2 */
         WORD32 acc_shift,                             /*!< [in] out accumulator left shift amount */
         WORD32 bias_shift                             /*!< [in] bias left shift amount */
  );

WORD32 xa_nn_matXvec_8x16_16_tanh(
         WORD16 * __restrict__ p_out,                /*!< [out] 16b result: rows x 1 */
         WORD8  * __restrict__ p_mat1,               /*!< [in] 8b mat1: rows x cols1 */
         WORD8  * __restrict__ p_mat2,               /*!< [in] 8b mat2: rows x cols2 */
         WORD16 * __restrict__ p_vec1,               /*!< [in] 16b vec1: cols1 x 1 */
         WORD16 * __restrict__ p_vec2,               /*!< [in] 16b vec2: cols2 x 1 */
         VOID   * __restrict__ p_bias,               /*!< [in] bias: rows x 1 */
         WORD32 rows,                                /*!< [in] number of rows */
         WORD32 cols1,                               /*!< [in] number of columns of mat1 */
         WORD32 cols2,                               /*!< [in] number of columns of mat2 */
         WORD32 row_stride1,                         /*!< [in] row stride for mat1 */
         WORD32 row_stride2,                         /*!< [in] row stride for mat2 */
         WORD32 acc_shift,                             /*!< [in] out accumulator left shift amount */
         WORD32 bias_shift,                            /*!< [in] bias left shift amount */
         WORD32 bias_precision,                      /*!< [in] bias precision */
         VOID   * __restrict__ p_scratch             /*!< [in,out] scratch: rows x 4 bytes */
  );
         
WORD32 xa_nn_matXvec_8x16_16_sigmoid(
         WORD16 * __restrict__ p_out,                /*!< [out] 16b result: rows x 1 */
         WORD8  * __restrict__ p_mat1,               /*!< [in] 8b mat1: rows x cols1 */
         WORD8  * __restrict__ p_mat2,               /*!< [in] 8b mat2: rows x cols2 */
         WORD16 * __restrict__ p_vec1,               /*!< [in] 16b vec1: cols1 x 1 */
         WORD16 * __restrict__ p_vec2,               /*!< [in] 16b vec2: cols2 x 1 */
         VOID   * __restrict__ p_bias,               /*!< [in] bias: rows x 1 */
         WORD32 rows,                                /*!< [in] number of rows */
         WORD32 cols1,                               /*!< [in] number of columns of mat1 */
         WORD32 cols2,                               /*!< [in] number of columns of mat2 */
         WORD32 row_stride1,                         /*!< [in] row stride for mat1 */
         WORD32 row_stride2,                         /*!< [in] row stride for mat2 */
         WORD32 acc_shift,                             /*!< [in] out accumulator left shift amount */
         WORD32 bias_shift,                            /*!< [in] bias left shift amount */
         WORD32 bias_precision,                      /*!< [in] bias precision */
         VOID   * __restrict__ p_scratch             /*!< [in,out] scratch: rows x 4 bytes */
  );
         
WORD32 xa_nn_matXvec_batch_8x16_64(
         WORD64 ** __restrict__ p_out,               /*!< [out] array of result: rows x 1 pointers */
         WORD8  *  __restrict__ p_mat1,              /*!< [in] 8b mat1: rows x cols1 */
         WORD16 ** __restrict__ p_vec1,              /*!< [in] 16b vec1: cols1 x 1 */
         WORD16 *  __restrict__ p_bias,              /*!< [in] 16b bias TBD: Need array? */
         WORD32 rows,                                /*!< [in] number of rows */
         WORD32 cols1,                               /*!< [in] number of columns of mat1 */
         WORD32 row_stride1,                         /*!< [in] row stride for mat1 */
         WORD32 acc_shift,                             /*!< [in] out accumulator left shift amount */
         WORD32 bias_shift,                            /*!< [in] bias left shift amount */
         WORD32 vec_count                            /*!< [in] number of vectors: 2, 4, 2n */
  );
 
WORD32 xa_nn_matXvec_8x8_8(
         WORD8  * __restrict__ p_out,                /*!< [out] 8b result: rows x 1 */
         WORD8  * __restrict__ p_mat1,               /*!< [in] 8b mat1: rows x cols1 */
         WORD8  * __restrict__ p_mat2,               /*!< [in] 8b mat2: rows x cols2 */
         WORD8  * __restrict__ p_vec1,               /*!< [in] 8b vec1: cols1 x 1 */
         WORD8  * __restrict__ p_vec2,               /*!< [in] 8b vec2: cols2 x 1 */
         WORD8  * __restrict__ p_bias,               /*!< [in] 8b bias: rows x 1 */
         WORD32 rows,                                /*!< [in] number of rows */
         WORD32 cols1,                               /*!< [in] number of columns of mat1 */
         WORD32 cols2,                               /*!< [in] number of columns of mat2 */
         WORD32 row_stride1,                         /*!< [in] row stride for mat1 */
         WORD32 row_stride2,                         /*!< [in] row stride for mat2 */
         WORD32 acc_shift,                             /*!< [in] out accumulator left shift amount */
         WORD32 bias_shift                             /*!< [in] bias left shift amount */
  );
         
WORD32 xa_nn_matXvec_8x8_16(
         WORD16 * __restrict__ p_out,                /*!< [out] 16b result: rows x 1 */
         WORD8  * __restrict__ p_mat1,               /*!< [in] 8b mat1: rows x cols1 */
         WORD8  * __restrict__ p_mat2,               /*!< [in] 8b mat2: rows x cols2 */
         WORD8  * __restrict__ p_vec1,               /*!< [in] 8b vec1: cols1 x 1 */
         WORD8  * __restrict__ p_vec2,               /*!< [in] 8b vec2: cols2 x 1 */
         WORD8  * __restrict__ p_bias,               /*!< [in] 8b bias: rows x 1 */
         WORD32 rows,                                /*!< [in] number of rows */
         WORD32 cols1,                               /*!< [in] number of columns of mat1 */
         WORD32 cols2,                               /*!< [in] number of columns of mat2 */
         WORD32 row_stride1,                         /*!< [in] row stride for mat1 */
         WORD32 row_stride2,                         /*!< [in] row stride for mat2 */
         WORD32 acc_shift,                             /*!< [in] out accumulator left shift amount */
         WORD32 bias_shift                             /*!< [in] bias left shift amount */
  );

WORD32 xa_nn_matXvec_8x8_32(
         WORD32 * __restrict__ p_out,                /*!< [out] 32b result: rows x 1 */
         WORD8  * __restrict__ p_mat1,               /*!< [in] 8b mat1: rows x cols1 */
         WORD8  * __restrict__ p_mat2,               /*!< [in] 8b mat2: rows x cols2 */
         WORD8  * __restrict__ p_vec1,               /*!< [in] 8b vec1: cols1 x 1 */
         WORD8  * __restrict__ p_vec2,               /*!< [in] 8b vec2: cols2 x 1 */
         WORD8  * __restrict__ p_bias,               /*!< [in] 8b bias: rows x 1 */
         WORD32 rows,                                /*!< [in] number of rows */
         WORD32 cols1,                               /*!< [in] number of columns of mat1 */
         WORD32 cols2,                               /*!< [in] number of columns of mat2 */
         WORD32 row_stride1,                         /*!< [in] row stride for mat1 */
         WORD32 row_stride2,                         /*!< [in] row stride for mat2 */
         WORD32 acc_shift,                             /*!< [in] out accumulator left shift amount */
         WORD32 bias_shift                             /*!< [in] bias left shift amount */
  );

WORD32 xa_nn_matXvec_8x8_8_tanh(
         WORD8  * __restrict__ p_out,                /*!< [out] 8b result: rows x 1 */
         WORD8  * __restrict__ p_mat1,               /*!< [in] 8b mat1: rows x cols1 */
         WORD8  * __restrict__ p_mat2,               /*!< [in] 8b mat2: rows x cols2 */
         WORD8  * __restrict__ p_vec1,               /*!< [in] 8b vec1: cols1 x 1 */
         WORD8  * __restrict__ p_vec2,               /*!< [in] 8b vec2: cols2 x 1 */
         VOID   * __restrict__ p_bias,               /*!< [in] bias: rows x 1 */
         WORD32 rows,                                /*!< [in] number of rows */
         WORD32 cols1,                               /*!< [in] number of columns of mat1 */
         WORD32 cols2,                               /*!< [in] number of columns of mat2 */
         WORD32 row_stride1,                         /*!< [in] row stride for mat1 */
         WORD32 row_stride2,                         /*!< [in] row stride for mat2 */
         WORD32 acc_shift,                             /*!< [in] out accumulator left shift amount */
         WORD32 bias_shift,                            /*!< [in] bias left shift amount */
         WORD32 bias_precision,                      /*!< [in] bias precision */
         VOID   * __restrict__ p_scratch             /*!< [in,out] scratch: rows x 4 bytes */
  );

WORD32 xa_nn_matXvec_8x8_8_sigmoid(
         WORD8  * __restrict__ p_out,                /*!< [out] 8b result: rows x 1 */
         WORD8  * __restrict__ p_mat1,               /*!< [in] 8b mat1: rows x cols1 */
         WORD8  * __restrict__ p_mat2,               /*!< [in] 8b mat2: rows x cols2 */
         WORD8  * __restrict__ p_vec1,               /*!< [in] 8b vec1: cols1 x 1 */
         WORD8  * __restrict__ p_vec2,               /*!< [in] 8b vec2: cols2 x 1 */
         VOID   * __restrict__ p_bias,               /*!< [in] bias: rows x 1 */
         WORD32 rows,                                /*!< [in] number of rows */
         WORD32 cols1,                               /*!< [in] number of columns of mat1 */
         WORD32 cols2,                               /*!< [in] number of columns of mat2 */
         WORD32 row_stride1,                         /*!< [in] row stride for mat1 */
         WORD32 row_stride2,                         /*!< [in] row stride for mat2 */
         WORD32 acc_shift,                             /*!< [in] out accumulator left shift amount */
         WORD32 bias_shift,                            /*!< [in] bias left shift amount */
         WORD32 bias_precision,                      /*!< [in] bias precision */
         VOID   * __restrict__ p_scratch             /*!< [in,out] scratch: rows x 4 bytes */
  );
         
WORD32 xa_nn_matXvec_batch_8x8_32(
         WORD32 ** __restrict__ p_out,               /*!< [out] array of result: rows x 1 pointers */
         WORD8  *  __restrict__ p_mat1,              /*!< [in] 8b mat1: rows x cols1 */
         WORD8  ** __restrict__ p_vec1,              /*!< [in] 8b vec1: cols1 x 1 */
         WORD8  *  __restrict__ p_bias,              /*!< [in] bias TBD: Need array? */
         WORD32 rows,                                /*!< [in] number of rows */
         WORD32 cols1,                               /*!< [in] number of columns of mat1 */
         WORD32 row_stride1,                         /*!< [in] row stride for mat1 */
         WORD32 acc_shift,                             /*!< [in] out accumulator left shift amount */
         WORD32 bias_shift,                            /*!< [in] bias left shift amount */
         WORD32 vec_count                            /*!< [in] number of vectors: 2, 4, 2n */
  );
         
WORD32 xa_nn_matXvec_f32xf32_f32_sigmoid(
       FLOAT32  * __restrict__ p_out,                /*!< [out] f32b result: rows x 1 */
       FLOAT32  * __restrict__ p_mat1,               /*!< [in] f32b mat1: rows x cols1 */
       FLOAT32  * __restrict__ p_mat2,               /*!< [in] f32b mat2: rows x cols2 */
       FLOAT32  * __restrict__ p_vec1,               /*!< [in] f32b vec1: cols1 x 1 */
       FLOAT32  * __restrict__ p_vec2,               /*!< [in] f32b vec2: cols2 x 1 */
       FLOAT32  * __restrict__ p_bias,               /*!< [in] f32b bias: rows x 1 */
       WORD32 rows,                                  /*!< [in] number of rows */
       WORD32 cols1,                                 /*!< [in] number of columns of mat1 */
       WORD32 cols2,                                 /*!< [in] number of columns of mat2 */
       WORD32 row_stride1,                           /*!< [in] row stride for mat1 */
       WORD32 row_stride2,                           /*!< [in] row stride for mat2 */
       FLOAT32  * __restrict__ p_scratch             /*!< [in,out] scratch: rows x 4 bytes */
  );

WORD32 xa_nn_matXvec_f32xf32_f32_tanh(
       FLOAT32  * __restrict__ p_out,                /*!< [out] f32b result: rows x 1 */
       FLOAT32  * __restrict__ p_mat1,               /*!< [in] f32b mat1: rows x cols1 */
       FLOAT32  * __restrict__ p_mat2,               /*!< [in] f32b mat2: rows x cols2 */
       FLOAT32  * __restrict__ p_vec1,               /*!< [in] f32b vec1: cols1 x 1 */
       FLOAT32  * __restrict__ p_vec2,               /*!< [in] f32b vec2: cols2 x 1 */
       FLOAT32  * __restrict__ p_bias,               /*!< [in] f32b bias: rows x 1 */
       WORD32 rows,                                  /*!< [in] number of rows */
       WORD32 cols1,                                 /*!< [in] number of columns of mat1 */
       WORD32 cols2,                                 /*!< [in] number of columns of mat2 */
       WORD32 row_stride1,                           /*!< [in] row stride for mat1 */
       WORD32 row_stride2,                           /*!< [in] row stride for mat2 */
       FLOAT32  * __restrict__ p_scratch             /*!< [in,out] scratch: rows x 4 bytes */
  );

WORD32 xa_nn_matXvec_f32xf32_f32(
       FLOAT32  * __restrict__ p_out,                /*!< [out] f32b result: rows x 1 */
       const FLOAT32  * __restrict__ p_mat1,         /*!< [in] f32b mat1: rows x cols1 */
       const FLOAT32  * __restrict__ p_mat2,         /*!< [in] f32b mat2: rows x cols2 */
       const FLOAT32  * __restrict__ p_vec1,         /*!< [in] f32b vec1: cols1 x 1 */
       const FLOAT32  * __restrict__ p_vec2,         /*!< [in] f32b vec2: cols2 x 1 */
       const FLOAT32  * __restrict__ p_bias,         /*!< [in] f32b bias: rows x 1 */
       WORD32 rows,                                  /*!< [in] number of rows */
       WORD32 cols1,                                 /*!< [in] number of columns of mat1 */
       WORD32 cols2,                                 /*!< [in] number of columns of mat2 */
       WORD32 row_stride1,                           /*!< [in] row stride for mat1 */
       WORD32 row_stride2                            /*!< [in] row stride for mat2 */
  );

WORD32 xa_nn_matXvec_batch_f32xf32_f32(
       FLOAT32  ** __restrict__ p_out,               /*!< [out] f32b result: rows x vec_count */
       FLOAT32  * __restrict__ p_mat1,               /*!< [in] f32b mat1: rows x cols1 */
       FLOAT32  ** __restrict__ p_vec1,              /*!< [in] f32b vec1: cols1 x vec_count */
       FLOAT32  * __restrict__ p_bias,               /*!< [in] f32b bias: rows x 1 */
       WORD32 rows,                                  /*!< [in] number of rows */
       WORD32 cols1,                                 /*!< [in] number of columns of mat1 */
       WORD32 row_stride1,                           /*!< [in] row stride for mat1 */
       WORD32 vec_count                              /*!< [in] number of vectors: 2, 4, 2n */
  );
 
WORD32 xa_nn_vec_sigmoid_32_32(               
    WORD32       * __restrict__ p_out,         /*!< [out] result: vec_length x 1, Q16.15 */
    const WORD32 * __restrict__ p_vec,         /*!< [in] input data: vec_length x 1, Q6.25 */
    WORD32       vec_length                    /*!< [in] length of vectors */
  );

WORD32 xa_nn_vec_tanh_32_32(               
    WORD32       * __restrict__ p_out,         /*!< [out] result: vec_length x 1, Q16.15 */
    const WORD32 * __restrict__ p_vec,         /*!< [in] input data: vec_length x 1, Q6.25 */
    WORD32       vec_length                    /*!< [in] length of vectors */
  );

WORD32 xa_nn_vec_relu_32_32(               
    WORD32       * __restrict__ p_out,         /*!< [out] result: vec_length x 1, Q16.15 */
    const WORD32 * __restrict__ p_vec,         /*!< [in] input data: vec_length x 1, Q6.25 */
    WORD32       threshold,                    /*!< [in] threshold, Q16.15 */
    WORD32       vec_length                    /*!< [in] length of vectors */
  );

WORD32 xa_nn_vec_relu1_32_32(               
    WORD32       * __restrict__ p_out,         /*!< [out] result: vec_length x 1, Q16.15 */
    const WORD32 * __restrict__ p_vec,         /*!< [in] input data: vec_length x 1, Q6.25 */
    WORD32       vec_length                    /*!< [in] length of vectors */
  );

WORD32 xa_nn_vec_relu6_32_32(               
    WORD32       * __restrict__ p_out,         /*!< [out] result: vec_length x 1, Q16.15 */
    const WORD32 * __restrict__ p_vec,         /*!< [in] input data: vec_length x 1, Q6.25 */
    WORD32       vec_length                    /*!< [in] length of vectors */
  );

WORD32 xa_nn_vec_softmax_32_32(               
    WORD32       * __restrict__ p_out,         /*!< [out] result: vec_length x 1, Q16.15 */
    const WORD32 * __restrict__ p_vec,         /*!< [in] input data: vec_length x 1, Q6.25 */
    WORD32       vec_length                    /*!< [in] length of vectors */
  );

WORD32 xa_nn_vec_sigmoid_f32_f32(               
    FLOAT32       * __restrict__ p_out,        /*!< [out] result: vec_length x 1, floating point */
    const FLOAT32 * __restrict__ p_vec,        /*!< [in] input data: vec_length x 1, floating point */
    WORD32        vec_length                   /*!< [in] length of vectors */
  );

WORD32 xa_nn_vec_tanh_f32_f32(               
    FLOAT32       * __restrict__ p_out,        /*!< [out] result: vec_length x 1, floating point */
    const FLOAT32 * __restrict__ p_vec,        /*!< [in] input data: vec_length x 1, floating point */
    WORD32        vec_length                   /*!< [in] length of vectors */
  );

WORD32 xa_nn_vec_relu_f32_f32(               
    FLOAT32       * __restrict__ p_out,        /*!< [out] result: vec_length x 1, floating point */
    const FLOAT32 * __restrict__ p_vec,        /*!< [in] input data: vec_length x 1, floating point */
    FLOAT32       threshold,                   /*!< [in] threshold, floating point */
    WORD32        vec_length                   /*!< [in] length of vectors */
  );

WORD32 xa_nn_vec_relu1_f32_f32(               
    FLOAT32       * __restrict__ p_out,        /*!< [out] result: vec_length x 1, floating point */
    const FLOAT32 * __restrict__ p_vec,        /*!< [in] input data: vec_length x 1, floating point */
    WORD32        vec_length                   /*!< [in] length of vectors */
  );

WORD32 xa_nn_vec_relu6_f32_f32(               
    FLOAT32       * __restrict__ p_out,        /*!< [out] result: vec_length x 1, floating point */
    const FLOAT32 * __restrict__ p_vec,        /*!< [in] input data: vec_length x 1, floating point */
    WORD32        vec_length                   /*!< [in] length of vectors */
  );

WORD32 xa_nn_vec_softmax_f32_f32(               
    FLOAT32       * __restrict__ p_out,        /*!< [out] result: vec_length x 1, floating point */
    const FLOAT32 * __restrict__ p_vec,        /*!< [in] input data: vec_length x 1, floating point */
    WORD32        vec_length                   /*!< [in] length of vectors */
  );
 
WORD32 xa_nn_vec_sigmoid_32_16(               
    WORD16       * __restrict__ p_out,         /*!< [out] result: vec_length x 1, Q0.15 */
    const WORD32 * __restrict__ p_vec,         /*!< [in] input data: vec_length x 1, Q6.25 */
    WORD32       vec_length                    /*!< [in] length of vectors */
  );

WORD32 xa_nn_vec_tanh_32_16(               
    WORD16       * __restrict__ p_out,         /*!< [out] result: vec_length x 1, Q0.15 */
    const WORD32 * __restrict__ p_vec,         /*!< [in] input data: vec_length x 1, Q6.25 */
    WORD32       vec_length                    /*!< [in] length of vectors */
  );

WORD32 xa_nn_vec_sigmoid_32_8(               
    WORD8        * __restrict__ p_out,         /*!< [out] result: vec_length x 1, Q0.7 */
    const WORD32 * __restrict__ p_vec,         /*!< [in] input data: vec_length x 1, Q6.25 */
    WORD32       vec_length                    /*!< [in] length of vectors */
  );

WORD32 xa_nn_vec_tanh_32_8(               
    WORD8        * __restrict__ p_out,         /*!< [out] result: vec_length x 1, Q0.7 */
    const WORD32 * __restrict__ p_vec,         /*!< [in] input data: vec_length x 1, Q6.25 */
    WORD32       vec_length                    /*!< [in] length of vectors */
  );

WORD32 xa_nn_vec_interpolation_q15(
    WORD16       * __restrict__ p_out,           /*!< [out] result: num_elements x 1 */
    const WORD16 * __restrict__ p_ifact,         /*!< [in] interp. factor: num_elements x 1 */
    const WORD16 * __restrict__ p_inp1,          /*!< [in] input data: num_elements x 1 */
    const WORD16 * __restrict__ p_inp2,          /*!< [in] input data: num_elements x 1 */
    WORD32       num_elements                    /*!< [in] length of vectors */
  );


WORD32 xa_nn_conv1d_std_getsize(
    WORD32 kernel_height,
    WORD32 input_width,
    WORD32 input_channels,
    WORD32 input_precision);

WORD32 xa_nn_conv1d_std_8x16(
    WORD16* __restrict__ p_out,
    WORD16* __restrict__ p_inp,
    WORD8*  __restrict__ p_kernel,
    WORD16* __restrict__ p_bias,
    WORD32 input_height,
    WORD32 input_width,
    WORD32 input_channels,
    WORD32 kernel_height,
    WORD32 out_channels,
    WORD32 y_stride,
    WORD32 y_padding,
    WORD32 out_height,
    WORD32 bias_shift,
    WORD32 acc_shift,
    WORD32 out_data_format,
    VOID *p_handle);

WORD32 xa_nn_conv1d_std_8x8(
    WORD8* __restrict__ p_out,
    WORD8* __restrict__ p_inp,
    WORD8* __restrict__ p_kernel,
    WORD8* __restrict__ p_bias,
    WORD32 input_height,
    WORD32 input_width,
    WORD32 input_channels,
    WORD32 kernel_height,
    WORD32 out_channels,
    WORD32 y_stride,
    WORD32 y_padding,
    WORD32 out_height,
    WORD32 bias_shift,
    WORD32 acc_shift,
    WORD32 out_data_format,
    VOID *p_handle);

WORD32 xa_nn_conv1d_std_16x16(
    WORD16* __restrict__ p_out,
    WORD16* __restrict__ p_inp,
    WORD16* __restrict__ p_kernel,
    WORD16* __restrict__ p_bias,
    WORD32 input_height,
    WORD32 input_width,
    WORD32 input_channels,
    WORD32 kernel_height,
    WORD32 out_channels,
    WORD32 y_stride,
    WORD32 y_padding,
    WORD32 out_height,
    WORD32 bias_shift,
    WORD32 acc_shift,
    WORD32 out_data_format,
    VOID *p_handle);

WORD32 xa_nn_conv1d_std_f32(
    FLOAT32* __restrict__ p_out,
    FLOAT32* __restrict__ p_inp,
    FLOAT32* __restrict__ p_kernel,
    FLOAT32* __restrict__ p_bias,
    WORD32 input_height,
    WORD32 input_width,
    WORD32 input_channels,
    WORD32 kernel_height,
    WORD32 out_channels,
    WORD32 y_stride,
    WORD32 y_padding,
    WORD32 out_height,
    WORD32 out_data_format,
    VOID *p_handle);


WORD32 xa_nn_conv2d_std_getsize(
    WORD32 input_height,
    WORD32 input_channels,
    WORD32 kernel_height,
    WORD32 kernel_width,
    WORD32 y_stride,
    WORD32 y_padding,
    WORD32 out_height,
    WORD32 input_precision);

WORD32 xa_nn_conv2d_std_8x16(
    WORD16* __restrict__ p_out,
    WORD16* __restrict__ p_inp,
    WORD8*  __restrict__ p_kernel,
    WORD16* __restrict__ p_bias,
    WORD32 input_height,
    WORD32 input_width,
    WORD32 input_channels,
    WORD32 kernel_height,
    WORD32 kernel_width,
    WORD32 out_channels,
    WORD32 x_stride,
    WORD32 y_stride,
    WORD32 x_padding,
    WORD32 y_padding,
    WORD32 out_height,
    WORD32 out_width,
    WORD32 bias_shift,
    WORD32 acc_shift,
    WORD32 out_data_format,
    VOID *p_handle);

WORD32 xa_nn_conv2d_std_8x8(
    WORD8* __restrict__ p_out,
    WORD8* __restrict__ p_inp,
    WORD8* __restrict__ p_kernel,
    WORD8* __restrict__ p_bias,
    WORD32 input_height,
    WORD32 input_width,
    WORD32 input_channels,
    WORD32 kernel_height,
    WORD32 kernel_width,
    WORD32 out_channels,
    WORD32 x_stride,
    WORD32 y_stride,
    WORD32 x_padding,
    WORD32 y_padding,
    WORD32 out_height,
    WORD32 out_width,
    WORD32 bias_shift,
    WORD32 acc_shift,
    WORD32 out_data_format,
    VOID *p_handle);

WORD32 xa_nn_conv2d_std_16x16(
    WORD16* __restrict__ p_out,
    WORD16* __restrict__ p_inp,
    WORD16* __restrict__ p_kernel,
    WORD16* __restrict__ p_bias,
    WORD32 input_height,
    WORD32 input_width,
    WORD32 input_channels,
    WORD32 kernel_height,
    WORD32 kernel_width,
    WORD32 out_channels,
    WORD32 x_stride,
    WORD32 y_stride,
    WORD32 x_padding,
    WORD32 y_padding,
    WORD32 out_height,
    WORD32 out_width,
    WORD32 bias_shift,
    WORD32 acc_shift,
    WORD32 out_data_format,
    VOID *p_handle);

WORD32 xa_nn_conv2d_std_f32(
    FLOAT32* __restrict__ p_out,
    const FLOAT32* __restrict__ p_inp,
    const FLOAT32* __restrict__ p_kernel,
    const FLOAT32* __restrict__ p_bias,
    WORD32 input_height,
    WORD32 input_width,
    WORD32 input_channels,
    WORD32 kernel_height,
    WORD32 kernel_width,
    WORD32 out_channels,
    WORD32 x_stride,
    WORD32 y_stride,
    WORD32 x_padding,
    WORD32 y_padding,
    WORD32 out_height,
    WORD32 out_width,
    WORD32 out_data_format,
    VOID *p_handle);

WORD32 xa_nn_conv2d_pointwise_f32(
    FLOAT32* __restrict__ p_out,
    FLOAT32* __restrict__ p_kernel,
    FLOAT32* __restrict__ p_inp,
    FLOAT32* __restrict__ p_bias,
    WORD32  input_height,
    WORD32  input_width,
    WORD32  input_channels,
    WORD32  out_channels,
    WORD32  out_data_format);

WORD32 xa_nn_conv2d_pointwise_8x16
  (pWORD16 __restrict__ p_out
   ,pWORD8  __restrict__ p_kernel
   ,pWORD16 __restrict__ p_inp
   ,pWORD16 __restrict__ p_bias
   ,WORD32  input_height
   ,WORD32  input_width
   ,WORD32  input_channels
   ,WORD32  out_channels
   ,WORD32  acc_shift
   ,WORD32  bias_shift
   ,WORD32  out_data_format
  );

WORD32 xa_nn_conv2d_pointwise_8x8
  (pWORD8 __restrict__ p_out
   ,pWORD8  __restrict__ p_kernel
   ,pWORD8 __restrict__ p_inp
   ,pWORD8 __restrict__ p_bias
   ,WORD32  input_height
   ,WORD32  input_width
   ,WORD32  input_channels
   ,WORD32  out_channels
   ,WORD32  acc_shift
   ,WORD32  bias_shift
   ,WORD32  out_data_format
  );

#ifndef NNLIB_V2
WORD32 xa_nn_conv2d_depthwise_getsize
  (WORD32 input_width
   ,WORD32 kernel_height
   ,WORD32 kernel_width
   ,WORD32 x_stride
   ,WORD32 y_stride
   ,WORD32 x_padding
   ,WORD32 output_width
   ,WORD32 circ_buf_precision
   );

WORD32 xa_nn_conv2d_depthwise_f32(
    FLOAT32* __restrict__ p_out,
    FLOAT32* __restrict__ p_kernel,
    FLOAT32* __restrict__ p_inp,
    FLOAT32* __restrict__ p_bias,
    WORD32  input_height,
    WORD32  input_width,
    WORD32  input_channels,
    WORD32  kernel_height,
    WORD32  kernel_width,
    WORD32  channels_multiplier,
    WORD32  x_stride,
    WORD32  y_stride,
    WORD32  x_padding,
    WORD32  y_padding,
    WORD32  out_height,
    WORD32  out_width,
    WORD32  out_data_format,
    pVOID p_scratch);

WORD32 xa_nn_conv2d_depthwise_8x16
    (pWORD16 __restrict__ p_out
     ,pWORD8 __restrict__ p_kernel
     ,pWORD16 __restrict__ p_inp
     ,pWORD16 __restrict__ p_bias
     ,WORD32  input_height
     ,WORD32  input_width
     ,WORD32  input_channels
     ,WORD32  kernel_height
     ,WORD32  kernel_width
     ,WORD32  channels_multiplier
     ,WORD32  x_stride
     ,WORD32  y_stride
     ,WORD32  x_padding
     ,WORD32  y_padding
     ,WORD32  out_height
     ,WORD32  out_width
     ,WORD32  acc_shift
     ,WORD32  bias_shift
     ,WORD32  out_data_format
     ,pVOID p_scratch
     );

WORD32 xa_nn_conv2d_depthwise_8x8
    (pWORD8 __restrict__ p_out
     ,pWORD8 __restrict__ p_kernel
     ,pWORD8 __restrict__ p_inp 
     ,pWORD8 __restrict__ p_bias
     ,WORD32  input_height
     ,WORD32  input_width
     ,WORD32  input_channels
     ,WORD32  kernel_height
     ,WORD32  kernel_width
     ,WORD32  channels_multiplier
     ,WORD32  x_stride
     ,WORD32  y_stride
     ,WORD32  x_padding
     ,WORD32  y_padding
     ,WORD32  out_height
     ,WORD32  out_width
     ,WORD32  acc_shift
     ,WORD32  bias_shift
     ,WORD32  out_data_format
     ,pVOID p_scratch
     );

WORD32 xa_nn_conv2d_depthwise_16x16
    (pWORD16 __restrict__ p_out
     ,pWORD16 __restrict__ p_kernel
     ,pWORD16 __restrict__ p_inp 
     ,pWORD16 __restrict__ p_bias
     ,WORD32  input_height
     ,WORD32  input_width
     ,WORD32  input_channels
     ,WORD32  kernel_height
     ,WORD32  kernel_width
     ,WORD32  channels_multiplier
     ,WORD32  x_stride
     ,WORD32  y_stride
     ,WORD32  x_padding
     ,WORD32  y_padding
     ,WORD32  out_height
     ,WORD32  out_width
     ,WORD32  acc_shift
     ,WORD32  bias_shift
     ,WORD32  out_data_format
     ,pVOID p_scratch
     );
#else
WORD32 xa_nn_conv2d_depthwise_getsize
  (WORD32 input_height
   ,WORD32 input_width
   ,WORD32 input_channels
   ,WORD32 kernel_height
   ,WORD32 kernel_width
   ,WORD32 channels_multiplier
   ,WORD32 x_stride
   ,WORD32 y_stride
   ,WORD32 x_padding
   ,WORD32 y_padding
   ,WORD32 output_height
   ,WORD32 output_width
   ,WORD32 circ_buf_precision
   ,WORD32 inp_data_format
   );

WORD32 xa_nn_conv2d_depthwise_f32(
    FLOAT32* __restrict__ p_out,
    const FLOAT32* __restrict__ p_kernel,
    const FLOAT32* __restrict__ p_inp,
    const FLOAT32* __restrict__ p_bias,
    WORD32  input_height,
    WORD32  input_width,
    WORD32  input_channels,
    WORD32  kernel_height,
    WORD32  kernel_width,
    WORD32  channels_multiplier,
    WORD32  x_stride,
    WORD32  y_stride,
    WORD32  x_padding,
    WORD32  y_padding,
    WORD32  out_height,
    WORD32  out_width,
    WORD32  inp_data_format,
    WORD32  out_data_format,
    pVOID p_scratch);

WORD32 xa_nn_conv2d_depthwise_8x16
    (pWORD16 __restrict__ p_out
     ,const pWORD8 __restrict__ p_kernel
     ,const pWORD16 __restrict__ p_inp
     ,const pWORD16 __restrict__ p_bias
     ,WORD32  input_height
     ,WORD32  input_width
     ,WORD32  input_channels
     ,WORD32  kernel_height
     ,WORD32  kernel_width
     ,WORD32  channels_multiplier
     ,WORD32  x_stride
     ,WORD32  y_stride
     ,WORD32  x_padding
     ,WORD32  y_padding
     ,WORD32  out_height
     ,WORD32  out_width
     ,WORD32  acc_shift
     ,WORD32  bias_shift
     ,WORD32  inp_data_format
     ,WORD32  out_data_format
     ,pVOID p_scratch
     );

WORD32 xa_nn_conv2d_depthwise_8x8
    (pWORD8 __restrict__ p_out
     ,const pWORD8 __restrict__ p_kernel
     ,const pWORD8 __restrict__ p_inp 
     ,const pWORD8 __restrict__ p_bias
     ,WORD32  input_height
     ,WORD32  input_width
     ,WORD32  input_channels
     ,WORD32  kernel_height
     ,WORD32  kernel_width
     ,WORD32  channels_multiplier
     ,WORD32  x_stride
     ,WORD32  y_stride
     ,WORD32  x_padding
     ,WORD32  y_padding
     ,WORD32  out_height
     ,WORD32  out_width
     ,WORD32  acc_shift
     ,WORD32  bias_shift
     ,WORD32  inp_data_format
     ,WORD32  out_data_format
     ,pVOID p_scratch
     );

WORD32 xa_nn_conv2d_depthwise_16x16
    (pWORD16 __restrict__ p_out
     ,const pWORD16 __restrict__ p_kernel
     ,const pWORD16 __restrict__ p_inp 
     ,const pWORD16 __restrict__ p_bias
     ,WORD32  input_height
     ,WORD32  input_width
     ,WORD32  input_channels
     ,WORD32  kernel_height
     ,WORD32  kernel_width
     ,WORD32  channels_multiplier
     ,WORD32  x_stride
     ,WORD32  y_stride
     ,WORD32  x_padding
     ,WORD32  y_padding
     ,WORD32  out_height
     ,WORD32  out_width
     ,WORD32  acc_shift
     ,WORD32  bias_shift
     ,WORD32  inp_data_format
     ,WORD32  out_data_format
     ,pVOID p_scratch
     );
#endif

WORD32 xa_nn_conv2d_pointwise_16x16
  (pWORD16 __restrict__ p_out
   ,pWORD16  __restrict__ p_kernel
   ,pWORD16 __restrict__ p_inp
   ,pWORD16 __restrict__ p_bias
   ,WORD32  input_height
   ,WORD32  input_width
   ,WORD32  input_channels
   ,WORD32  out_channels
   ,WORD32  acc_shift
   ,WORD32  bias_shift
   ,WORD32  out_data_format
  );

#ifndef NNLIB_V2
WORD32 xa_nn_avgpool_8(
    WORD8 *__restrict__ p_out,          /*!< [out] 8b result (WHD): out_height x out_width x input_channels */
    WORD8 *__restrict__ p_inp,          /*!< [in] 8b input cube (WHD): input_height x input_width x input_channels */
    WORD32  input_height,               /*!< [in] input height*/
    WORD32  input_width,                /*!< [in] input width */
    WORD32  input_channels,             /*!< [in] input channels */
    WORD32  kernel_height,              /*!< [in] pooling window height */
    WORD32  kernel_width,               /*!< [in] pooling window width */
    WORD32  x_stride,                   /*!< [in] horizontal stride */
    WORD32  y_stride,                   /*!< [in] vertical stride */
    WORD32  x_padding,                  /*!< [in] left padding width */
    WORD32  y_padding,                  /*!< [in] top padding height */
    WORD32  out_height,                 /*!< [in] output height */
    WORD32  out_width,                  /*!< [in] output width */
    WORD32  out_data_format,            /*!< [in] input data format: WHD */
    VOID *p_scratch);                   /*!< [in] scratch memory of size given by xa_nn_avgpool_getsize with inp_precision=8 */

WORD32 xa_nn_avgpool_16(
    WORD16 *__restrict__ p_out,         /*!< [out] 16b result (WHD): out_height x out_width x input_channels */
    WORD16 *__restrict__ p_inp,         /*!< [in] 16b input cube (WHD): input_height x input_width x input_channels */
    WORD32  input_height,               /*!< [in] input height*/
    WORD32  input_width,                /*!< [in] input width */
    WORD32  input_channels,             /*!< [in] input channels */
    WORD32  kernel_height,              /*!< [in] pooling window height */
    WORD32  kernel_width,               /*!< [in] pooling window width */
    WORD32  x_stride,                   /*!< [in] horizontal stride */
    WORD32  y_stride,                   /*!< [in] vertical stride */
    WORD32  x_padding,                  /*!< [in] left padding width */
    WORD32  y_padding,                  /*!< [in] top padding height */
    WORD32  out_height,                 /*!< [in] output height */
    WORD32  out_width,                  /*!< [in] output width */
    WORD32  out_data_format,            /*!< [in] input data format: WHD */
    VOID *p_scratch);                   /*!< [in] scratch memory of size given by xa_nn_avgpool_getsize with inp_precision=16 */

WORD32 xa_nn_avgpool_f32(
        FLOAT32 *__restrict__ p_out,        /*!< [out] f32b result (WHD) : out_height x out_width x input_channels */
const   FLOAT32 *__restrict__ p_inp,        /*!< [in] f32b input cube (WHD) : input_height x input_width x input_channels */
        WORD32  input_height,               /*!< [in] input height*/
        WORD32  input_width,                /*!< [in] input width */
        WORD32  input_channels,             /*!< [in] input channels */
        WORD32  kernel_height,              /*!< [in] pooling window height */
        WORD32  kernel_width,               /*!< [in] pooling window width */
        WORD32  x_stride,                   /*!< [in] horizontal stride */
        WORD32  y_stride,                   /*!< [in] vertical stride */
        WORD32  x_padding,                  /*!< [in] left padding width */
        WORD32  y_padding,                  /*!< [in] top padding height */
        WORD32  out_height,                 /*!< [in] output height */
        WORD32  out_width,                  /*!< [in] output width */
        WORD32  out_data_format,            /*!< [in] output data format: WHD */
        VOID *p_scratch);                   /*!< [in] scratch memory of size given by xa_nn_avgpool_getsize with inp_precision=-1 */

WORD32 xa_nn_avgpool_getsize(
        WORD32 inp_precision,
        WORD32 input_width,
        WORD32 kernel_height,
        WORD32 kernel_width,
        WORD32 x_stride,
        WORD32 y_stride,
        WORD32 x_padding,
        WORD32 out_height,
        WORD32 out_width);

#else

WORD32 xa_nn_avgpool_8(
    WORD8 *__restrict__ p_out,          /*!< [out] 8b result (WHD): out_height x out_width x input_channels */
    WORD8 *__restrict__ p_inp,          /*!< [in] 8b input cube (WHD): input_height x input_width x input_channels */
    WORD32  input_height,               /*!< [in] input height*/
    WORD32  input_width,                /*!< [in] input width */
    WORD32  input_channels,             /*!< [in] input channels */
    WORD32  kernel_height,              /*!< [in] pooling window height */
    WORD32  kernel_width,               /*!< [in] pooling window width */
    WORD32  x_stride,                   /*!< [in] horizontal stride */
    WORD32  y_stride,                   /*!< [in] vertical stride */
    WORD32  x_padding,                  /*!< [in] left padding width */
    WORD32  y_padding,                  /*!< [in] top padding height */
    WORD32  out_height,                 /*!< [in] output height */
    WORD32  out_width,                  /*!< [in] output width */
    WORD32  inp_data_format,            /*!< [in] input data format: WHD */
    WORD32  out_data_format,            /*!< [in] output data format: WHD */
    VOID *p_scratch);                   /*!< [in] scratch memory of size given by xa_nn_avgpool_getsize with inp_precision=8 */

WORD32 xa_nn_avgpool_16(
    WORD16 *__restrict__ p_out,         /*!< [out] 16b result (WHD): out_height x out_width x input_channels */
    WORD16 *__restrict__ p_inp,         /*!< [in] 16b input cube (WHD): input_height x input_width x input_channels */
    WORD32  input_height,               /*!< [in] input height*/
    WORD32  input_width,                /*!< [in] input width */
    WORD32  input_channels,             /*!< [in] input channels */
    WORD32  kernel_height,              /*!< [in] pooling window height */
    WORD32  kernel_width,               /*!< [in] pooling window width */
    WORD32  x_stride,                   /*!< [in] horizontal stride */
    WORD32  y_stride,                   /*!< [in] vertical stride */
    WORD32  x_padding,                  /*!< [in] left padding width */
    WORD32  y_padding,                  /*!< [in] top padding height */
    WORD32  out_height,                 /*!< [in] output height */
    WORD32  out_width,                  /*!< [in] output width */
    WORD32  inp_data_format,            /*!< [in] input data format: WHD */
    WORD32  out_data_format,            /*!< [in] output data format: WHD */
    VOID *p_scratch);                   /*!< [in] scratch memory of size given by xa_nn_avgpool_getsize with inp_precision=16 */

WORD32 xa_nn_avgpool_f32(
        FLOAT32 *__restrict__ p_out,        /*!< [out] f32b result (WHD) : out_height x out_width x input_channels */
const   FLOAT32 *__restrict__ p_inp,        /*!< [in] f32b input cube (WHD) : input_height x input_width x input_channels */
        WORD32  input_height,               /*!< [in] input height*/
        WORD32  input_width,                /*!< [in] input width */
        WORD32  input_channels,             /*!< [in] input channels */
        WORD32  kernel_height,              /*!< [in] pooling window height */
        WORD32  kernel_width,               /*!< [in] pooling window width */
        WORD32  x_stride,                   /*!< [in] horizontal stride */
        WORD32  y_stride,                   /*!< [in] vertical stride */
        WORD32  x_padding,                  /*!< [in] left padding width */
        WORD32  y_padding,                  /*!< [in] top padding height */
        WORD32  out_height,                 /*!< [in] output height */
        WORD32  out_width,                  /*!< [in] output width */
        WORD32  inp_data_format,            /*!< [in] input data format: WHD */
        WORD32  out_data_format,            /*!< [in] output data format: WHD */
        VOID *p_scratch);                   /*!< [in] scratch memory of size given by xa_nn_avgpool_getsize with inp_precision=-1 */

WORD32 xa_nn_avgpool_getsize(
    WORD32 input_channels,
    WORD32 inp_precision,
    WORD32 out_precision,
    WORD32 input_height,
    WORD32 input_width,
    WORD32 kernel_height,
    WORD32 kernel_width,
    WORD32 x_stride,
    WORD32 y_stride,
    WORD32 x_padding,
    WORD32 y_padding,
    WORD32 out_height,
    WORD32 out_width,
    WORD32 inp_data_format,
    WORD32 out_data_format);
#endif

#ifndef NNLIB_V2
WORD32 xa_nn_maxpool_8(
    WORD8 *__restrict__ p_out,
    WORD8 *__restrict__ p_inp ,
    WORD32  input_height,
    WORD32  input_width,
    WORD32  input_channels,
    WORD32  kernel_height,
    WORD32  kernel_width,
    WORD32  x_stride,
    WORD32  y_stride,
    WORD32  x_padding,
    WORD32  y_padding,
    WORD32  out_height,
    WORD32  out_width,
    WORD32  out_data_format,
    VOID   *p_scratch);

WORD32 xa_nn_maxpool_16(
    WORD16 *__restrict__ p_out,
    WORD16 *__restrict__ p_inp ,
    WORD32  input_height,
    WORD32  input_width,
    WORD32  input_channels,
    WORD32  kernel_height,
    WORD32  kernel_width,
    WORD32  x_stride,
    WORD32  y_stride,
    WORD32  x_padding,
    WORD32  y_padding,
    WORD32  out_height,
    WORD32  out_width,
    WORD32  out_data_format,
    VOID   *p_scratch);

WORD32 xa_nn_maxpool_f32(
        FLOAT32 *__restrict__ p_out,
const   FLOAT32 *__restrict__ p_inp ,
        WORD32  input_height,
        WORD32  input_width,
        WORD32  input_channels,
        WORD32  kernel_height,
        WORD32  kernel_width,
        WORD32  x_stride,
        WORD32  y_stride,
        WORD32  x_padding,
        WORD32  y_padding,
        WORD32  out_height,
        WORD32  out_width,
        WORD32  out_data_format,
        VOID   *p_scratch);

WORD32 xa_nn_maxpool_getsize(
        WORD32 inp_precision,
        WORD32 input_width,
        WORD32 kernel_height,
        WORD32 kernel_width,
        WORD32 x_stride,
        WORD32 y_stride,
        WORD32 x_padding,
        WORD32 out_width);
#else
WORD32 xa_nn_maxpool_f32(
        FLOAT32 *__restrict__ p_out,
const   FLOAT32 *__restrict__ p_inp ,
        WORD32  input_height,
        WORD32  input_width,
        WORD32  input_channels,
        WORD32  kernel_height,
        WORD32  kernel_width,
        WORD32  x_stride,
        WORD32  y_stride,
        WORD32  x_padding,
        WORD32  y_padding,
        WORD32  out_height,
        WORD32  out_width,
        WORD32  inp_data_format,            /*!< [in] input data format: WHD */
        WORD32  out_data_format,
        VOID   *p_scratch);

WORD32 xa_nn_maxpool_getsize(
        WORD32 input_channels,
        WORD32 inp_precision,
        WORD32 out_precision,
        WORD32 input_height,
        WORD32 input_width,
        WORD32 kernel_height,
        WORD32 kernel_width,
        WORD32 x_stride,
        WORD32 y_stride,
        WORD32 x_padding,
        WORD32 y_padding,
        WORD32 out_height,
        WORD32 out_width,
        WORD32 inp_data_format,
        WORD32 out_data_format);

WORD32 xa_nn_maxpool_8(
    WORD8 *__restrict__ p_out,
    WORD8 *__restrict__ p_inp ,
    WORD32  input_height,
    WORD32  input_width,
    WORD32  input_channels,
    WORD32  kernel_height,
    WORD32  kernel_width,
    WORD32  x_stride,
    WORD32  y_stride,
    WORD32  x_padding,
    WORD32  y_padding,
    WORD32  out_height,
    WORD32  out_width,
    WORD32  inp_data_format,
    WORD32  out_data_format,
    VOID   *p_scratch);

WORD32 xa_nn_maxpool_16(
    WORD16 *__restrict__ p_out,
    WORD16 *__restrict__ p_inp ,
    WORD32  input_height,
    WORD32  input_width,
    WORD32  input_channels,
    WORD32  kernel_height,
    WORD32  kernel_width,
    WORD32  x_stride,
    WORD32  y_stride,
    WORD32  x_padding,
    WORD32  y_padding,
    WORD32  out_height,
    WORD32  out_width,
    WORD32  inp_data_format,
    WORD32  out_data_format,
    VOID   *p_scratch);
#endif


WORD32 xa_nn_fully_connected_f32
  (FLOAT32 *__restrict__ p_out
   ,const FLOAT32 *__restrict__ p_weight
   ,const FLOAT32 *__restrict__ p_inp
   ,const FLOAT32 *__restrict__ p_bias
   ,WORD32  weight_depth
   ,WORD32  out_depth
  );

WORD32 xa_nn_fully_connected_16x16_16
  (pWORD16 __restrict__ p_out
   ,pWORD16  __restrict__ p_weight
   ,pWORD16 __restrict__ p_inp
   ,pWORD16 __restrict__ p_bias
   ,WORD32  weight_depth
   ,WORD32  out_depth
   ,WORD32  acc_shift
   ,WORD32  bias_shift
  );

WORD32 xa_nn_fully_connected_8x16_16
  (pWORD16 __restrict__ p_out
   ,pWORD8  __restrict__ p_weight
   ,pWORD16 __restrict__ p_inp
   ,pWORD16 __restrict__ p_bias
   ,WORD32  weight_depth
   ,WORD32  out_depth
   ,WORD32  acc_shift
   ,WORD32  bias_shift
  );

WORD32 xa_nn_fully_connected_8x8_8
  (pWORD8 __restrict__ p_out
   ,pWORD8  __restrict__ p_weight
   ,pWORD8 __restrict__ p_inp
   ,pWORD8 __restrict__ p_bias
   ,WORD32  weight_depth
   ,WORD32  out_depth
   ,WORD32  acc_shift
   ,WORD32  bias_shift
  );

#ifdef NNLIB_V2

WORD32 xa_nn_vec_activation_min_max_asym8_asym8(
    UWORD8 * __restrict__ p_out, 
    const  UWORD8 * __restrict__ p_vec, 
    int    activation_min,
    int    activation_max,
    WORD32 vec_length);

WORD32 xa_nn_conv1d_std_asym8xasym8(
    UWORD8* __restrict__ p_out,
    UWORD8* __restrict__ p_inp,
    UWORD8* __restrict__ p_kernel,
    WORD32* __restrict__ p_bias,
    WORD32 input_height,
    WORD32 input_width,
    WORD32 input_channels,
    WORD32 kernel_height,
    WORD32 out_channels,
    WORD32 y_stride,
    WORD32 y_padding,
    WORD32 out_height,
    WORD32 input_zero_bias,
    WORD32 kernel_zero_bias,
    WORD32 out_multiplier,
    WORD32 out_shift,
    WORD32 out_zero_bias,
    WORD32 out_data_format,
    VOID *p_scratch);

WORD32 xa_nn_conv2d_std_asym8xasym8(
    UWORD8* __restrict__ p_out,
    const UWORD8* __restrict__ p_inp,
    const UWORD8* __restrict__ p_kernel,
    const WORD32* __restrict__ p_bias,
    WORD32 input_height,
    WORD32 input_width,
    WORD32 input_channels,
    WORD32 kernel_height,
    WORD32 kernel_width,
    WORD32 out_channels,
    WORD32 x_stride,
    WORD32 y_stride,
    WORD32 x_padding,
    WORD32 y_padding,
    WORD32 out_height,
    WORD32 out_width,
    WORD32 input_zero_bias,
    WORD32 kernel_zero_bias,
    WORD32 out_multiplier,
    WORD32 out_shift,
    WORD32 out_zero_bias,
    WORD32 out_data_format,
    VOID *p_scratch);

WORD32 xa_nn_matXvec_asym8xasym8_asym8(
    UWORD8 * __restrict__ p_out,
    const UWORD8 * __restrict__ p_mat1,
    const UWORD8 * __restrict__ p_mat2,
    const UWORD8 * __restrict__ p_vec1,
    const UWORD8 * __restrict__ p_vec2,
    const WORD32 * __restrict__ p_bias,
    WORD32 rows,
    WORD32 cols1,
    WORD32 cols2,
    WORD32 row_stride1,
    WORD32 row_stride2,
    WORD32 mat1_zero_bias,
    WORD32 mat2_zero_bias,
    WORD32 vec1_zero_bias,
    WORD32 vec2_zero_bias,
    WORD32 out_multiplier,
    WORD32 out_shift,
    WORD32 out_zero_bias
    );

WORD32 xa_nn_matXvec_batch_asym8xasym8_asym8(
    UWORD8 ** __restrict__ p_out,
    UWORD8 * __restrict__ p_mat1,
    UWORD8 ** __restrict__ p_vec1,
    WORD32 * __restrict__ p_bias,
    WORD32 rows,
    WORD32 cols1,
    WORD32 row_stride1,
    WORD32 vec_count,
    WORD32 mat1_zero_bias,
    WORD32 vec1_zero_bias,
    WORD32 out_multiplier,
    WORD32 out_shift,
    WORD32 out_zero_bias);

WORD32 xa_nn_conv2d_depthwise_asym8xasym8(
    pUWORD8 __restrict__ p_out,
    const UWORD8 *__restrict__ p_kernel,
    const UWORD8 *__restrict__ p_inp,
    const WORD32 *__restrict__ p_bias,
    WORD32  input_height,
    WORD32  input_width,
    WORD32  input_channels,
    WORD32  kernel_height,
    WORD32  kernel_width,
    WORD32  channels_multiplier,
    WORD32  x_stride,
    WORD32  y_stride,
    WORD32  x_padding,
    WORD32  y_padding,
    WORD32  out_height,
    WORD32  out_width,
    WORD32  input_zero_bias,
    WORD32  kernel_zero_bias,
    WORD32  out_multiplier,
    WORD32  out_shift,
    WORD32  out_zero_bias,
    WORD32  inp_data_format,
    WORD32  out_data_format,
    pVOID p_scratch);

WORD32 xa_nn_conv2d_pointwise_asym8xasym8
  (pUWORD8 __restrict__ p_out
   ,pUWORD8  __restrict__ p_kernel
   ,pUWORD8 __restrict__ p_inp
   ,pWORD32 __restrict__ p_bias
   ,WORD32  input_height
   ,WORD32  input_width
   ,WORD32  input_channels
   ,WORD32  out_channels
   ,WORD32  input_zero_bias
   ,WORD32  kernel_zero_bias
   ,WORD32  out_multiplier
   ,WORD32  out_shift
   ,WORD32  out_zero_bias
   ,WORD32  out_data_format);

WORD32 xa_nn_vec_activation_min_max_f32_f32(FLOAT32 * __restrict__ p_out, 
           const  FLOAT32 * __restrict__ p_vec, 
                  FLOAT32 activation_min,
                  FLOAT32 activation_max,
                  WORD32  vec_length);

WORD32 xa_nn_maxpool_asym8(
        UWORD8* __restrict__ p_out,
const   UWORD8* __restrict__ p_inp,
        WORD32  input_height,
        WORD32  input_width,
        WORD32  input_channels,
        WORD32  kernel_height,
        WORD32  kernel_width,
        WORD32  x_stride,
        WORD32  y_stride,
        WORD32  x_padding,
        WORD32  y_padding,
        WORD32  out_height,
        WORD32  out_width,
        WORD32  inp_data_format,
        WORD32  out_data_format,
        VOID   *p_scratch);

WORD32 xa_nn_avgpool_asym8(
        UWORD8* __restrict__ p_out,
const   UWORD8* __restrict__ p_inp,
        WORD32  input_height,
        WORD32  input_width,
        WORD32  input_channels,
        WORD32  kernel_height,
        WORD32  kernel_width,
        WORD32  x_stride,
        WORD32  y_stride,
        WORD32  x_padding,
        WORD32  y_padding,
        WORD32  out_height,
        WORD32  out_width,
        WORD32  inp_data_format,
        WORD32  out_data_format,
        VOID *p_scratch);

WORD32 xa_nn_fully_connected_asym8xasym8_asym8
  (pUWORD8 __restrict__ p_out
   ,const UWORD8 *__restrict__ p_weight
   ,const UWORD8 *__restrict__ p_inp
   ,const WORD32 *__restrict__ p_bias
   ,WORD32  weight_depth
   ,WORD32  out_depth
   ,WORD32  input_zero_bias
   ,WORD32  weight_zero_bias
   ,WORD32  out_multiplier
   ,WORD32  out_shift
   ,WORD32  out_zero_bias
  );

WORD32 xa_nn_elm_mul_f32xf32_f32(FLOAT32 * __restrict__ p_out, 
                               const FLOAT32 * __restrict__ p_inp1, 
                               const FLOAT32 * __restrict__ p_inp2, 
                               WORD32 num_elm);

WORD32 xa_nn_elm_add_f32xf32_f32(FLOAT32 * __restrict__ p_out, 
                               const FLOAT32 * __restrict__ p_inp1, 
                               const FLOAT32 * __restrict__ p_inp2, 
                               WORD32 num_elm);

WORD32 xa_nn_elm_mul_acc_f32xf32_f32(FLOAT32 * __restrict__ p_out, 
                               const FLOAT32 * __restrict__ p_inp1, 
                               const FLOAT32 * __restrict__ p_inp2, 
                               WORD32 num_elm);

WORD32 xa_nn_elm_sub_f32xf32_f32(FLOAT32 * __restrict__ p_out, 
                               const FLOAT32 * __restrict__ p_inp1, 
                               const FLOAT32 * __restrict__ p_inp2, 
                               WORD32 num_elm);

WORD32 xa_nn_elm_div_f32xf32_f32(FLOAT32 * __restrict__ p_out, 
                               const FLOAT32 * __restrict__ p_inp1, 
                               const FLOAT32 * __restrict__ p_inp2, 
                               WORD32 num_elm);

WORD32 xa_nn_elm_floor_f32_f32(FLOAT32 * __restrict__ p_out, 
                           const FLOAT32 * __restrict__ p_inp, 
                           WORD32 num_elm);

WORD32 xa_nn_elm_add_asym8xasym8_asym8(UWORD8 * __restrict__ p_out, 
                            WORD32  out_zero_bias,
                            WORD32  out_left_shift,
                            WORD32  out_multiplier,                
                            WORD32  out_activation_min,
                            WORD32  out_activation_max,
                    const   UWORD8 * __restrict__ p_inp1, 
                            WORD32  inp1_zero_bias,
                            WORD32  inp1_left_shift,
                            WORD32  inp1_multiplier,                
                    const   UWORD8 * __restrict__ p_inp2, 
                            WORD32  inp2_zero_bias,
                            WORD32  inp2_left_shift,
                            WORD32  inp2_multiplier,                
                            WORD32  left_shift,
                            WORD32  num_elm);

WORD32 xa_nn_elm_mul_asym8xasym8_asym8(UWORD8 * __restrict__ p_out, 
                            WORD32  out_zero_bias,
                            WORD32  out_shift,
                            WORD32  out_multiplier,                
                            WORD32  out_activation_min,
                            WORD32  out_activation_max,
                    const   UWORD8 * __restrict__ p_inp1, 
                            WORD32  inp1_zero_bias,
                    const   UWORD8 * __restrict__ p_inp2, 
                            WORD32  inp2_zero_bias,
                            WORD32  num_elm);

WORD32 xa_nn_vec_softmax_asym8_asym8( UWORD8 * __restrict__ p_out, 
                    const   UWORD8 * __restrict__ p_vec, 
                            WORD32   diffmin,
                            WORD32  input_left_shift,
                            WORD32  input_multiplier,                
                            WORD32  vec_length,
                            pVOID   p_scratch);

WORD32 xa_nn_vec_sigmoid_asym8_asym8(UWORD8 *p_out,
                      const UWORD8 *p_vec,
                            WORD32 zero_point,
                            WORD32 input_range_radius,
                            WORD32 input_multiplier,
                            WORD32 input_left_shift,
                            WORD32 vec_length);

int get_softmax_scratch_size(int inp_precision, int out_precision, int length);

WORD32 xa_nn_l2_norm_f32(
    FLOAT32 * __restrict__ p_out,
    const FLOAT32 * __restrict__ p_inp,
    WORD32 num_elm);

#endif /* NNLIB_V2 */

#if defined(__cplusplus)
}
#endif
#endif /* __XA_NNLIB_KERNELS_API_H__ */
