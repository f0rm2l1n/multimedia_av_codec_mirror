/*
 * Copyright (C) 2023 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef NATIVE_AVCODEC_LIST_H
#define NATIVE_AVCODEC_LIST_H

#include <stdint.h>
#include <stdio.h>
#include "native_avformat.h"
#include "native_avcodec_base.h"
#include "native_avcapability.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Find the supported encoder name by format(must contains video or audio MIME).
 * @param format Indicates a media description which contains required encoder capability.
 * @return  Returns encoder name, if not find, return null.
 * @since 10
 * @version 1.0
 */
const char *OH_AVCodec_FindEncoder(const OH_AVFormat *format);

/**
 * @brief Find the supported decoder name by format(must contains video or audio MIME).
 * @param format Indicates a media description which contains required decoder capability.
 * @return  Returns decoder name, if not find, return null.
 * @since 10
 * @version 1.0
 */
const char *OH_AVCodec_FindDecoder(const OH_AVFormat *format);

/**
 * @brief Create a capability by codec name
 * @param codeName Codec name
 * @return A OH_AVCapability instance
 * @since 10
 * @version 1.0
 */
OH_AVCapability *OH_AVCodec_CreateCapability(const char *name);

/**
 * @brief Destroy the capability
 * @param capability codec capability get from OH_AVCodec_GetCapability
 * @return Returns AV_ERR_OK if the execution is successful,
 * otherwise returns a specific error code, refer to {@link OH_AVErrCode}
 * @since 10
 * @version 1.0
 */
OH_AVErrCode OH_AVCodec_DestroyCapability(OH_AVCapability *capability);

#ifdef __cplusplus
}
#endif

#endif // NATIVE_AVCODEC_LIST_H