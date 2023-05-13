/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#ifndef PLUGIN_INTF_PLUGIN_BASE_H
#define PLUGIN_INTF_PLUGIN_BASE_H

#include <memory>
#include <string>

namespace OHOS {
namespace Media {
namespace Plugin {
/**
 * @brief Base class of a plugin. All plugins of different types inherit this interface.
 *
 * @details The base class contains only common operation methods and defines basic operation processes.
 * Different operations are valid only in the corresponding states. The timing of calls is guaranteed by
 * the plugin framework. Some operations also change the plugin status.
 * For details, see the description of each function.
 *
 * @since 10
 * @version 1.0
 */
struct PluginBase {
    /// Constructor
    explicit PluginBase(std::string name): pluginName_(std::move(name)) {}

    /// Destructor
    virtual ~PluginBase() = default;

    /**
     * @brief Get plugin name
     *
     * @return plugin name
     */
    std::string GetName() const
    {
        return pluginName_;
    }

protected:
    const std::string pluginName_;
};
} // namespace Plugin
} // namespace Media
} // namespace OHOS
#endif // PLUGIN_INTF_PLUGIN_BASE_H
