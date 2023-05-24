/*
 * Copyright (C) 2022 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <iostream>
#include <cstdio>

#include <atomic>
#include <fstream>
#include <thread>
#include <mutex>
#include <queue>
#include <string>

#include "gtest/gtest.h"
#include "videodec_ndk_sample.h"
#include "native_avcodec_videodecoder.h"
#include "native_avformat.h"
#include "native_averrors.h"
#include "native_avcodec_base.h"

using namespace std;
using namespace OHOS;
using namespace OHOS::Media;
using namespace testing::ext;
namespace OHOS {
    namespace Media {
        class ActsCodecApiNdkTest : public testing::Test {
        public:
            // SetUpTestCase: Called before all test cases
            static void SetUpTestCase(void);
            // TearDownTestCase: Called after all test case
            static void TearDownTestCase(void);
            // SetUp: Called before each test cases
            void SetUp(void);
            // TearDown: Called after each test cases
            void TearDown(void);
        };

        OH_AVCodec *vdec_ = NULL;

        const string INVALID_CODEC_NAME = "avdec_h264";
        const string CODEC_NAME = "video_decoder.avc";
        constexpr uint32_t DEFAULT_WIDTH = 1920;
        constexpr uint32_t DEFAULT_HEIGHT = 1080;
        constexpr uint32_t DEFAULT_FRAME_RATE = 30;

        void ActsCodecApiNdkTest::SetUpTestCase() {}
        void ActsCodecApiNdkTest::TearDownTestCase() {}
        void ActsCodecApiNdkTest::SetUp() {}
        void ActsCodecApiNdkTest::TearDown()
        {
            if (vdec_ != NULL){
                OH_VideoDecoder_Destroy(vdec_);
                vdec_ = nullptr;
            }

        }
    } // namespace Media
} // namespace OHOS


/**
 * @tc.number    : VIDEO_SWDEC_ILLEGAL_PARA_0100
 * @tc.name      : OH_VideoDecoder_FindDecoder para error
 * @tc.desc      : function test
 */
HWTEST_F(ActsCodecApiNdkTest, VIDEO_SWDEC_ILLEGAL_PARA_0100, TestSize.Level2)
{
    vdec_ = OH_VideoDecoder_CreateByMime(NULL);
    ASSERT_EQ(NULL, vdec_);
}

/**
 * @tc.number    : VIDEO_SWDEC_ILLEGAL_PARA_0200
 * @tc.name      : OH_VideoDecoder_CreateByName para error
 * @tc.desc      : function test
 */
HWTEST_F(ActsCodecApiNdkTest, VIDEO_SWDEC_ILLEGAL_PARA_0200, TestSize.Level2)
{
    vdec_ = OH_VideoDecoder_CreateByName(NULL);
    ASSERT_EQ(NULL, vdec_);
}

/**
 * @tc.number    : VIDEO_SWDEC_ILLEGAL_PARA_3300
 * @tc.name      : OH_VideoDecoder_SetCallback para error
 * @tc.desc      : function test
 */
HWTEST_F(ActsCodecApiNdkTest, VIDEO_SWDEC_ILLEGAL_PARA_3300, TestSize.Level2)
{
    OH_AVCodecAsyncCallback cb_;
    cb_.onError = VdecError;
    cb_.onStreamChanged = VdecFormatChanged;
    cb_.onNeedInputData = VdecInputDataReady;
    cb_.onNeedOutputData = VdecOutputDataReady;

    VDecSignal *signal_ = new VDecSignal();
    ASSERT_EQ(AV_ERR_INVALID_VAL, OH_VideoDecoder_SetCallback(NULL, cb_, static_cast<void *>(signal_)));
}

/**
 * @tc.number    : VIDEO_SWDEC_ILLEGAL_PARA_1800
 * @tc.name      : OH_VideoDecoder_SetCallback para error
 * @tc.desc      : function test
 */
