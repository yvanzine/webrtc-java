/*
 * Copyright 2025 Your Name
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

#include "JNI_RandomBufferVideoDeviceSource.h"
#include "media/video/RandomBufferVideoTrackDeviceSource.h"
#include "JavaRef.h"
#include "JavaObject.h"
#include "JavaUtils.h"

JNIEXPORT void JNICALL Java_dev_onvoid_webrtc_media_video_RandomBufferVideoDeviceSource_start
(JNIEnv * env, jobject caller)
{
	jni::RandomBufferVideoTrackDeviceSource * videoSource = GetHandle<jni::RandomBufferVideoTrackDeviceSource>(env, caller);
	CHECK_HANDLE(videoSource);

	try {
		videoSource->start();
	}
	catch (...) {
		ThrowCxxJavaException(env);
	}
}

JNIEXPORT void JNICALL Java_dev_onvoid_webrtc_media_video_RandomBufferVideoDeviceSource_stop
(JNIEnv * env, jobject caller)
{
	jni::RandomBufferVideoTrackDeviceSource * videoSource = GetHandle<jni::RandomBufferVideoTrackDeviceSource>(env, caller);
	CHECK_HANDLE(videoSource);

	try {
		videoSource->stop();
	}
	catch (...) {
		ThrowCxxJavaException(env);
	}
}

JNIEXPORT void JNICALL Java_dev_onvoid_webrtc_media_video_RandomBufferVideoDeviceSource_dispose
(JNIEnv * env, jobject caller)
{
	jni::RandomBufferVideoTrackDeviceSource * videoSource = GetHandle<jni::RandomBufferVideoTrackDeviceSource>(env, caller);
	CHECK_HANDLE(videoSource);

	webrtc::RefCountReleaseStatus status = videoSource->Release();

	if (status != webrtc::RefCountReleaseStatus::kDroppedLastRef) {
		RTC_LOG(LS_WARNING) << "Native object was not deleted. A reference is still around somewhere.";
	}

	SetHandle<std::nullptr_t>(env, caller, nullptr);

	videoSource = nullptr;
}

JNIEXPORT void JNICALL Java_dev_onvoid_webrtc_media_video_RandomBufferVideoDeviceSource_initialize
(JNIEnv * env, jobject caller)
{
	rtc::scoped_refptr<jni::RandomBufferVideoTrackDeviceSource> videoSource = webrtc::make_ref_counted<jni::RandomBufferVideoTrackDeviceSource>();

	SetHandle(env, caller, videoSource.release());
}
