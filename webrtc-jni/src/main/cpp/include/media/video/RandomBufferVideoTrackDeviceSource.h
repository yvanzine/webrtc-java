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

#ifndef JNI_WEBRTC_MEDIA_RANDOM_BUFFER_VIDEO_TRACK_DEVICE_SOURCE_H_
#define JNI_WEBRTC_MEDIA_RANDOM_BUFFER_VIDEO_TRACK_DEVICE_SOURCE_H_

#include "api/create_peerconnection_factory.h"
#include "api/video/video_frame.h"
#include "api/video/video_sink_interface.h"
#include "pc/video_track_source.h"
#include "media/base/video_adapter.h"
#include "media/base/video_broadcaster.h"
#include "modules/video_capture/video_capture.h"
#include "modules/video_capture/video_capture_defines.h"

#include "media/video/VideoDevice.h"

#include <thread>
#include <atomic>

namespace jni {

class RandomBufferVideoTrackDeviceSource : public webrtc::VideoTrackSource, public rtc::VideoSinkInterface<webrtc::VideoFrame> {
public:
	RandomBufferVideoTrackDeviceSource();
	~RandomBufferVideoTrackDeviceSource();

    void setVideoCaptureCapability(const webrtc::VideoCaptureCapability & capability);
    // VideoSourceInterface implementation.
    void AddOrUpdateSink(rtc::VideoSinkInterface<webrtc::VideoFrame> * sink, const rtc::VideoSinkWants & wants) override;
    void RemoveSink(rtc::VideoSinkInterface<webrtc::VideoFrame> * sink) override;

    // VideoTrackSource implementation.
    rtc::VideoSourceInterface<webrtc::VideoFrame> * source() override;

    // VideoSinkInterface implementation.
    void OnFrame(const webrtc::VideoFrame & frame) override;

    void start();
	void stop();

private:
	void captureLoop();
	std::thread captureThread;
	std::atomic<bool> running;

    void updateVideoAdapter();

private:
    webrtc::VideoCaptureCapability capability;
    rtc::VideoBroadcaster broadcaster;
    cricket::VideoAdapter videoAdapter;
};

} // namespace jni

#endif // JNI_WEBRTC_MEDIA_RANDOM_BUFFER_VIDEO_TRACK_DEVICE_SOURCE_H_
