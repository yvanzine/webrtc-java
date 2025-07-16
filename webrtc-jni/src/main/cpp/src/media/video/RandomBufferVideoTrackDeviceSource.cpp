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

#include "media/video/RandomBufferVideoTrackDeviceSource.h"
#include "Exception.h"
#include "api/video/i420_buffer.h"
#include <thread>
#include <atomic>
#include <random>
#include <chrono>
#include <iostream>

#include "api/rtp_parameters.h"

namespace jni {

    RandomBufferVideoTrackDeviceSource::RandomBufferVideoTrackDeviceSource() :
		VideoTrackSource(/*remote=*/false),
		running(false) {
		capability.width = 1280;
		capability.height = 720;
		capability.maxFPS = 30;

		// capability.width = 640;
		// capability.height = 360;
		// capability.maxFPS = 60;

		// capability.width = 320;
		// capability.height = 180;
		// capability.maxFPS = 60;
		std::cout << "RandomBufferVideoTrackDeviceSource created." << std::endl;
	}

	RandomBufferVideoTrackDeviceSource::~RandomBufferVideoTrackDeviceSource() {
		stop();
	}

	rtc::VideoSourceInterface<webrtc::VideoFrame> * RandomBufferVideoTrackDeviceSource::source()
	{
		return this;
	}

	void RandomBufferVideoTrackDeviceSource::setVideoCaptureCapability(const webrtc::VideoCaptureCapability & capability)
	{
		this->capability = capability;
	}

	void RandomBufferVideoTrackDeviceSource::AddOrUpdateSink(rtc::VideoSinkInterface<webrtc::VideoFrame> * sink, const rtc::VideoSinkWants & wants)
	{
		broadcaster.AddOrUpdateSink(sink, wants);

		updateVideoAdapter();
	}

	void RandomBufferVideoTrackDeviceSource::RemoveSink(rtc::VideoSinkInterface<webrtc::VideoFrame> * sink)
	{
		broadcaster.RemoveSink(sink);

		updateVideoAdapter();
	}

    void RandomBufferVideoTrackDeviceSource::updateVideoAdapter()
	{
		rtc::VideoSinkWants wants = broadcaster.wants();
		videoAdapter.OnOutputFormatRequest(std::make_pair(capability.width, capability.height), wants.max_pixel_count, wants.max_framerate_fps);
    }

	void RandomBufferVideoTrackDeviceSource::start() {
		if (running.load()) return;
		running.store(true);
 		captureThread = std::thread([this]() { this->captureLoop(); });
	}

	void RandomBufferVideoTrackDeviceSource::stop() {
		running.store(false);
		if (captureThread.joinable()) {
			captureThread.join();
		}
	}

	void RandomBufferVideoTrackDeviceSource::captureLoop() {
		std::mt19937 rng(std::random_device{}());
		std::uniform_int_distribution<uint8_t> dist(0, 255);
		const int width = capability.width;
		const int height = capability.height;
		const int fps = capability.maxFPS;
		const int frameIntervalMs = 1000 / (fps > 0 ? fps : 30);
        std::chrono::time_point<std::chrono::system_clock> lastTimeStamp = std::chrono::system_clock::now();

        rtc::scoped_refptr<webrtc::I420Buffer> buffer = webrtc::I420Buffer::Create(width, height);

		while (running.load()) {


			uint8_t* y = buffer->MutableDataY();
			uint8_t* u = buffer->MutableDataU();
			uint8_t* v = buffer->MutableDataV();
			int y_stride = buffer->StrideY();
			int u_stride = buffer->StrideU();
			int v_stride = buffer->StrideV();

			for (int i = 0; i < height; ++i) {
				for (int j = 0; j < width; ++j) {
					y[i * y_stride + j] = dist(rng);
				}
			}
			for (int i = 0; i < height / 2; ++i) {
				for (int j = 0; j < width / 2; ++j) {
					u[i * u_stride + j] = dist(rng);
					v[i * v_stride + j] = dist(rng);
				}
			}

			webrtc::VideoFrame frame = webrtc::VideoFrame::Builder()
				.set_video_frame_buffer(buffer)
				.set_rotation(webrtc::kVideoRotation_0)
				.set_timestamp_us(rtc::TimeMicros())
				.build();

            OnFrame(frame);

            auto now = std::chrono::system_clock::now();
            auto diff = duration_cast<std::chrono::milliseconds>(now - lastTimeStamp).count(); 
            auto remainingIntervalMs = std::chrono::milliseconds( diff > frameIntervalMs ? 0 : frameIntervalMs - diff );
            // Sleep for the frame interval to simulate a real video capture device.
            //std::cout << "Sleeping for " << remainingIntervalMs << "milliseconds" << std::endl;
			std::this_thread::sleep_for(remainingIntervalMs);
            //std::cout << "Difference between pushed frames " << duration_cast<std::chrono::milliseconds>((now + remainingIntervalMs) - lastTimeStamp) << "milliseconds" << std::endl;
            lastTimeStamp = now + remainingIntervalMs;

		}
	}


	void RandomBufferVideoTrackDeviceSource::OnFrame(const webrtc::VideoFrame & frame)
	{
        //std::cout << "RandomBufferVideoTrackDeviceSource::OnFrame" << std::endl; 

		int croppedWidth = 0;
		int croppedHeight = 0;
		int outWidth = 0;
		int outHeight = 0;

		if (!videoAdapter.AdaptFrameResolution(frame.width(), frame.height(), frame.timestamp_us() * 1000,
			&croppedWidth, &croppedHeight, &outWidth, &outHeight)) {
			// Drop frame in order to respect frame rate constraint.

            std::cout << "Dropping frames" << std::endl;

			return;
		}

		if (true /* outHeight != frame.height() || outWidth != frame.width()*/) {

            //std::cout << "resizing frames" << outWidth << "," << outHeight << std::endl;

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
