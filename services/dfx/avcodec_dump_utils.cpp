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

#include "avcodec_dump_utils.h"
#include "avcodec_errors.h"
#include "avcodec_log.h"

namespace {
    constexpr OHOS::HiviewDFX::HiLogLabel LABEL = {LOG_CORE, LOG_DOMAIN, "AVCodecDumpUtils"};
}

namespace OHOS {
namespace Media {

int32_t AVCodecDumpControler::AddInfo(const uint32_t dumpIdx, const std::string &name, const std::string &value)
{
    CHECK_AND_RETURN_RET_LOG((dumpIdx >> 24) > 0, AVCS_ERR_INVALID_VAL, "Add dump info failed, get a invalid dump index.");
    CHECK_AND_RETURN_RET_LOG(!name.empty(), AVCS_ERR_INVALID_VAL, "Add dump info failed, get a empty name.");
    if (dumpInfoMap_.find(dumpIdx) != dumpInfoMap_.end()) {
        AVCODEC_LOGW("Dump info index already exist, index: %{public}d", dumpIdx);
        return AVCS_ERR_OK;
    }

    int32_t level = GetLevel(dumpIdx);
    length_[level - 1] = length_[level - 1] > name.length() ? length_[level - 1] : name.length();
    dumpInfoMap_.emplace(dumpIdx, make_pair(name, value));
    return AVCS_ERR_OK;
}

int32_t AVCodecDumpControler::AddInfoFromFormat(const uint32_t dumpIdx, const Format &format, const std::string_view &key, const std::string &name)
{
    CHECK_AND_RETURN_RET_LOG(!key.empty(), AVCS_ERR_INVALID_VAL, "Add dump info failed, get a empty key.");

    std::string value;
    switch (format.GetValueType(key)) {
            case FORMAT_TYPE_INT32: {
                int32_t valueTemp = 0;
                format.GetIntValue(key, valueTemp);
                value = std::to_string(valueTemp);
                break;
            }
            case FORMAT_TYPE_INT64: {
                int64_t valueTemp = 0;
                format.GetLongValue(key, valueTemp);
                value = std::to_string(valueTemp);
                break;
            }
            case FORMAT_TYPE_FLOAT: {
                float valueTemp = 0;
                format.GetFloatValue(key, valueTemp);
                value = std::to_string(valueTemp);
                break;
            }
            case FORMAT_TYPE_DOUBLE: {
                double valueTemp = 0;
                format.GetDoubleValue(key, valueTemp);
                value = std::to_string(valueTemp);
                break;
            }
            case FORMAT_TYPE_STRING: {
                format.GetStringValue(key, value);
                break;
            }
            case FORMAT_TYPE_ADDR:
                break;
            default:
                AVCODEC_LOGE("Add info from format failed. Key: %{public}s", key.data());
    }

    this->AddInfo(dumpIdx, name, value);
    return AVCS_ERR_OK;
}

int32_t AVCodecDumpControler::GetDumpString(std::string &dumpString)
{
    for (auto iter : dumpInfoMap_) {
        int level = GetLevel(iter.first);
        std::string name = iter.second.first;
        std::string value = iter.second.second;
        dumpString += std::string((level - 1) * 4, ' ') + name + std::string(length_[level - 1] - name.length(), ' ');
        if (!value.empty()) {
            dumpString +=  " - " + value;
        }
        dumpString += std::string("\n");
    }
    return 0;
}

uint32_t AVCodecDumpControler::GetLevel(const uint32_t dumpIdx) 
{
    int level = 1;
    if (dumpIdx & 0xFF) {
        level = 4;
    } else if ((dumpIdx >> 8) & 0xFF) {
        level = 3;
    } else if ((dumpIdx >> 16) & 0xFF) {
        level = 2;
    }
    return level;
}

} // namespace OHOS
} // namespace Media