HWTEST_F(ActsCodecApiNdkTest, VIDEO_SWDEC_ILLEGAL_PARA_1800, TestSize.Level2)
{
    vdec_ = OH_VideoDecoder_CreateByName(CODEC_NAME.c_str());
    ASSERT_NE(NULL, vdec_);

    OH_AVCodecAsyncCallback cb2_;
    cb2_.onError = NULL;
    cb2_.onStreamChanged = NULL;
    cb2_.onNeedInputData = NULL;
    cb2_.onNeedOutputData = NULL;
    VDecSignal *signal_ = new VDecSignal();
    ASSERT_EQ(AV_ERR_OK, OH_VideoDecoder_SetCallback(vdec_, cb2_, static_cast<void *>(signal_)));
}

/**
 * @tc.number    : VIDEO_SWDEC_ILLEGAL_PARA_0300
 * @tc.name      : OH_VideoDecoder_SetCallback para error
 * @tc.desc      : function test
 */
HWTEST_F(ActsCodecApiNdkTest, VIDEO_SWDEC_ILLEGAL_PARA_0300, TestSize.Level2)
{
    vdec_ = OH_VideoDecoder_CreateByName(CODEC_NAME.c_str());
    OH_AVCodecAsyncCallback cb_;
    cb_.onError = VdecError;
    cb_.onStreamChanged = VdecFormatChanged;
    cb_.onNeedInputData = VdecInputDataReady;
    cb_.onNeedOutputData = VdecOutputDataReady;
    ASSERT_EQ(AV_ERR_OK, OH_VideoDecoder_SetCallback(vdec_, cb_, NULL));
}

/**
 * @tc.number    : VIDEO_SWDEC_ILLEGAL_PARA_0400
 * @tc.name      : OH_VideoDecoder_Destroy para error
 * @tc.desc      : function test
 */
HWTEST_F(ActsCodecApiNdkTest, VIDEO_SWDEC_ILLEGAL_PARA_0400, TestSize.Level2)
{
    ASSERT_EQ(AV_ERR_INVALID_VAL, OH_VideoDecoder_Destroy(NULL));
}

/**
 * @tc.number    : VIDEO_SWDEC_ILLEGAL_PARA_0500
 * @tc.name      : OH_VideoDecoder_Configure para error
 * @tc.desc      : function test
 */
HWTEST_F(ActsCodecApiNdkTest, VIDEO_SWDEC_ILLEGAL_PARA_0500, TestSize.Level2)
{
    vdec_ = OH_VideoDecoder_CreateByName(CODEC_NAME.c_str());
    ASSERT_NE(NULL, vdec_);
    ASSERT_EQ(AV_ERR_INVALID_VAL, OH_VideoDecoder_Configure(vdec_, NULL));
}

/**
 * @tc.number    : VIDEO_SWDEC_ILLEGAL_PARA_1400
 * @tc.name      : OH_VideoDecoder_Configure para error
 * @tc.desc      : function test
 */
HWTEST_F(ActsCodecApiNdkTest, VIDEO_SWDEC_ILLEGAL_PARA_1400, TestSize.Level2)
{
    OH_AVFormat *format = OH_AVFormat_Create();
    ASSERT_NE(NULL, format);
    ASSERT_EQ(AV_ERR_INVALID_VAL, OH_VideoDecoder_Configure(NULL, format));
}

/**
 * @tc.number    : VIDEO_SWDEC_ILLEGAL_PARA_1400
 * @tc.name      : OH_VideoDecoder_Configure para error
 * @tc.desc      : function test
 */
HWTEST_F(ActsCodecApiNdkTest, VIDEO_SWDEC_ILLEGAL_PARA_5000, TestSize.Level2)
{
    OH_AVFormat *format = OH_AVFormat_Create();
    ASSERT_NE(NULL, format);

    string widthStr = "width";
    (void)OH_AVFormat_SetIntValue(format, widthStr.c_str(), DEFAULT_WIDTH);
    ASSERT_EQ(AV_ERR_INVALID_VAL, OH_VideoDecoder_Configure(NULL, format));
}

