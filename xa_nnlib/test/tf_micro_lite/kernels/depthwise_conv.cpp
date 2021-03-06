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

#include "tensorflow/lite/c/builtin_op_data.h"
#include "tensorflow/lite/c/c_api_internal.h"
#include "tensorflow/lite/kernels/internal/common.h"
#include "tensorflow/lite/kernels/internal/quantization_util.h"
#include "tensorflow/lite/kernels/internal/tensor_ctypes.h"
#include "tensorflow/lite/kernels/kernel_util.h"
#include "tensorflow/lite/kernels/padding.h"


#include "tensorflow/lite/kernels/internal/reference/depthwiseconv_float.h"
#ifndef HIFI_NNLIB_OPT
#include "tensorflow/lite/kernels/internal/reference/depthwiseconv_uint8.h"
#else
#include "xa_nnlib_api.h"
#endif

#ifdef PROFILE
#define PROF_ALLOCATE
#include <xtensa/config/core-isa.h>
#include "xt_profiler.h"
#endif

namespace tflite {
namespace ops {
namespace micro {
namespace depthwise_conv {
namespace {

constexpr int kInputTensor = 0;
constexpr int kFilterTensor = 1;
constexpr int kBiasTensor = 2;
constexpr int kOutputTensor = 0;

struct OpData {
  TfLitePaddingValues padding;
  // The scaling factor from input to output (aka the 'real multiplier') can
  // be represented as a fixed point multiplier plus a left shift.
  int32_t output_multiplier;
  int output_shift;
  // The range of the fused activation layer. For example for kNone and
  // uint8_t these would be 0 and 255.
  int32_t output_activation_min;
  int32_t output_activation_max;
};

TfLiteStatus CalculateOpData(TfLiteContext* context, TfLiteNode* node,
                             TfLiteDepthwiseConvParams* params, int width,
                             int height, int filter_width, int filter_height,
                             int out_width, int out_height,
                             const TfLiteType data_type, OpData* data) {
  data->padding.height = ComputePadding(params->stride_height, 1, height,
                                        filter_height, out_height);
  data->padding.width =
      ComputePadding(params->stride_width, 1, width, filter_width, out_width);

  // Note that quantized inference requires that all tensors have their
  // parameters set. This is usually done during quantized training.
  if (data_type != kTfLiteFloat32) {
    const TfLiteTensor* input = GetInput(context, node, kInputTensor);
    const TfLiteTensor* filter = GetInput(context, node, kFilterTensor);
    const TfLiteTensor* bias =
        GetOptionalInputTensor(context, node, kBiasTensor);
    TfLiteTensor* output = GetOutput(context, node, kOutputTensor);

    double real_multiplier = 0.0;
    TF_LITE_ENSURE_STATUS(GetQuantizedConvolutionMultipler(
        context, input, filter, bias, output, &real_multiplier));
    int exponent;
    QuantizeMultiplier(real_multiplier, &data->output_multiplier, &exponent);
    data->output_shift = -exponent;
    CalculateActivationRangeUint8(params->activation, output,
                                  &data->output_activation_min,
                                  &data->output_activation_max);
  }
  return kTfLiteOk;
}

}  // namespace

void* Init(TfLiteContext* context, const char* buffer, size_t length) {
  return nullptr;
}

void Free(TfLiteContext* context, void* buffer) {}

TfLiteStatus Prepare(TfLiteContext* context, TfLiteNode* node) {
  return kTfLiteOk;
}

void EvalFloat(TfLiteContext* context, TfLiteNode* node,
               TfLiteDepthwiseConvParams* params, OpData* data,
               const TfLiteTensor* input, const TfLiteTensor* filter,
               const TfLiteTensor* bias, TfLiteTensor* output) {
  float output_activation_min, output_activation_max;
  CalculateActivationRange(params->activation, &output_activation_min,
                           &output_activation_max);

  tflite::DepthwiseParams op_params;
  // Padding type is ignored, but still set.
  op_params.padding_type = PaddingType::kSame;
  op_params.padding_values.width = data->padding.width;
  op_params.padding_values.height = data->padding.height;
  op_params.stride_width = params->stride_width;
  op_params.stride_height = params->stride_height;
  op_params.dilation_width_factor = 1;
  op_params.dilation_height_factor = 1;
  op_params.depth_multiplier = params->depth_multiplier;
  op_params.float_activation_min = output_activation_min;
  op_params.float_activation_max = output_activation_max;

  tflite::reference_ops::DepthwiseConv(
      op_params, GetTensorShape(input), GetTensorData<float>(input),
      GetTensorShape(filter), GetTensorData<float>(filter),
      GetTensorShape(bias), GetTensorData<float>(bias), GetTensorShape(output),
      GetTensorData<float>(output));
}

void EvalQuantized(TfLiteContext* context, TfLiteNode* node,
                   TfLiteDepthwiseConvParams* params, OpData* data,
                   const TfLiteTensor* input, const TfLiteTensor* filter,
                   const TfLiteTensor* bias, TfLiteTensor* output) {
  const int32_t input_offset = -input->params.zero_point;
  const int32_t filter_offset = -filter->params.zero_point;
  const int32_t output_offset = output->params.zero_point;
#ifdef PROFILE
  char profiler_name[MAX_PROFILER_NAME_LENGTH];
  char profiler_params[MAX_PROFILER_PARAMS_LENGTH];
#endif
#ifdef HIFI_NNLIB_OPT
  uint8 *kernel;
  const uint8 *flt = GetTensorData<uint8_t>(filter);
  int i, j;
  int kh, kw, kc, kc_pad;
  void *p_scratch;
  int scratch_size;
#endif
  tflite::DepthwiseParams op_params;
  // Padding type is ignored, but still set.
  op_params.padding_type = PaddingType::kSame;
  op_params.padding_values.width = data->padding.width;
  op_params.padding_values.height = data->padding.height;
  op_params.stride_width = params->stride_width;
  op_params.stride_height = params->stride_height;
  op_params.dilation_width_factor = 1;
  op_params.dilation_height_factor = 1;
  op_params.depth_multiplier = params->depth_multiplier;
  op_params.quantized_activation_min = data->output_activation_min;
  op_params.quantized_activation_max = data->output_activation_max;
  op_params.input_offset = input_offset;
  op_params.weights_offset = filter_offset;
  op_params.output_offset = output_offset;
  op_params.output_multiplier = data->output_multiplier;
  // Legacy ops used mixed left and right shifts. Now all are +ve-means-left.
  op_params.output_shift = -data->output_shift;

#ifdef HIFI_NNLIB_OPT
  scratch_size = xa_nn_conv2d_depthwise_getsize(GetTensorShape(input).Dims(1),
          GetTensorShape(input).Dims(2),
          GetTensorShape(input).Dims(3),
          GetTensorShape(filter).Dims(1),
          GetTensorShape(filter).Dims(2),
          op_params.depth_multiplier,
          op_params.stride_width,
          op_params.stride_height,
          op_params.padding_values.width,
          op_params.padding_values.height,
          GetTensorShape(output).Dims(1),
          GetTensorShape(output).Dims(2),
          -3,
          0);
  p_scratch = malloc(scratch_size);
  /* Kernel pad */
  kh = GetTensorShape(filter).Dims(1);
  kw = GetTensorShape(filter).Dims(2);
  kc = GetTensorShape(filter).Dims(3);
  kc_pad = (GetTensorShape(filter).Dims(3)+3)&(~3);
  kernel = (uint8 *)malloc(kh*kw*kc_pad);
  /* Pad kernel depth to be multiple of 4 as required by NNLib */
  for(i = 0; i < kh*kw; i++) {
    for(j = 0; j < kc; j++) {
      kernel[i*kc_pad+j] = flt[i*kc+j];
    }
    for(; j < kc_pad; j++) {
      kernel[i*kc_pad+j] = (uint8_t)filter->params.zero_point;
    }
  }
#endif

#ifdef PROFILE
  {
      int total_macs;
      // Set profiler name 
      sprintf(profiler_name, "%s_asym8xasym8", "depthwise_conv");
      // Set profiler parameters                            
      sprintf(profiler_params, "input_height=%d, input_width=%d, input_channels=%d, kernel_height=%d, kernel_width=%d, out_channels=%d, out_height=%d, out_width=%d", 
              GetTensorShape(input).Dims(1), GetTensorShape(input).Dims(2), GetTensorShape(input).Dims(3), 
              GetTensorShape(filter).Dims(1), GetTensorShape(filter).Dims(2), 
              GetTensorShape(input).Dims(3)*op_params.depth_multiplier, GetTensorShape(output).Dims(1), GetTensorShape(output).Dims(2));
    
      total_macs = GetTensorShape(input).Dims(3)*op_params.depth_multiplier * GetTensorShape(output).Dims(1) * GetTensorShape(output).Dims(2)*
                   GetTensorShape(filter).Dims(1)*GetTensorShape(filter).Dims(2); 


      XTPWR_PROFILER_OPEN(0, profiler_name, profiler_params, total_macs, "Cycles/point", 0);
      XTPWR_PROFILER_START(0);
  }
#endif  

#ifndef HIFI_NNLIB_OPT
  tflite::reference_ops::DepthwiseConv(
      op_params, GetTensorShape(input), GetTensorData<uint8_t>(input),
      GetTensorShape(filter), GetTensorData<uint8_t>(filter),
      GetTensorShape(bias), GetTensorData<int32_t>(bias),
      GetTensorShape(output), GetTensorData<uint8_t>(output));
#else
  {
      int ret;
      ret = xa_nn_conv2d_depthwise_asym8xasym8(GetTensorData<uint8_t>(output),
              kernel, // GetTensorData<uint8_t>(filter),
              GetTensorData<uint8_t>(input),
              GetTensorData<int32_t>(bias),
              GetTensorShape(input).Dims(1),
              GetTensorShape(input).Dims(2),
              GetTensorShape(input).Dims(3),
              GetTensorShape(filter).Dims(1),
              GetTensorShape(filter).Dims(2),
              op_params.depth_multiplier,
              op_params.stride_width,
              op_params.stride_height,
              op_params.padding_values.width,
              op_params.padding_values.height,
              GetTensorShape(output).Dims(1),
              GetTensorShape(output).Dims(2),
              input_offset,
              filter_offset,
              op_params.output_multiplier,
              op_params.output_shift,
              op_params.output_offset,
              0,
              0,
              p_scratch);
  }
#endif

#ifdef PROFILE
  XTPWR_PROFILER_STOP(0);
  XTPWR_PROFILER_UPDATE(0);    
  XTPWR_PROFILER_PRINT(0); 
  XTPWR_PROFILER_CLOSE(0, 1);
#endif
}

TfLiteStatus Eval(TfLiteContext* context, TfLiteNode* node) {
  auto* params =
      reinterpret_cast<TfLiteDepthwiseConvParams*>(node->builtin_data);

  TfLiteTensor* output = GetOutput(context, node, kOutputTensor);
  const TfLiteTensor* input = GetInput(context, node, kInputTensor);
  const TfLiteTensor* filter = GetInput(context, node, kFilterTensor);
  const TfLiteTensor* bias =
      (NumInputs(node) == 3) ? GetInput(context, node, kBiasTensor) : nullptr;

  const TfLiteType data_type = input->type;
  int width = SizeOfDimension(input, 2);
  int height = SizeOfDimension(input, 1);
  int filter_width = SizeOfDimension(filter, 2);
  int filter_height = SizeOfDimension(filter, 1);
  int out_width = ComputeOutSize(params->padding, width, filter_width,
                                 params->stride_width);
  int out_height = ComputeOutSize(params->padding, height, filter_height,
                                  params->stride_height);
  OpData local_data_object;
  OpData* data = &local_data_object;
  TF_LITE_ENSURE_STATUS(CalculateOpData(context, node, params, width, height,
                                        filter_width, filter_height, out_width,
                                        out_height, data_type, data));

  // TODO(aselle): Consider whether float conv and quantized conv should be
  // separate ops to avoid dispatch overhead here.
  switch (input->type) {  // Already know in/out types are same.
    case kTfLiteFloat32:
      EvalFloat(context, node, params, data, input, filter, bias, output);
      break;
    case kTfLiteUInt8:
      EvalQuantized(context, node, params, data, input, filter, bias, output);
      break;
    default:
      context->ReportError(context, "Type %d not currently supported.",
                           input->type);
      return kTfLiteError;
  }
  return kTfLiteOk;
}

}  // namespace depthwise_conv

TfLiteRegistration* Register_DEPTHWISE_CONV_2D() {
  static TfLiteRegistration r = {depthwise_conv::Init, depthwise_conv::Free,
                                 depthwise_conv::Prepare, depthwise_conv::Eval};
  return &r;
}

}  // namespace micro
}  // namespace ops
}  // namespace tflite
