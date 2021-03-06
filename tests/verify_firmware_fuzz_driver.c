/* Copyright (c) 2010 The Chromium OS Authors. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 *
 * Utility for aiding fuzz testing of firmware image verification code.
 */

#include <stdint.h>
#include <stdio.h>

#include "file_keys.h"
#include "firmware_image.h"
#include "utility.h"

int VerifySignedFirmware(const char* root_key_file,
                         const char* verification_file,
                         const char* firmware_file) {
  int error, error_code = 0;
  uint64_t len;
  uint8_t* verification_blob = BufferFromFile(verification_file, &len);
  uint8_t* firmware_blob = BufferFromFile(firmware_file, &len);
  uint8_t* root_key_blob = BufferFromFile(root_key_file, &len);

  if (!root_key_blob) {
    fprintf(stderr, "Couldn't read pre-processed public root key.\n");
    error_code = 1;
  }

  if (!error_code && !firmware_blob) {
    fprintf(stderr, "Couldn't read firmware image.\n");
    error_code = 1;
  }

  if (!error_code && !verification_blob) {
    fprintf(stderr, "Couldn't read verification data image.\n");
    error_code = 1;
  }

  if (!error_code && (error = VerifyFirmware(root_key_blob,
                                             verification_blob,
                                             firmware_blob))) {
    fprintf(stderr, "%s\n", VerifyFirmwareErrorString(error));
    error_code = 1;
  }
  Free(root_key_blob);
  Free(firmware_blob);
  if (error_code)
    return 0;
  else
    return 1;
}

int main(int argc, char* argv[]) {
  if (argc != 4) {
    fprintf(stderr, "Usage: %s <verification blob> <image_to_verify> <root_keyb>"
            "\n", argv[0]);
    return -1;
  }
  if (VerifySignedFirmware(argv[3], argv[1], argv[2])) {
    fprintf(stderr, "Verification SUCCESS!\n");
    return 0;
  } else {
    fprintf(stderr, "Verification FAILURE!\n");
    return -1;
  }
}
