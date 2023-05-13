#include <memory>
#include "avsource.h"
#include "native_avsource.h"
#include "native_avmagic.h"

struct AVSourceObject : public OH_AVSource {
    explicit AVSourceObject(const std::shared_ptr<OHOS::Media::AVSource> &source)
        : OH_AVSource(AVMagic::AVCODEC_MAGIC_AVSOURCE), source_(source) {}
    ~AVSourceObject() = default;

    const std::shared_ptr<OHOS::Media::AVSource> source_;
};