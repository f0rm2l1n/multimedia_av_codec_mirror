/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
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

import {AsyncCallback} from './basic';

/**
 * Provides interfaces to napitest.
 *
 * @since 7
 */
declare namespace napitest {
	interface AVFileDescriptor {
        fd: number;
        offset?: number;
        length?: number;
    }
	
	function setSurfaceID(surfaceID: string, inJsFp: AVFileDescriptor, outFd: number, width: number, height: number, callback: AsyncCallback<string>): void;
	function setSurfaceID(surfaceID: string, inJsFp: AVFileDescriptor, outFd: number, width: number, height: number): Promise<string>;
   
	function getSurfaceID(callback: AsyncCallback<string>): void;
	function getSurfaceID(): Promise<string>;
}
export default napitest;

