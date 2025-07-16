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

#include "media/video/CpuBufferVideoTrackDeviceSource.h"
#include "Exception.h"
#include "api/video/i420_buffer.h"
#include <thread>
#include <atomic>
#include <random>
#include <chrono>
#include <iostream>

#include "api/rtp_parameters.h"

namespace jni {

    CpuBufferVideoTrackDeviceSource::CpuBufferVideoTrackDeviceSource() :
		VideoTrackSource(/*remote=*/false) {
		std::cout << "CpuBufferVideoTrackDeviceSource created." << std::endl;
	}

	CpuBufferVideoTrackDeviceSource::~CpuBufferVideoTrackDeviceSource() {
		std::cout << "CpuBufferVideoTrackDeviceSource destructed." << std::endl;
	}

	rtc::VideoSourceInterface<webrtc::VideoFrame> * CpuBufferVideoTrackDeviceSource::source()
	{
		return this;
	}

	void CpuBufferVideoTrackDeviceSource::setVideoCaptureCapability(const webrtc::VideoCaptureCapability & capability)
	{
		this->capability = capability;
	}

	void CpuBufferVideoTrackDeviceSource::AddOrUpdateSink(rtc::VideoSinkInterface<webrtc::VideoFrame> * sink, const rtc::VideoSinkWants & wants)
	{
		broadcaster.AddOrUpdateSink(sink, wants);

		updateVideoAdapter();
	}

	void CpuBufferVideoTrackDeviceSource::RemoveSink(rtc::VideoSinkInterface<webrtc::VideoFrame> * sink)
	{
		broadcaster.RemoveSink(sink);

		updateVideoAdapter();
	}

    void CpuBufferVideoTrackDeviceSource::updateVideoAdapter()
	{
		rtc::VideoSinkWants wants = broadcaster.wants();
		videoAdapter.OnOutputFormatRequest(std::make_pair(capability.width, capability.height), wants.max_pixel_count, wants.max_framerate_fps);
    }

	void CpuBufferVideoTrackDeviceSource::OnFrame(const webrtc::VideoFrame & frame)
	{
		int croppedWidth = 0;
		int croppedHeight = 0;
		int outWidth = 0;
		int outHeight = 0;

		if (!videoAdapter.AdaptFrameResolution(frame.width(), frame.height(), frame.timestamp_us() * 1000,
			&croppedWidth, &croppedHeight, &outWidth, &outHeight)) {
			// Drop frame in order to respect frame rate constraint.
            // std::cout << "Dropping frames" << std::endl;

			return;
		}

		if (outHeight != frame.height() || outWidth != frame.width()) {
            // std::cout << "resizing frames" << outWidth << "," << outHeight << std::endl;
			// Video adapter has requested a down-scale. Allocate a new buffer and return scaled version.
			rtc::scoped_refptr<webrtc::I420Buffer> scaled_buffer = webrtc::I420Buffer::Create(outWidth, outHeight);

			scaled_buffer->ScaleFrom(*frame.video_frame_buffer()->ToI420());
			
			broadcaster.OnFrame(webrtc::VideoFrame::Builder()
				.set_video_frame_buffer(scaled_buffer)
				.set_rotation(webrtc::kVideoRotation_0)
				.set_timestamp_us(frame.timestamp_us())
				.set_id(frame.id())
				.build());
		}
		else {
			// No adaptations needed, just return the frame as is.
			broadcaster.OnFrame(frame);
		}
	}
} // namespace jni