/**
 * @tc.number    : VIDEO_SWDEC_ILLEGAL_PARA_1400
 * @tc.name      : OH_VideoDecoder_Configure para error
 * @tc.desc      : function test
 */
HWTEST_F(ActsCodecApiNdkTest, VIDEO_SWDEC_ILLEGAL_PARA_5100, TestSize.Level2)
{
    OH_AVFormat *format = OH_AVFormat_Create();
    ASSERT_NE(NULL, format);

    string widthStr = "width";
    string heightStr = "height";
    string frameRateStr = "frame_rate";
    (void)OH_AVFormat_SetIntValue(format, widthStr.c_str(), DEFAULT_WIDTH);
    (void)OH_AVFormat_SetIntValue(format, heightStr.c_str(), DEFAULT_HEIGHT);
    (void)OH_AVFormat_SetIntValue(format, frameRateStr.c_str(), DEFAULT_FRAME_RATE);
    ASSERT_EQ(AV_ERR_INVALID_VAL, OH_VideoDecoder_Configure(NULL, format));
}

/**
 * @tc.number    : VIDEO_SWDEC_ILLEGAL_PARA_0600
 * @tc.name      : OH_VideoDecoder_Start para error
 * @tc.desc      : function test
 */
HWTEST_F(ActsCodecApiNdkTest, VIDEO_SWDEC_ILLEGAL_PARA_0600, TestSize.Level2)
{
    ASSERT_EQ(AV_ERR_INVALID_VAL, OH_VideoDecoder_Start(NULL));
}

/**
 * @tc.number    : VIDEO_SWDEC_ILLEGAL_PARA_0700
 * @tc.name      : OH_VideoDecoder_Stop para error
 * @tc.desc      : function test
 */
HWTEST_F(ActsCodecApiNdkTest, VIDEO_SWDEC_ILLEGAL_PARA_0700, TestSize.Level2)
{
    ASSERT_EQ(AV_ERR_INVALID_VAL, OH_VideoDecoder_Stop(NULL));
}

/**
 * @tc.number    : VIDEO_SWDEC_ILLEGAL_PARA_0800
 * @tc.name      : OH_VideoDecoder_Flush para error
 * @tc.desc      : function test
 */
HWTEST_F(ActsCodecApiNdkTest, VIDEO_SWDEC_ILLEGAL_PARA_0800, TestSize.Level2)
{
    ASSERT_EQ(AV_ERR_INVALID_VAL, OH_VideoDecoder_Flush(NULL));
}

/**
 * @tc.number    : VIDEO_SWDEC_ILLEGAL_PARA_0900
 * @tc.name      : OH_VideoDecoder_Reset para error
 * @tc.desc      : function test
 */
HWTEST_F(ActsCodecApiNdkTest, VIDEO_SWDEC_ILLEGAL_PARA_0900, TestSize.Level2)
{
    ASSERT_EQ(AV_ERR_INVALID_VAL, OH_VideoDecoder_Reset(NULL));
}

/**
 * @tc.number    : VIDEO_SWDEC_ILLEGAL_PARA_1000
 * @tc.name      : OH_VideoDecoder_GetOutputDescription para error
 * @tc.desc      : function test
 */
HWTEST_F(ActsCodecApiNdkTest, VIDEO_SWDEC_ILLEGAL_PARA_1000, TestSize.Level2)
{
    ASSERT_EQ(NULL, OH_VideoDecoder_GetOutputDescription(NULL));
}

/**
 * @tc.number    : VIDEO_SWDEC_ILLEGAL_PARA_1100
 * @tc.name      : OH_VideoDecoder_SetParameter para error
 * @tc.desc      : function test
 */
