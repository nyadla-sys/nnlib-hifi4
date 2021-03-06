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
/* Copyright 2017 The TensorFlow Authors. All Rights Reserved.

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
==============================================================================*/
#ifndef TENSORFLOW_LITE_KERNELS_PADDING_H_
#define TENSORFLOW_LITE_KERNELS_PADDING_H_

#include "tensorflow/lite/c/builtin_op_data.h"

namespace tflite {

inline int ComputePadding(int stride, int dilation_rate, int in_size,
                          int filter_size, int out_size) {
  int effective_filter_size = (filter_size - 1) * dilation_rate + 1;
  int padding = ((out_size - 1) * stride + effective_filter_size - in_size) / 2;
  return padding > 0 ? padding : 0;
}

// Matching GetWindowedOutputSize in TensorFlow.
inline int ComputeOutSize(TfLitePadding padding, int image_size,
                          int filter_size, int stride) {
  switch (padding) {
    case kTfLitePaddingSame:
      return (image_size + stride - 1) / stride;
    case kTfLitePaddingValid:
      return (image_size + stride - filter_size) / stride;
    default:
      return 0;
  }
}

inline TfLitePaddingValues ComputePaddingHeightWidth(
    int stride_height, int stride_width, int dilation_rate, int in_height,
    int in_width, int filter_height, int filter_width, TfLitePadding padding) {
  int out_width = ComputeOutSize(padding, in_width, filter_width, stride_width);
  int out_height =
      ComputeOutSize(padding, in_height, filter_height, stride_height);

  TfLitePaddingValues padding_values;
  padding_values.height =
      ComputePadding(stride_height, 1, in_height, filter_height, out_height);
  padding_values.width =
      ComputePadding(stride_width, 1, in_width, filter_width, out_width);
  return padding_values;
}
}  // namespace tflite

#endif  // TENSORFLOW_LITE_KERNELS_PADDING_H_
