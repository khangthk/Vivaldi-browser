// Copyright 2024 Google LLC
//
// This source code is licensed under the BSD-style license found in the
// LICENSE file in the root directory of this source tree.
//
// Auto-generated file. Do not edit!
//   Specification: test/f16-f32acc-rdsum.yaml
//   Generator: tools/generate-rdsum-test.py


#include <xnnpack/common.h>
#include <xnnpack/reduce.h>
#include <xnnpack/isa-checks.h>
#include <xnnpack/microparams-init.h>

#include "rdsum-microkernel-tester.h"
#include <gtest/gtest.h>


#if XNN_ENABLE_ARM_FP16_VECTOR && (XNN_ARCH_ARM || XNN_ARCH_ARM64)
  TEST(F16_F32ACC_RDSUM_7P7X__NEONFP16ARITH_C16, channels_eq_16_2pass_fulltile) {
    TEST_REQUIRES_ARM_NEON_FP16_ARITH;
    RDSumMicrokernelTester()
      .rows(14)
      .channels(16)
      .Test(xnn_f16_f32acc_rdsum_ukernel_7p7x__neonfp16arith_c16, xnn_init_f16_f32acc_scale_scalar_params);
  }

  TEST(F16_F32ACC_RDSUM_7P7X__NEONFP16ARITH_C16, channels_eq_16_2pass_fulltile_with_input_stride) {
    TEST_REQUIRES_ARM_NEON_FP16_ARITH;
    RDSumMicrokernelTester()
      .rows(14)
      .channels(16)
      .input_stride(19)
      .Test(xnn_f16_f32acc_rdsum_ukernel_7p7x__neonfp16arith_c16, xnn_init_f16_f32acc_scale_scalar_params);
  }

  TEST(F16_F32ACC_RDSUM_7P7X__NEONFP16ARITH_C16, channels_eq_16_2pass_subtile) {
    TEST_REQUIRES_ARM_NEON_FP16_ARITH;
    for (size_t rows = 1; rows < 14; rows++) {
      RDSumMicrokernelTester()
        .rows(rows)
        .channels(16)
        .Test(xnn_f16_f32acc_rdsum_ukernel_7p7x__neonfp16arith_c16, xnn_init_f16_f32acc_scale_scalar_params);
    }
  }

  TEST(F16_F32ACC_RDSUM_7P7X__NEONFP16ARITH_C16, channels_eq_16_2pass_subtile_with_input_stride) {
    TEST_REQUIRES_ARM_NEON_FP16_ARITH;
    for (size_t rows = 1; rows < 14; rows++) {
      RDSumMicrokernelTester()
        .rows(rows)
        .channels(16)
        .input_stride(19)
        .Test(xnn_f16_f32acc_rdsum_ukernel_7p7x__neonfp16arith_c16, xnn_init_f16_f32acc_scale_scalar_params);
    }
  }

  TEST(F16_F32ACC_RDSUM_7P7X__NEONFP16ARITH_C16, channels_eq_16_multipass_fulltile) {
    TEST_REQUIRES_ARM_NEON_FP16_ARITH;
    for (size_t rows = 1; rows <= 35; rows += 7) {
      RDSumMicrokernelTester()
        .rows(rows)
        .channels(16)
        .Test(xnn_f16_f32acc_rdsum_ukernel_7p7x__neonfp16arith_c16, xnn_init_f16_f32acc_scale_scalar_params);
    }
  }

  TEST(F16_F32ACC_RDSUM_7P7X__NEONFP16ARITH_C16, channels_eq_16_multipass_fulltile_with_input_stride) {
    TEST_REQUIRES_ARM_NEON_FP16_ARITH;
    for (size_t rows = 1; rows <= 35; rows += 7) {
      RDSumMicrokernelTester()
        .rows(rows)
        .channels(16)
        .input_stride(19)
        .Test(xnn_f16_f32acc_rdsum_ukernel_7p7x__neonfp16arith_c16, xnn_init_f16_f32acc_scale_scalar_params);
    }
  }

  TEST(F16_F32ACC_RDSUM_7P7X__NEONFP16ARITH_C16, channels_div_16_2pass_fulltile) {
    TEST_REQUIRES_ARM_NEON_FP16_ARITH;
    for (size_t channels = 32; channels < 128; channels += 16) {
      RDSumMicrokernelTester()
        .rows(14)
        .channels(channels)
        .Test(xnn_f16_f32acc_rdsum_ukernel_7p7x__neonfp16arith_c16, xnn_init_f16_f32acc_scale_scalar_params);
    }
  }

  TEST(F16_F32ACC_RDSUM_7P7X__NEONFP16ARITH_C16, channels_div_16_2pass_subtile) {
    TEST_REQUIRES_ARM_NEON_FP16_ARITH;
    for (size_t channels = 32; channels < 128; channels += 16) {
      for (size_t rows = 1; rows < 14; rows++) {
        RDSumMicrokernelTester()
          .rows(rows)
          .channels(channels)
          .Test(xnn_f16_f32acc_rdsum_ukernel_7p7x__neonfp16arith_c16, xnn_init_f16_f32acc_scale_scalar_params);
      }
    }
  }

  TEST(F16_F32ACC_RDSUM_7P7X__NEONFP16ARITH_C16, channels_div_16_multipass_fulltile) {
    TEST_REQUIRES_ARM_NEON_FP16_ARITH;
    for (size_t channels = 32; channels < 128; channels += 16) {
      for (size_t rows = 1; rows <= 35; rows += 7) {
        RDSumMicrokernelTester()
          .rows(rows)
          .channels(channels)
          .Test(xnn_f16_f32acc_rdsum_ukernel_7p7x__neonfp16arith_c16, xnn_init_f16_f32acc_scale_scalar_params);
      }
    }
  }

  TEST(F16_F32ACC_RDSUM_7P7X__NEONFP16ARITH_C16, channels_div_16_multipass_fulltile_with_input_stride) {
    TEST_REQUIRES_ARM_NEON_FP16_ARITH;
    for (size_t channels = 32; channels < 128; channels += 16) {
      for (size_t rows = 1; rows <= 35; rows += 7) {
        RDSumMicrokernelTester()
          .rows(rows)
          .channels(channels)
          .input_stride(263)
          .Test(xnn_f16_f32acc_rdsum_ukernel_7p7x__neonfp16arith_c16, xnn_init_f16_f32acc_scale_scalar_params);
      }
    }
  }

  TEST(F16_F32ACC_RDSUM_7P7X__NEONFP16ARITH_C16, channels_lt_16_2pass_fulltile) {
    TEST_REQUIRES_ARM_NEON_FP16_ARITH;
    for (size_t channels = 1; channels < 16; channels++) {
      RDSumMicrokernelTester()
        .rows(14)
        .channels(channels)
        .Test(xnn_f16_f32acc_rdsum_ukernel_7p7x__neonfp16arith_c16, xnn_init_f16_f32acc_scale_scalar_params);
    }
  }

  TEST(F16_F32ACC_RDSUM_7P7X__NEONFP16ARITH_C16, channels_lt_16_2pass_subtile) {
    TEST_REQUIRES_ARM_NEON_FP16_ARITH;
    for (size_t channels = 1; channels < 16; channels++) {
      for (size_t rows = 1; rows < 14; rows++) {
        RDSumMicrokernelTester()
          .rows(rows)
          .channels(channels)
          .Test(xnn_f16_f32acc_rdsum_ukernel_7p7x__neonfp16arith_c16, xnn_init_f16_f32acc_scale_scalar_params);
      }
    }
  }

  TEST(F16_F32ACC_RDSUM_7P7X__NEONFP16ARITH_C16, channels_lt_16_multipass_fulltile) {
    TEST_REQUIRES_ARM_NEON_FP16_ARITH;
    for (size_t channels = 1; channels < 16; channels++) {
      for (size_t rows = 1; rows <= 35; rows += 7) {
        RDSumMicrokernelTester()
          .rows(rows)
          .channels(channels)
          .Test(xnn_f16_f32acc_rdsum_ukernel_7p7x__neonfp16arith_c16, xnn_init_f16_f32acc_scale_scalar_params);
      }
    }
  }

  TEST(F16_F32ACC_RDSUM_7P7X__NEONFP16ARITH_C16, channels_lt_16_multipass_fulltile_with_input_stride) {
    TEST_REQUIRES_ARM_NEON_FP16_ARITH;
    for (size_t channels = 1; channels < 16; channels++) {
      for (size_t rows = 1; rows <= 35; rows += 7) {
        RDSumMicrokernelTester()
          .rows(rows)
          .channels(channels)
          .input_stride(19)
          .Test(xnn_f16_f32acc_rdsum_ukernel_7p7x__neonfp16arith_c16, xnn_init_f16_f32acc_scale_scalar_params);
      }
    }
  }

  TEST(F16_F32ACC_RDSUM_7P7X__NEONFP16ARITH_C16, channels_gt_16_2pass_fulltile) {
    TEST_REQUIRES_ARM_NEON_FP16_ARITH;
    for (size_t channels = 17; channels < 32; channels++) {
      RDSumMicrokernelTester()
        .rows(14)
        .channels(channels)
        .Test(xnn_f16_f32acc_rdsum_ukernel_7p7x__neonfp16arith_c16, xnn_init_f16_f32acc_scale_scalar_params);
    }
  }

  TEST(F16_F32ACC_RDSUM_7P7X__NEONFP16ARITH_C16, channels_gt_16_2pass_subtile) {
    TEST_REQUIRES_ARM_NEON_FP16_ARITH;
    for (size_t channels = 17; channels < 32; channels++) {
      for (size_t rows = 1; rows < 14; rows++) {
        RDSumMicrokernelTester()
          .rows(rows)
          .channels(channels)
          .Test(xnn_f16_f32acc_rdsum_ukernel_7p7x__neonfp16arith_c16, xnn_init_f16_f32acc_scale_scalar_params);
      }
    }
  }

  TEST(F16_F32ACC_RDSUM_7P7X__NEONFP16ARITH_C16, channels_gt_16_multipass_fulltile) {
    TEST_REQUIRES_ARM_NEON_FP16_ARITH;
    for (size_t channels = 17; channels < 32; channels++) {
      for (size_t rows = 1; rows < 35; rows += 14) {
        RDSumMicrokernelTester()
          .rows(rows)
          .channels(channels)
          .Test(xnn_f16_f32acc_rdsum_ukernel_7p7x__neonfp16arith_c16, xnn_init_f16_f32acc_scale_scalar_params);
      }
    }
  }

  TEST(F16_F32ACC_RDSUM_7P7X__NEONFP16ARITH_C16, channels_gt_16_multipass_fulltile_with_input_stride) {
    TEST_REQUIRES_ARM_NEON_FP16_ARITH;
    for (size_t channels = 17; channels < 32; channels++) {
      for (size_t rows = 1; rows < 35; rows += 14) {
        RDSumMicrokernelTester()
          .rows(rows)
          .channels(channels)
          .input_stride(47)
          .Test(xnn_f16_f32acc_rdsum_ukernel_7p7x__neonfp16arith_c16, xnn_init_f16_f32acc_scale_scalar_params);
      }
    }
  }
