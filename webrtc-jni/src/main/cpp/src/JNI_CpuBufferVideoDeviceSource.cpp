/*
 * Copyright 2025 SLB
 *
 * Maintained by Yuri Vanzine
 *
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

#include "JNI_CpuBufferVideoDeviceSource.h"
#include "media/video/CpuBufferVideoTrackDeviceSource.h"
#include "JavaRef.h"
#include "JavaObject.h"
#include "JavaUtils.h"

#include "api/video/i420_buffer.h"
#include <iostream>

JNIEXPORT void JNICALL Java_dev_onvoid_webrtc_media_video_CpuBufferVideoDeviceSource_dispose
(JNIEnv * env, jobject caller)
{
	jni::CpuBufferVideoTrackDeviceSource * videoSource = GetHandle<jni::CpuBufferVideoTrackDeviceSource>(env, caller);
	CHECK_HANDLE(videoSource);

	webrtc::RefCountReleaseStatus status = videoSource->Release();

	if (status != webrtc::RefCountReleaseStatus::kDroppedLastRef) {
		RTC_LOG(LS_WARNING) << "Native object was not deleted. A reference is still around somewhere.";
		std::cout << "Native object CpuBufferVideoTrackDeviceSource was not deleted. A reference is still around somewhere." << std::endl;
	}

	SetHandle<std::nullptr_t>(env, caller, nullptr);

	videoSource = nullptr;
}

JNIEXPORT void JNICALL Java_dev_onvoid_webrtc_media_video_CpuBufferVideoDeviceSource_initialize
(JNIEnv * env, jobject caller)
{
	rtc::scoped_refptr<jni::CpuBufferVideoTrackDeviceSource> videoSource = webrtc::make_ref_counted<jni::CpuBufferVideoTrackDeviceSource>();

	SetHandle(env, caller, videoSource.release());
}

JNIEXPORT void JNICALL Java_dev_onvoid_webrtc_media_video_CpuBufferVideoDeviceSource_pushFrame
(JNIEnv * env, jobject caller, jbyteArray yuvBuffer, jint width, jint height, jlong timestamp)
{
    jni::CpuBufferVideoTrackDeviceSource * videoSource = GetHandle<jni::CpuBufferVideoTrackDeviceSource>(env, caller);
    CHECK_HANDLE(videoSource);

    jsize bufferLen = env->GetArrayLength(yuvBuffer);
    jbyte* bufferPtr = env->GetByteArrayElements(yuvBuffer, nullptr);

    // Create I420Buffer from raw YUV data
    rtc::scoped_refptr<webrtc::I420Buffer> i420Buffer = webrtc::I420Buffer::Create(width, height);

    int ySize = width * height;
    int uSize = (width / 2) * (height / 2);
    int vSize = uSize;

    memcpy(i420Buffer->MutableDataY(), bufferPtr, ySize);
    memcpy(i420Buffer->MutableDataU(), bufferPtr + ySize, uSize);
    memcpy(i420Buffer->MutableDataV(), bufferPtr + ySize + uSize, vSize);

    env->ReleaseByteArrayElements(yuvBuffer, bufferPtr, JNI_ABORT);

    webrtc::VideoFrame frame = webrtc::VideoFrame::Builder()
        .set_video_frame_buffer(i420Buffer)
        .set_rotation(webrtc::kVideoRotation_0)
        .set_timestamp_us(timestamp)
        .build();

    videoSource->OnFrame(frame);
}

