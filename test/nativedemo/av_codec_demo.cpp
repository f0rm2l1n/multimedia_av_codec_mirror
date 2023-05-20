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

#include <climits>
#include <iostream>
#include <vector>
#include "avmuxer_demo_runner.h"
#include "avcodec_audio_decoder_inner_demo.h"
#include "avcodec_audio_encoder_inner_demo.h"
#include "avcodec_audio_decoder_demo.h"
#include "avcodec_audio_aac_encoder_demo.h"
#include "avcodec_audio_flac_encoder_demo.h"
#include "codeclist_demo.h"

using namespace OHOS;
using namespace OHOS::Media;
using namespace OHOS::Media::AudioDemo;
using namespace OHOS::Media::AudioFlacDemo;
using namespace OHOS::Media::AudioAacDemo;
using namespace OHOS::Media::InnerAudioDemo;
using namespace std;

static int RunAudioDecoder()
{
    cout << "Please select number for format (default AAC Decoder): " << endl;
    cout << "0: AAC" << endl;
    cout << "1: FLAC" << endl;
    cout << "2: MP3" << endl;
    cout << "3: VORBIS" << endl;
    string mode;
    AudioFormatType audioFormatType = TYPE_AAC;
    (void)getline(cin, mode);
    if (mode == "" || mode == "0") {
        audioFormatType = TYPE_AAC;
    } else if (mode == "1") {
        audioFormatType = TYPE_FLAC;
    } else if (mode == "2") {
        audioFormatType = TYPE_MP3;
    } else if (mode == "3") {
        audioFormatType = TYPE_VORBIS;
    }  else {
        cout << "no that selection" << endl;
        return 0;
    }
    auto audioDec = std::make_unique<ADecDemo>();
    if (audioDec) {
        audioDec->RunCase(audioFormatType);
    }
    cout << "demo audio decoder end" << endl;
    return 0;
}

static int RunAudioEncoder()
{
    cout << "Please select number for format (default AAC Encoder): " << endl;
    cout << "0: AAC" << endl;
    cout << "1: FLAC" << endl;

    string mode;
    (void)getline(cin, mode);
    if (mode == "" || mode == "0") {
        auto audioEnc = std::make_unique<AEncAacDemo>();
        if (audioEnc) {
            audioEnc->RunCase();
        }
    } else if (mode == "1") {
        auto audioEnc = std::make_unique<AEncFlacDemo>();
        if (audioEnc) {
            audioEnc->RunCase();
        }
    }  else {
        cout << "no that selection" << endl;
        return 0;
    }
    cout << "demo audio encoder end" << endl;
    return 0;
}

static int RunAudioInnerDecoder()
{
    cout << "Please select number for format (default AAC Decoder): " << endl;
    cout << "0: AAC" << endl;
    cout << "1: FLAC" << endl;
    cout << "2: MP3" << endl;
    cout << "3: VORBIS" << endl;
    string mode;
    (void)getline(cin, mode);
    if (mode == "" || mode == "0") {
        auto audioDec = std::make_unique<ADecInnerDemo>();
        if (audioDec) {
            audioDec->RunCase();
        }
    } else if (mode == "1") {
        auto audioDec = std::make_unique<ADecInnerDemo>();
        if (audioDec) {
            audioDec->RunCase();
        }
    } else if (mode == "2") {
        auto audioDec = std::make_unique<ADecInnerDemo>();
        if (audioDec) {
            audioDec->RunCase();
        }
    } else if (mode == "3") {
        auto audioDec = std::make_unique<ADecInnerDemo>();
        if (audioDec) {
            audioDec->RunCase();
        }
    }  else {
        cout << "no that selection" << endl;
        return 0;
    }
    cout << "demo audio decoder end" << endl;
    return 0;
}

static int RunAudioInnerEncoder()
{
    cout << "Please select number for format (default AAC Encoder): " << endl;
    cout << "0: AAC" << endl;
    cout << "1: FLAC" << endl;
    string mode;
    (void)getline(cin, mode);
    if (mode == "" || mode == "0") {
        auto audioEnc = std::make_unique<AEnInnerDemo>();
        if (audioEnc) {
            audioEnc->RunCase();
        }
    } else if (mode == "1") {
        auto audioEnc = std::make_unique<AEnInnerDemo>();
        if (audioEnc) {
            audioEnc->RunCase();
        }
    }  else {
        cout << "no that selection" << endl;
        return 0;
    }
    cout << "demo audio encoder end" << endl;
    return 0;
}

static int RunCodecList()
{
    auto codecList = std::make_unique<CodecListDemo>();
    if (codecList == nullptr) {
        cout << "codec list is null" << endl;
        return 0;
    }
    codecList->RunCase();
    cout << "codec list end" << endl;
    return 0;
}

static void OptionPrint()
{
    cout << "Please select a demo scenario number(default Audio Decoder): " << endl;
    cout << "0:Audio Decoder" << endl;
    cout << "1:Audio Encoder" << endl;
    cout << "2:Audio Inner Decoder" << endl;
    cout << "3:Audio Inner Encoder" << endl;
    cout << "4:muxer demo" << endl;
    cout << "6:codeclist" << endl;
}

int main(int argc, char *argv[])
{
    constexpr int minRequiredArgCount = 2;
    string path;
    if (argc >= minRequiredArgCount && argv[1] != nullptr) {
        path = argv[1];
    }
    OptionPrint();
    string mode;
    (void)getline(cin, mode);
    if (mode == "" || mode == "0") {
        (void)RunAudioDecoder();
    } else if (mode == "1") {
        (void)RunAudioEncoder();
    } else if (mode == "2") {
        (void)RunAudioInnerDecoder();
    } else if (mode == "3") {
        (void)RunAudioInnerEncoder();
    } else if (mode == "4") {
        (void)AvmuxerDemoCase();
    } else if (mode == "6") {
        (void)RunCodecList();
    }  else {
        cout << "no that selection" << endl;
    }
    return 0;
}