#endif  // XNN_ENABLE_ARM_FP16_VECTOR && (XNN_ARCH_ARM || XNN_ARCH_ARM64)


#if XNN_ENABLE_ARM_FP16_VECTOR && (XNN_ARCH_ARM || XNN_ARCH_ARM64)
  TEST(F16_F32ACC_RDSUM_7P7X__NEONFP16ARITH_C32, channels_eq_32_2pass_fulltile) {
    TEST_REQUIRES_ARM_NEON_FP16_ARITH;
    RDSumMicrokernelTester()
      .rows(14)
      .channels(32)
      .Test(xnn_f16_f32acc_rdsum_ukernel_7p7x__neonfp16arith_c32, xnn_init_f16_f32acc_scale_scalar_params);
  }

  TEST(F16_F32ACC_RDSUM_7P7X__NEONFP16ARITH_C32, channels_eq_32_2pass_fulltile_with_input_stride) {
    TEST_REQUIRES_ARM_NEON_FP16_ARITH;
    RDSumMicrokernelTester()
      .rows(14)
      .channels(32)
      .input_stride(37)
      .Test(xnn_f16_f32acc_rdsum_ukernel_7p7x__neonfp16arith_c32, xnn_init_f16_f32acc_scale_scalar_params);
  }

  TEST(F16_F32ACC_RDSUM_7P7X__NEONFP16ARITH_C32, channels_eq_32_2pass_subtile) {
    TEST_REQUIRES_ARM_NEON_FP16_ARITH;
    for (size_t rows = 1; rows < 14; rows++) {
      RDSumMicrokernelTester()
        .rows(rows)
        .channels(32)
        .Test(xnn_f16_f32acc_rdsum_ukernel_7p7x__neonfp16arith_c32, xnn_init_f16_f32acc_scale_scalar_params);
    }
  }

  TEST(F16_F32ACC_RDSUM_7P7X__NEONFP16ARITH_C32, channels_eq_32_2pass_subtile_with_input_stride) {
    TEST_REQUIRES_ARM_NEON_FP16_ARITH;
    for (size_t rows = 1; rows < 14; rows++) {
      RDSumMicrokernelTester()
        .rows(rows)
        .channels(32)
        .input_stride(37)
        .Test(xnn_f16_f32acc_rdsum_ukernel_7p7x__neonfp16arith_c32, xnn_init_f16_f32acc_scale_scalar_params);
    }
  }

  TEST(F16_F32ACC_RDSUM_7P7X__NEONFP16ARITH_C32, channels_eq_32_multipass_fulltile) {
    TEST_REQUIRES_ARM_NEON_FP16_ARITH;
    for (size_t rows = 1; rows <= 35; rows += 7) {
      RDSumMicrokernelTester()
        .rows(rows)
        .channels(32)
        .Test(xnn_f16_f32acc_rdsum_ukernel_7p7x__neonfp16arith_c32, xnn_init_f16_f32acc_scale_scalar_params);
    }
  }

  TEST(F16_F32ACC_RDSUM_7P7X__NEONFP16ARITH_C32, channels_eq_32_multipass_fulltile_with_input_stride) {
    TEST_REQUIRES_ARM_NEON_FP16_ARITH;
    for (size_t rows = 1; rows <= 35; rows += 7) {
      RDSumMicrokernelTester()
        .rows(rows)
        .channels(32)
        .input_stride(37)
        .Test(xnn_f16_f32acc_rdsum_ukernel_7p7x__neonfp16arith_c32, xnn_init_f16_f32acc_scale_scalar_params);
    }
  }

  TEST(F16_F32ACC_RDSUM_7P7X__NEONFP16ARITH_C32, channels_div_32_2pass_fulltile) {
    TEST_REQUIRES_ARM_NEON_FP16_ARITH;
    for (size_t channels = 64; channels < 256; channels += 32) {
      RDSumMicrokernelTester()
        .rows(14)
        .channels(channels)
        .Test(xnn_f16_f32acc_rdsum_ukernel_7p7x__neonfp16arith_c32, xnn_init_f16_f32acc_scale_scalar_params);
    }
  }

  TEST(F16_F32ACC_RDSUM_7P7X__NEONFP16ARITH_C32, channels_div_32_2pass_subtile) {
    TEST_REQUIRES_ARM_NEON_FP16_ARITH;
    for (size_t channels = 64; channels < 256; channels += 32) {
      for (size_t rows = 1; rows < 14; rows++) {
        RDSumMicrokernelTester()
          .rows(rows)
          .channels(channels)
          .Test(xnn_f16_f32acc_rdsum_ukernel_7p7x__neonfp16arith_c32, xnn_init_f16_f32acc_scale_scalar_params);
      }
    }
  }

  TEST(F16_F32ACC_RDSUM_7P7X__NEONFP16ARITH_C32, channels_div_32_multipass_fulltile) {
    TEST_REQUIRES_ARM_NEON_FP16_ARITH;
    for (size_t channels = 64; channels < 256; channels += 32) {
      for (size_t rows = 1; rows <= 35; rows += 7) {
        RDSumMicrokernelTester()
          .rows(rows)
          .channels(channels)
          .Test(xnn_f16_f32acc_rdsum_ukernel_7p7x__neonfp16arith_c32, xnn_init_f16_f32acc_scale_scalar_params);
      }
    }
  }

  TEST(F16_F32ACC_RDSUM_7P7X__NEONFP16ARITH_C32, channels_div_32_multipass_fulltile_with_input_stride) {
    TEST_REQUIRES_ARM_NEON_FP16_ARITH;
    for (size_t channels = 64; channels < 256; channels += 32) {
      for (size_t rows = 1; rows <= 35; rows += 7) {
        RDSumMicrokernelTester()
          .rows(rows)
          .channels(channels)
          .input_stride(521)
          .Test(xnn_f16_f32acc_rdsum_ukernel_7p7x__neonfp16arith_c32, xnn_init_f16_f32acc_scale_scalar_params);
      }
    }
  }

  TEST(F16_F32ACC_RDSUM_7P7X__NEONFP16ARITH_C32, channels_lt_32_2pass_fulltile) {
    TEST_REQUIRES_ARM_NEON_FP16_ARITH;
    for (size_t channels = 1; channels < 32; channels++) {
      RDSumMicrokernelTester()
        .rows(14)
        .channels(channels)
        .Test(xnn_f16_f32acc_rdsum_ukernel_7p7x__neonfp16arith_c32, xnn_init_f16_f32acc_scale_scalar_params);
    }
  }

  TEST(F16_F32ACC_RDSUM_7P7X__NEONFP16ARITH_C32, channels_lt_32_2pass_subtile) {
    TEST_REQUIRES_ARM_NEON_FP16_ARITH;
    for (size_t channels = 1; channels < 32; channels++) {
      for (size_t rows = 1; rows < 14; rows++) {
        RDSumMicrokernelTester()
          .rows(rows)
          .channels(channels)
          .Test(xnn_f16_f32acc_rdsum_ukernel_7p7x__neonfp16arith_c32, xnn_init_f16_f32acc_scale_scalar_params);
      }
    }
  }

  TEST(F16_F32ACC_RDSUM_7P7X__NEONFP16ARITH_C32, channels_lt_32_multipass_fulltile) {
    TEST_REQUIRES_ARM_NEON_FP16_ARITH;
    for (size_t channels = 1; channels < 32; channels++) {
      for (size_t rows = 1; rows <= 35; rows += 7) {
        RDSumMicrokernelTester()
          .rows(rows)
          .channels(channels)
          .Test(xnn_f16_f32acc_rdsum_ukernel_7p7x__neonfp16arith_c32, xnn_init_f16_f32acc_scale_scalar_params);
      }
    }
  }

  TEST(F16_F32ACC_RDSUM_7P7X__NEONFP16ARITH_C32, channels_lt_32_multipass_fulltile_with_input_stride) {
    TEST_REQUIRES_ARM_NEON_FP16_ARITH;
    for (size_t channels = 1; channels < 32; channels++) {
      for (size_t rows = 1; rows <= 35; rows += 7) {
        RDSumMicrokernelTester()
          .rows(rows)
          .channels(channels)
          .input_stride(37)
          .Test(xnn_f16_f32acc_rdsum_ukernel_7p7x__neonfp16arith_c32, xnn_init_f16_f32acc_scale_scalar_params);
      }
    }
  }

  TEST(F16_F32ACC_RDSUM_7P7X__NEONFP16ARITH_C32, channels_gt_32_2pass_fulltile) {
    TEST_REQUIRES_ARM_NEON_FP16_ARITH;
    for (size_t channels = 33; channels < 64; channels++) {
      RDSumMicrokernelTester()
        .rows(14)
        .channels(channels)
        .Test(xnn_f16_f32acc_rdsum_ukernel_7p7x__neonfp16arith_c32, xnn_init_f16_f32acc_scale_scalar_params);
    }
  }

  TEST(F16_F32ACC_RDSUM_7P7X__NEONFP16ARITH_C32, channels_gt_32_2pass_subtile) {
    TEST_REQUIRES_ARM_NEON_FP16_ARITH;
    for (size_t channels = 33; channels < 64; channels++) {
      for (size_t rows = 1; rows < 14; rows++) {
        RDSumMicrokernelTester()
          .rows(rows)
          .channels(channels)
          .Test(xnn_f16_f32acc_rdsum_ukernel_7p7x__neonfp16arith_c32, xnn_init_f16_f32acc_scale_scalar_params);
      }
    }
  }

  TEST(F16_F32ACC_RDSUM_7P7X__NEONFP16ARITH_C32, channels_gt_32_multipass_fulltile) {
    TEST_REQUIRES_ARM_NEON_FP16_ARITH;
    for (size_t channels = 33; channels < 64; channels++) {
      for (size_t rows = 1; rows < 35; rows += 14) {
        RDSumMicrokernelTester()
          .rows(rows)
          .channels(channels)
          .Test(xnn_f16_f32acc_rdsum_ukernel_7p7x__neonfp16arith_c32, xnn_init_f16_f32acc_scale_scalar_params);
      }
    }
  }

  TEST(F16_F32ACC_RDSUM_7P7X__NEONFP16ARITH_C32, channels_gt_32_multipass_fulltile_with_input_stride) {
    TEST_REQUIRES_ARM_NEON_FP16_ARITH;
    for (size_t channels = 33; channels < 64; channels++) {
      for (size_t rows = 1; rows < 35; rows += 14) {
        RDSumMicrokernelTester()
          .rows(rows)
          .channels(channels)
          .input_stride(79)
          .Test(xnn_f16_f32acc_rdsum_ukernel_7p7x__neonfp16arith_c32, xnn_init_f16_f32acc_scale_scalar_params);
      }
    }
  }