HWTEST_F(ActsCodecApiNdkTest, VIDEO_SWDEC_ILLEGAL_PARA_1100, TestSize.Level2)
{
    vdec_ = OH_VideoDecoder_CreateByName(CODEC_NAME.c_str());
    ASSERT_EQ(AV_ERR_INVALID_VAL, OH_VideoDecoder_SetParameter(vdec_, NULL));
}

/**
 * @tc.number    : VIDEO_SWDEC_ILLEGAL_PARA_1500
 * @tc.name      : OH_VideoDecoder_SetParameter para error
 * @tc.desc      : function test
 */
HWTEST_F(ActsCodecApiNdkTest, VIDEO_SWDEC_ILLEGAL_PARA_1500, TestSize.Level2)
{
    OH_AVFormat *format = OH_AVFormat_Create();
    ASSERT_NE(NULL, format);

    string widthStr = "width";
    string heightStr = "height";
    string frameRateStr = "frame_rate";
    (void)OH_AVFormat_SetIntValue(format, widthStr.c_str(), DEFAULT_WIDTH);
    (void)OH_AVFormat_SetIntValue(format, heightStr.c_str(), DEFAULT_HEIGHT);
    (void)OH_AVFormat_SetIntValue(format, frameRateStr.c_str(), DEFAULT_FRAME_RATE);
    ASSERT_EQ(AV_ERR_INVALID_VAL, OH_VideoDecoder_SetParameter(NULL, format));
}

/**
 * @tc.number    : VIDEO_SWDEC_ILLEGAL_PARA_1200
 * @tc.name      : OH_VideoDecoder_SetSurface para error
 * @tc.desc      : function test
 */
HWTEST_F(ActsCodecApiNdkTest, VIDEO_SWDEC_ILLEGAL_PARA_1200, TestSize.Level2)
{
    vdec_ = OH_VideoDecoder_CreateByName(CODEC_NAME.c_str());
    ASSERT_EQ(AV_ERR_INVALID_VAL, OH_VideoDecoder_SetSurface(vdec_, NULL));
}

/**
 * @tc.number    : VIDEO_SWDEC_ILLEGAL_PARA_1300
 * @tc.name      : OH_VideoDecoder_IsValid para error
 * @tc.desc      : function test
 */
HWTEST_F(ActsCodecApiNdkTest, VIDEO_SWDEC_ILLEGAL_PARA_1300, TestSize.Level2)
{
    vdec_ = OH_VideoDecoder_CreateByName(CODEC_NAME.c_str());
}

/**
 * @tc.number    : VIDEO_SWDEC_ILLEGAL_PARA_1600
 * @tc.name      : OH_VideoDecoder_CreateByName para error
 * @tc.desc      : function test
 */
HWTEST_F(ActsCodecApiNdkTest, VIDEO_SWDEC_ILLEGAL_PARA_1600, TestSize.Level2)
{
    vdec_ = OH_VideoDecoder_CreateByName(INVALID_CODEC_NAME.c_str());
    ASSERT_EQ(NULL, vdec_);
}

/**
 * @tc.number    : VIDEO_SWDEC_ILLEGAL_PARA_1700
 * @tc.name      : OH_VideoDecoder_CreateByName para error
 * @tc.desc      : function test
 */
HWTEST_F(ActsCodecApiNdkTest, VIDEO_SWDEC_ILLEGAL_PARA_1700, TestSize.Level2)
{
    vdec_ = OH_VideoDecoder_CreateByMime(INVALID_CODEC_NAME.c_str());
    ASSERT_EQ(NULL, vdec_);
}

/**
 * @tc.number    : VIDEO_SWDEC_ILLEGAL_PARA_2500
 * @tc.name      : OH_VideoDecoder_RenderOutputData para error
 * @tc.desc      : function test
 */
HWTEST_F(ActsCodecApiNdkTest, VIDEO_SWDEC_ILLEGAL_PARA_2500, TestSize.Level2)
{
    ASSERT_EQ(AV_ERR_INVALID_VAL, OH_VideoDecoder_RenderOutputData(NULL, 0));
}

