/* Copyright (c) 2010 The Chromium OS Authors. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 *
 * Splicing tests for the kernel image verification library.
 */

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "cryptolib.h"
#include "file_keys.h"
#include "kernel_image.h"
#include "test_common.h"
#include "utility.h"

#define FIRMWARE_KEY_BASE_NAME "testkeys/key_rsa2048"
#define KERNEL_KEY_BASE_NAME "testkeys/key_rsa1024"

const char* kFirmwareKeyPublicFile = FIRMWARE_KEY_BASE_NAME ".keyb";
const char* kFirmwareKeyFile = FIRMWARE_KEY_BASE_NAME ".pem";
const char* kKernelKeyPublicFile = KERNEL_KEY_BASE_NAME ".keyb";
const char* kKernelKeyFile = KERNEL_KEY_BASE_NAME ".pem";

void VerifyKernelSplicingTest()
{
  uint64_t len;
  KernelImage* image1 = NULL;
  KernelImage* image2 = NULL;
  uint8_t* kernel_blob = NULL;
  uint8_t* kernel_sign_key_buf = NULL;
  RSAPublicKey* firmware_key = RSAPublicKeyFromFile(kFirmwareKeyPublicFile);
  uint8_t* firmware_key_blob = BufferFromFile(kFirmwareKeyPublicFile, &len);
  kernel_sign_key_buf= BufferFromFile(kKernelKeyPublicFile, &len);
  image1 = GenerateTestKernelImage(3, /* RSA2048/SHA1 */
                                   0, /* RSA1024/SHA1 */
                                   kernel_sign_key_buf,
                                   1,  /* Kernel Key Version. */
                                   1,  /* Kernel Version */
                                   1000,  /* Kernel Size. */
                                   kFirmwareKeyFile,
                                   kKernelKeyFile,
                                   'K');  /* Kernel data fill. */
  image2 = GenerateTestKernelImage(3,  /* RSA2058/SHA1 */
                                   0, /* RSA1024/SHA1 */
                                   kernel_sign_key_buf,
                                   1,  /* Kernel Key Version. */
                                   2,  /* Kernel Version */
                                   1000,  /* Kernel Size */
                                   kFirmwareKeyFile,
                                   kKernelKeyFile,
                                   'L');  /* Different Kernel data fill. */
  /* Make sure the originals verify. */
  TEST_EQ(VerifyKernelImage(firmware_key, image1, 0),
          VERIFY_KERNEL_SUCCESS,
          "KernelImage kernel_data Original");
  TEST_EQ(VerifyKernelImage(firmware_key, image2, 0),
          VERIFY_KERNEL_SUCCESS,
          "KernelImage kernel_data Original");

  /* Splice kernel_data + kernel signature from [image1]
   * and put it into [image2]. */
  Memcpy(image2->kernel_signature, image1->kernel_signature,
         siglen_map[0]);
  Memcpy(image2->kernel_data, image1->kernel_data,
         image2->kernel_len);

  TEST_NEQ(VerifyKernelImage(firmware_key, image2, 0),
           VERIFY_KERNEL_SUCCESS,
           "KernelImage kernel_data Splicing");
  kernel_blob = GetKernelBlob(image2, &len);
  TEST_NEQ(VerifyKernel(firmware_key_blob, kernel_blob, 0),
          VERIFY_KERNEL_SUCCESS,
          "Kernel Blob kernel_data Splicing");
}

int main(int argc, char* argv[])
{
  int error_code = 0;
  VerifyKernelSplicingTest();
  if (!gTestSuccess)
    error_code = 255;
  return error_code;
}