#endif  // XNN_ENABLE_ARM_FP16_VECTOR && (XNN_ARCH_ARM || XNN_ARCH_ARM64)


#if XNN_ENABLE_ARM_FP16_VECTOR && (XNN_ARCH_ARM || XNN_ARCH_ARM64)
  TEST(F16_F32ACC_RDSUM_7P7X__NEONFP16ARITH_C64, channels_eq_64_2pass_fulltile) {
    TEST_REQUIRES_ARM_NEON_FP16_ARITH;
    RDSumMicrokernelTester()
      .rows(14)
      .channels(64)
      .Test(xnn_f16_f32acc_rdsum_ukernel_7p7x__neonfp16arith_c64, xnn_init_f16_f32acc_scale_scalar_params);
  }

  TEST(F16_F32ACC_RDSUM_7P7X__NEONFP16ARITH_C64, channels_eq_64_2pass_fulltile_with_input_stride) {
    TEST_REQUIRES_ARM_NEON_FP16_ARITH;
    RDSumMicrokernelTester()
      .rows(14)
      .channels(64)
      .input_stride(67)
      .Test(xnn_f16_f32acc_rdsum_ukernel_7p7x__neonfp16arith_c64, xnn_init_f16_f32acc_scale_scalar_params);
  }

  TEST(F16_F32ACC_RDSUM_7P7X__NEONFP16ARITH_C64, channels_eq_64_2pass_subtile) {
    TEST_REQUIRES_ARM_NEON_FP16_ARITH;
    for (size_t rows = 1; rows < 14; rows++) {
      RDSumMicrokernelTester()
        .rows(rows)
        .channels(64)
        .Test(xnn_f16_f32acc_rdsum_ukernel_7p7x__neonfp16arith_c64, xnn_init_f16_f32acc_scale_scalar_params);
    }
  }

  TEST(F16_F32ACC_RDSUM_7P7X__NEONFP16ARITH_C64, channels_eq_64_2pass_subtile_with_input_stride) {
    TEST_REQUIRES_ARM_NEON_FP16_ARITH;
    for (size_t rows = 1; rows < 14; rows++) {
      RDSumMicrokernelTester()
        .rows(rows)
        .channels(64)
        .input_stride(67)
        .Test(xnn_f16_f32acc_rdsum_ukernel_7p7x__neonfp16arith_c64, xnn_init_f16_f32acc_scale_scalar_params);
    }
  }

  TEST(F16_F32ACC_RDSUM_7P7X__NEONFP16ARITH_C64, channels_eq_64_multipass_fulltile) {
    TEST_REQUIRES_ARM_NEON_FP16_ARITH;
    for (size_t rows = 1; rows <= 35; rows += 7) {
      RDSumMicrokernelTester()
        .rows(rows)
        .channels(64)
        .Test(xnn_f16_f32acc_rdsum_ukernel_7p7x__neonfp16arith_c64, xnn_init_f16_f32acc_scale_scalar_params);
    }
  }

  TEST(F16_F32ACC_RDSUM_7P7X__NEONFP16ARITH_C64, channels_eq_64_multipass_fulltile_with_input_stride) {
    TEST_REQUIRES_ARM_NEON_FP16_ARITH;
    for (size_t rows = 1; rows <= 35; rows += 7) {
      RDSumMicrokernelTester()
        .rows(rows)
        .channels(64)
        .input_stride(67)
        .Test(xnn_f16_f32acc_rdsum_ukernel_7p7x__neonfp16arith_c64, xnn_init_f16_f32acc_scale_scalar_params);
    }
  }

  TEST(F16_F32ACC_RDSUM_7P7X__NEONFP16ARITH_C64, channels_div_64_2pass_fulltile) {
    TEST_REQUIRES_ARM_NEON_FP16_ARITH;
    for (size_t channels = 128; channels < 512; channels += 64) {
      RDSumMicrokernelTester()
        .rows(14)
        .channels(channels)
        .Test(xnn_f16_f32acc_rdsum_ukernel_7p7x__neonfp16arith_c64, xnn_init_f16_f32acc_scale_scalar_params);
    }
  }

  TEST(F16_F32ACC_RDSUM_7P7X__NEONFP16ARITH_C64, channels_div_64_2pass_subtile) {
    TEST_REQUIRES_ARM_NEON_FP16_ARITH;
    for (size_t channels = 128; channels < 512; channels += 64) {
      for (size_t rows = 1; rows < 14; rows++) {
        RDSumMicrokernelTester()
          .rows(rows)
          .channels(channels)
          .Test(xnn_f16_f32acc_rdsum_ukernel_7p7x__neonfp16arith_c64, xnn_init_f16_f32acc_scale_scalar_params);
      }
    }
  }

  TEST(F16_F32ACC_RDSUM_7P7X__NEONFP16ARITH_C64, channels_div_64_multipass_fulltile) {
    TEST_REQUIRES_ARM_NEON_FP16_ARITH;
    for (size_t channels = 128; channels < 512; channels += 64) {
      for (size_t rows = 1; rows <= 35; rows += 7) {
        RDSumMicrokernelTester()
          .rows(rows)
          .channels(channels)
          .Test(xnn_f16_f32acc_rdsum_ukernel_7p7x__neonfp16arith_c64, xnn_init_f16_f32acc_scale_scalar_params);
      }
    }
  }

  TEST(F16_F32ACC_RDSUM_7P7X__NEONFP16ARITH_C64, channels_div_64_multipass_fulltile_with_input_stride) {
    TEST_REQUIRES_ARM_NEON_FP16_ARITH;
    for (size_t channels = 128; channels < 512; channels += 64) {
      for (size_t rows = 1; rows <= 35; rows += 7) {
        RDSumMicrokernelTester()
          .rows(rows)
          .channels(channels)
          .input_stride(1031)
          .Test(xnn_f16_f32acc_rdsum_ukernel_7p7x__neonfp16arith_c64, xnn_init_f16_f32acc_scale_scalar_params);
      }
    }
  }

  TEST(F16_F32ACC_RDSUM_7P7X__NEONFP16ARITH_C64, channels_lt_64_2pass_fulltile) {
    TEST_REQUIRES_ARM_NEON_FP16_ARITH;
    for (size_t channels = 1; channels < 64; channels++) {
      RDSumMicrokernelTester()
        .rows(14)
        .channels(channels)
        .Test(xnn_f16_f32acc_rdsum_ukernel_7p7x__neonfp16arith_c64, xnn_init_f16_f32acc_scale_scalar_params);
    }
  }

  TEST(F16_F32ACC_RDSUM_7P7X__NEONFP16ARITH_C64, channels_lt_64_2pass_subtile) {
    TEST_REQUIRES_ARM_NEON_FP16_ARITH;
    for (size_t channels = 1; channels < 64; channels++) {
      for (size_t rows = 1; rows < 14; rows++) {
        RDSumMicrokernelTester()
          .rows(rows)
          .channels(channels)
          .Test(xnn_f16_f32acc_rdsum_ukernel_7p7x__neonfp16arith_c64, xnn_init_f16_f32acc_scale_scalar_params);
      }
    }
  }

  TEST(F16_F32ACC_RDSUM_7P7X__NEONFP16ARITH_C64, channels_lt_64_multipass_fulltile) {
    TEST_REQUIRES_ARM_NEON_FP16_ARITH;
    for (size_t channels = 1; channels < 64; channels++) {
      for (size_t rows = 1; rows <= 35; rows += 7) {
        RDSumMicrokernelTester()
          .rows(rows)
          .channels(channels)
          .Test(xnn_f16_f32acc_rdsum_ukernel_7p7x__neonfp16arith_c64, xnn_init_f16_f32acc_scale_scalar_params);
      }
    }
  }

  TEST(F16_F32ACC_RDSUM_7P7X__NEONFP16ARITH_C64, channels_lt_64_multipass_fulltile_with_input_stride) {
    TEST_REQUIRES_ARM_NEON_FP16_ARITH;
    for (size_t channels = 1; channels < 64; channels++) {
      for (size_t rows = 1; rows <= 35; rows += 7) {
        RDSumMicrokernelTester()
          .rows(rows)
          .channels(channels)
          .input_stride(67)
          .Test(xnn_f16_f32acc_rdsum_ukernel_7p7x__neonfp16arith_c64, xnn_init_f16_f32acc_scale_scalar_params);
      }
    }
  }

  TEST(F16_F32ACC_RDSUM_7P7X__NEONFP16ARITH_C64, channels_gt_64_2pass_fulltile) {
    TEST_REQUIRES_ARM_NEON_FP16_ARITH;
    for (size_t channels = 65; channels < 128; channels++) {
      RDSumMicrokernelTester()
        .rows(14)
        .channels(channels)
        .Test(xnn_f16_f32acc_rdsum_ukernel_7p7x__neonfp16arith_c64, xnn_init_f16_f32acc_scale_scalar_params);
    }
  }

  TEST(F16_F32ACC_RDSUM_7P7X__NEONFP16ARITH_C64, channels_gt_64_2pass_subtile) {
    TEST_REQUIRES_ARM_NEON_FP16_ARITH;
    for (size_t channels = 65; channels < 128; channels++) {
      for (size_t rows = 1; rows < 14; rows++) {
        RDSumMicrokernelTester()
          .rows(rows)
          .channels(channels)
          .Test(xnn_f16_f32acc_rdsum_ukernel_7p7x__neonfp16arith_c64, xnn_init_f16_f32acc_scale_scalar_params);
      }
    }
  }

  TEST(F16_F32ACC_RDSUM_7P7X__NEONFP16ARITH_C64, channels_gt_64_multipass_fulltile) {
    TEST_REQUIRES_ARM_NEON_FP16_ARITH;
    for (size_t channels = 65; channels < 128; channels++) {
      for (size_t rows = 1; rows < 35; rows += 14) {
        RDSumMicrokernelTester()
          .rows(rows)
          .channels(channels)
          .Test(xnn_f16_f32acc_rdsum_ukernel_7p7x__neonfp16arith_c64, xnn_init_f16_f32acc_scale_scalar_params);
      }
    }
  }

  TEST(F16_F32ACC_RDSUM_7P7X__NEONFP16ARITH_C64, channels_gt_64_multipass_fulltile_with_input_stride) {
    TEST_REQUIRES_ARM_NEON_FP16_ARITH;
    for (size_t channels = 65; channels < 128; channels++) {
      for (size_t rows = 1; rows < 35; rows += 14) {
        RDSumMicrokernelTester()
          .rows(rows)
          .channels(channels)
          .input_stride(149)
          .Test(xnn_f16_f32acc_rdsum_ukernel_7p7x__neonfp16arith_c64, xnn_init_f16_f32acc_scale_scalar_params);
      }
    }
  }
#endif  // XNN_ENABLE_ARM_FP16_VECTOR && (XNN_ARCH_ARM || XNN_ARCH_ARM64)