/**
 * @tc.number    : VIDEO_SWDEC_ILLEGAL_PARA_2600
 * @tc.name      : OH_VideoDecoder_RenderOutputData para error
 * @tc.desc      : function test
 */
HWTEST_F(ActsCodecApiNdkTest, VIDEO_SWDEC_ILLEGAL_PARA_2600, TestSize.Level2)
{
    vdec_ = OH_VideoDecoder_CreateByName(CODEC_NAME.c_str());
    ASSERT_EQ(AV_ERR_OPERATE_NOT_PERMIT, OH_VideoDecoder_RenderOutputData(vdec_, 0));
}

/**
 * @tc.number    : VIDEO_SWDEC_ILLEGAL_PARA_2700
 * @tc.name      : OH_VideoDecoder_FreeOutputData para error
 * @tc.desc      : function test
 */
HWTEST_F(ActsCodecApiNdkTest, VIDEO_SWDEC_ILLEGAL_PARA_2700, TestSize.Level2)
{
    ASSERT_EQ(AV_ERR_INVALID_VAL, OH_VideoDecoder_FreeOutputData(NULL, 0));
}

/**
 * @tc.number    : VIDEO_SWDEC_ILLEGAL_PARA_2800
 * @tc.name      : OH_VideoDecoder_FreeOutputData para error
 * @tc.desc      : function test
 */
HWTEST_F(ActsCodecApiNdkTest, VIDEO_SWDEC_ILLEGAL_PARA_2800, TestSize.Level2)
{
    vdec_ = OH_VideoDecoder_CreateByName(CODEC_NAME.c_str());
    ASSERT_EQ(AV_ERR_OPERATE_NOT_PERMIT, OH_VideoDecoder_FreeOutputData(vdec_, 0));
}

/**
 * @tc.number    : VIDEO_SWDEC_ILLEGAL_PARA_2900
 * @tc.name      : OH_VideoDecoder_FreeOutputData para error
 * @tc.desc      : function test
 */
HWTEST_F(ActsCodecApiNdkTest, VIDEO_SWDEC_ILLEGAL_PARA_2900, TestSize.Level2)
{
    vdec_ = OH_VideoDecoder_CreateByName(CODEC_NAME.c_str());
    ASSERT_EQ(AV_ERR_OPERATE_NOT_PERMIT, OH_VideoDecoder_FreeOutputData(vdec_, -1));
}

/**
 * @tc.number    : VIDEO_SWDEC_ILLEGAL_PARA_3000
 * @tc.name      : OH_VideoDecoder_FreeOutputData para error
 * @tc.desc      : function test
 */
HWTEST_F(ActsCodecApiNdkTest, VIDEO_SWDEC_ILLEGAL_PARA_3000, TestSize.Level2)
{
    vdec_ = OH_VideoDecoder_CreateByName(CODEC_NAME.c_str());

    OH_AVCodecBufferAttr attr;
    attr.pts = -1;
    attr.size = -1;
    attr.offset = -1;
    attr.flags = AVCODEC_BUFFER_FLAGS_EOS;

    ASSERT_EQ(AV_ERR_INVALID_VAL, OH_VideoDecoder_PushInputData(vdec_, 0, attr));
}

/**
 * @tc.number    : VIDEO_SWDEC_ILLEGAL_PARA_3100
 * @tc.name      : OH_VideoDecoder_FreeOutputData para error
 * @tc.desc      : function test
 */
HWTEST_F(ActsCodecApiNdkTest, VIDEO_SWDEC_ILLEGAL_PARA_3100, TestSize.Level2)
{
    OH_AVCodecBufferAttr attr;
    attr.pts = 0;
    attr.size = 0;
    attr.offset = 0;
    attr.flags = AVCODEC_BUFFER_FLAGS_EOS;

    ASSERT_EQ(AV_ERR_INVALID_VAL, OH_VideoDecoder_PushInputData(NULL, -1, attr));
}

