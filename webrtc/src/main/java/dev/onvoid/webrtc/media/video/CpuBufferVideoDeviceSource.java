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

package dev.onvoid.webrtc.media.video;

public class CpuBufferVideoDeviceSource extends VideoTrackSource {
    private Thread captureThread;
    private volatile boolean running = false;
    private int width = 1280;
    private int height = 720;
    private int fps = 30;

	public CpuBufferVideoDeviceSource() {
		super();

		initialize();
	}

	public native void setVideoCaptureCapability(VideoCaptureCapability capability);

    public void start() {
        running = true;
        captureThread = new Thread(() -> {
            int ySize = width * height;
            int uSize = (width / 2) * (height / 2);
            int vSize = uSize;
            int frameSize = ySize + uSize + vSize;
            byte[] yuvBuffer = new byte[frameSize];
            java.util.Random rand = new java.util.Random();

            long frameIntervalMs = 1000 / fps;
            while (running) {
                for (int i = 0; i < frameSize; i++) {
                    yuvBuffer[i] = (byte) rand.nextInt(256);
                }
                long timestamp = System.nanoTime() / 1000;
                pushFrame(yuvBuffer, width, height, timestamp);

                try {
                    Thread.sleep(frameIntervalMs);
                } catch (InterruptedException e) {
                    break;
                }
            }
        });
        captureThread.start();
    }

    public void stop() {
        running = false;
        if (captureThread != null) {
            captureThread.interrupt();
        }
    }

	public native void dispose();
	private native void initialize();

	public native void pushFrame(byte[] yuvBuffer, int width, int height, long timestamp);
}