/**
 * @tc.number    : VIDEO_SWDEC_ILLEGAL_PARA_3200
 * @tc.name      : OH_VideoDecoder_FreeOutputData para error
 * @tc.desc      : function test
 */
HWTEST_F(ActsCodecApiNdkTest, VIDEO_SWDEC_ILLEGAL_PARA_3200, TestSize.Level2)
{
    OH_AVCodecBufferAttr attr;
    attr.pts = 0;
    attr.size = 0;
    attr.offset = 0;
    attr.flags = AVCODEC_BUFFER_FLAGS_EOS;

    ASSERT_EQ(AV_ERR_INVALID_VAL, OH_VideoDecoder_PushInputData(NULL, 0, attr));
}

/**
 * @tc.number    : VIDEO_SWDEC_API_0100
 * @tc.name      : create create
 * @tc.desc      : function test
 */
HWTEST_F(ActsCodecApiNdkTest, VIDEO_SWDEC_API_0100, TestSize.Level2)
{
    vdec_ = OH_VideoDecoder_CreateByName(CODEC_NAME.c_str());
    ASSERT_NE(vdec_, NULL);
    vdec_ = OH_VideoDecoder_CreateByName(CODEC_NAME.c_str());
    ASSERT_NE(vdec_, NULL);
}

/**
 * @tc.number    : VIDEO_SWDEC_API_0200
 * @tc.name      : create configure configure
 * @tc.desc      : function test
 */
HWTEST_F(ActsCodecApiNdkTest, VIDEO_SWDEC_API_0200, TestSize.Level2)
{
    vdec_ = OH_VideoDecoder_CreateByName(CODEC_NAME.c_str());
    ASSERT_NE(NULL, vdec_);

    OH_AVFormat *format = OH_AVFormat_Create();
    ASSERT_NE(NULL, format);

    string widthStr = "width";
    string heightStr = "height";
    string frameRateStr = "frame_rate";
    (void)OH_AVFormat_SetIntValue(format, widthStr.c_str(), DEFAULT_WIDTH);
    (void)OH_AVFormat_SetIntValue(format, heightStr.c_str(), DEFAULT_HEIGHT);
    (void)OH_AVFormat_SetIntValue(format, frameRateStr.c_str(), DEFAULT_FRAME_RATE);

    ASSERT_EQ(AV_ERR_OK, OH_VideoDecoder_Configure(vdec_, format));
    ASSERT_EQ(AV_ERR_OPERATE_NOT_PERMIT, OH_VideoDecoder_Configure(vdec_, format));
}

/**
 * @tc.number    : VIDEO_SWDEC_API_0300
 * @tc.name      : create configure start start
 * @tc.desc      : function test
 */
HWTEST_F(ActsCodecApiNdkTest, VIDEO_SWDEC_API_0300, TestSize.Level2)
{
    vdec_ = OH_VideoDecoder_CreateByName(CODEC_NAME.c_str());
    ASSERT_NE(NULL, vdec_);

    OH_AVFormat *format = OH_AVFormat_Create();
    ASSERT_NE(NULL, format);

    string widthStr = "width";
    string heightStr = "height";
    string frameRateStr = "frame_rate";
    (void)OH_AVFormat_SetIntValue(format, widthStr.c_str(), DEFAULT_WIDTH);
    (void)OH_AVFormat_SetIntValue(format, heightStr.c_str(), DEFAULT_HEIGHT);
    (void)OH_AVFormat_SetIntValue(format, frameRateStr.c_str(), DEFAULT_FRAME_RATE);

    ASSERT_EQ(AV_ERR_OK, OH_VideoDecoder_Configure(vdec_, format));
    ASSERT_EQ(AV_ERR_OK, OH_VideoDecoder_Start(vdec_));
    ASSERT_EQ(AV_ERR_OPERATE_NOT_PERMIT, OH_VideoDecoder_Start(vdec_));
}

/**
 * @tc.number    : VIDEO_SWDEC_API_0400
 * @tc.name      : create configure start stop stop
 * @tc.desc      : function test
 */
HWTEST_F(ActsCodecApiNdkTest, VIDEO_SWDEC_API_0400, TestSize.Level2)
{
    vdec_ = OH_VideoDecoder_CreateByName(CODEC_NAME.c_str());
    ASSERT_NE(NULL, vdec_);

    OH_AVFormat *format = OH_AVFormat_Create();
    ASSERT_NE(NULL, format);

    string widthStr = "width";
    string heightStr = "height";
    string frameRateStr = "frame_rate";
    (void)OH_AVFormat_SetIntValue(format, widthStr.c_str(), DEFAULT_WIDTH);
    (void)OH_AVFormat_SetIntValue(format, heightStr.c_str(), DEFAULT_HEIGHT);
    (void)OH_AVFormat_SetIntValue(format, frameRateStr.c_str(), DEFAULT_FRAME_RATE);

    ASSERT_EQ(AV_ERR_OK, OH_VideoDecoder_Configure(vdec_, format));
    ASSERT_EQ(AV_ERR_OK, OH_VideoDecoder_Start(vdec_));
    ASSERT_EQ(AV_ERR_OK, OH_VideoDecoder_Stop(vdec_));
    ASSERT_EQ(AV_ERR_OPERATE_NOT_PERMIT, OH_VideoDecoder_Stop(vdec_));
}

/**
 * @tc.number    : VIDEO_SWDEC_API_0500
 * @tc.name      : create configure start stop reset reset
 * @tc.desc      : function test
 */
HWTEST_F(ActsCodecApiNdkTest, VIDEO_SWDEC_API_0500, TestSize.Level2)
{
    vdec_ = OH_VideoDecoder_CreateByName(CODEC_NAME.c_str());
    ASSERT_NE(NULL, vdec_);

    OH_AVFormat *format = OH_AVFormat_Create();
    ASSERT_NE(NULL, format);

    string widthStr = "width";
    string heightStr = "height";
    string frameRateStr = "frame_rate";
    (void)OH_AVFormat_SetIntValue(format, widthStr.c_str(), DEFAULT_WIDTH);
    (void)OH_AVFormat_SetIntValue(format, heightStr.c_str(), DEFAULT_HEIGHT);
    (void)OH_AVFormat_SetIntValue(format, frameRateStr.c_str(), DEFAULT_FRAME_RATE);

    ASSERT_EQ(AV_ERR_OK, OH_VideoDecoder_Configure(vdec_, format));
    ASSERT_EQ(AV_ERR_OK, OH_VideoDecoder_Start(vdec_));
    ASSERT_EQ(AV_ERR_OK, OH_VideoDecoder_Stop(vdec_));
    ASSERT_EQ(AV_ERR_OK, OH_VideoDecoder_Reset(vdec_));
    ASSERT_EQ(AV_ERR_OPERATE_NOT_PERMIT, OH_VideoDecoder_Reset(vdec_));
}

/**
 * @tc.number    : VIDEO_SWDEC_API_0600
 * @tc.name      : create configure start EOS EOS
 * @tc.desc      : function test
 */
HWTEST_F(ActsCodecApiNdkTest, VIDEO_SWDEC_API_0600, TestSize.Level2)
{
    vdec_ = OH_VideoDecoder_CreateByName(CODEC_NAME.c_str());
    ASSERT_NE(NULL, vdec_);

    OH_AVFormat *format = OH_AVFormat_Create();
    ASSERT_NE(NULL, format);

    string widthStr = "width";
    string heightStr = "height";
    string frameRateStr = "frame_rate";
    (void)OH_AVFormat_SetIntValue(format, widthStr.c_str(), DEFAULT_WIDTH);
    (void)OH_AVFormat_SetIntValue(format, heightStr.c_str(), DEFAULT_HEIGHT);
    (void)OH_AVFormat_SetIntValue(format, frameRateStr.c_str(), DEFAULT_FRAME_RATE);

    ASSERT_EQ(AV_ERR_OK, OH_VideoDecoder_Configure(vdec_, format));
    ASSERT_EQ(AV_ERR_OK, OH_VideoDecoder_Start(vdec_));

    OH_AVCodecBufferAttr attr;
    attr.pts = 0;
    attr.size = 0;
    attr.offset = 0;
    attr.flags = AVCODEC_BUFFER_FLAGS_EOS;

    ASSERT_EQ(AV_ERR_OK, OH_VideoDecoder_PushInputData(vdec_, 0, attr));
    ASSERT_EQ(AV_ERR_OPERATE_NOT_PERMIT, OH_VideoDecoder_PushInputData(vdec_, 0, attr));
}

/**
 * @tc.number    : VIDEO_SWDEC_API_0700
 * @tc.name      : create configure start flush flush
 * @tc.desc      : function test
 */
HWTEST_F(ActsCodecApiNdkTest, VIDEO_SWDEC_API_0700, TestSize.Level2)
{
    vdec_ = OH_VideoDecoder_CreateByName(CODEC_NAME.c_str());
    ASSERT_NE(NULL, vdec_);

    OH_AVFormat *format = OH_AVFormat_Create();
    ASSERT_NE(NULL, format);

    string widthStr = "width";
    string heightStr = "height";
    string frameRateStr = "frame_rate";
    (void)OH_AVFormat_SetIntValue(format, widthStr.c_str(), DEFAULT_WIDTH);
    (void)OH_AVFormat_SetIntValue(format, heightStr.c_str(), DEFAULT_HEIGHT);
    (void)OH_AVFormat_SetIntValue(format, frameRateStr.c_str(), DEFAULT_FRAME_RATE);

    ASSERT_EQ(AV_ERR_OK, OH_VideoDecoder_Configure(vdec_, format));
    ASSERT_EQ(AV_ERR_OK, OH_VideoDecoder_Start(vdec_));
    ASSERT_EQ(AV_ERR_OK, OH_VideoDecoder_Flush(vdec_));
    ASSERT_EQ(AV_ERR_OPERATE_NOT_PERMIT, OH_VideoDecoder_Flush(vdec_));
}


/**
 * @tc.number    : VIDEO_SWDEC_API_0800
 * @tc.name      : create configure start stop release release
 * @tc.desc      : function test
 */
HWTEST_F(ActsCodecApiNdkTest, VIDEO_SWDEC_API_0800, TestSize.Level2)
{
    vdec_ = OH_VideoDecoder_CreateByName(CODEC_NAME.c_str());
    ASSERT_NE(NULL, vdec_);

    OH_AVFormat *format = OH_AVFormat_Create();
    ASSERT_NE(NULL, format);

    string widthStr = "width";
    string heightStr = "height";
    string frameRateStr = "frame_rate";
    (void)OH_AVFormat_SetIntValue(format, widthStr.c_str(), DEFAULT_WIDTH);
    (void)OH_AVFormat_SetIntValue(format, heightStr.c_str(), DEFAULT_HEIGHT);
    (void)OH_AVFormat_SetIntValue(format, frameRateStr.c_str(), DEFAULT_FRAME_RATE);

    ASSERT_EQ(AV_ERR_OK, OH_VideoDecoder_Configure(vdec_, format));
    ASSERT_EQ(AV_ERR_OK, OH_VideoDecoder_Start(vdec_));
    ASSERT_EQ(AV_ERR_OK, OH_VideoDecoder_Stop(vdec_));
    ASSERT_EQ(AV_ERR_OK, OH_VideoDecoder_Destroy(vdec_));
    vdec_ = nullptr;
    ASSERT_EQ(AV_ERR_INVALID_VAL, OH_VideoDecoder_Destroy(vdec_));
}
