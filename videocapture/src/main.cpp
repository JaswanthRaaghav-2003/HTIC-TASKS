#include <opencv2/opencv.hpp>
#include <thread>
#include <atomic>
#include <mutex>
#include <iostream>
#include <chrono>

using namespace cv;
using namespace std;

//Shared State
atomic<bool> appRunning(true);
atomic<bool> recording(false);
atomic<bool> takeSnapshot(false);

mutex frameMutex;
Mat sharedFrame;

VideoWriter videoWriter;

//Button Areas
Rect btnRecord(20, 60, 160, 50);
Rect btnSnap(200, 60, 160, 50);
Rect btnQuit(380, 60, 160, 50);

// Mouse Callback
void onMouse(int event, int x, int y, int, void*) {
    if (event != EVENT_LBUTTONDOWN) return;

    Point p(x, y);

    if (btnRecord.contains(p)) {
        recording = !recording;
        cout << (recording ? "Recording started\n" : "Recording stopped\n");
    }
    else if (btnSnap.contains(p)) {
        takeSnapshot = true;
    }
    else if (btnQuit.contains(p)) {
        appRunning = false;
    }
}

//Capture Thread
void captureFrames(VideoCapture& cap) {
    Mat frame;
    while (appRunning) {
        cap >> frame;
        if (frame.empty()) continue;

        {
            lock_guard<mutex> lock(frameMutex);
            frame.copyTo(sharedFrame);
        }
        this_thread::sleep_for(chrono::milliseconds(1));
    }
}

//Recording Thread
void recordVideo() {
    while (appRunning) {
        if (recording) {
            Mat copy;
            {
                lock_guard<mutex> lock(frameMutex);
                if (sharedFrame.empty()) continue;
                sharedFrame.copyTo(copy);
            }
            videoWriter.write(copy);
        }
        this_thread::sleep_for(chrono::milliseconds(10));
    }
}

//Buttons
void drawButton(Mat& img, Rect r, const string& text, Scalar color) {
    rectangle(img, r, color, FILLED);
    rectangle(img, r, Scalar(255,255,255), 2);

    int baseline = 0;
    Size textSize = getTextSize(text, FONT_HERSHEY_SIMPLEX, 0.7, 2, &baseline);

    Point textPos(
        r.x + (r.width - textSize.width) / 2,
        r.y + (r.height + textSize.height) / 2
    );

    putText(img, text, textPos,
        FONT_HERSHEY_SIMPLEX, 0.7, Scalar(255,255,255), 2);
}


int main() {
    VideoCapture cap;

    bool cameraFound = false;
    for (int i = 0; i < 5; i++) {
        cap.open(i);
        if (cap.isOpened()) {
            cout << "Camera opened at index " << i << endl;
            cameraFound = true;
            break;
        }
    }

    if (!cameraFound) {
        cerr << "No camera found!\n";
        system("pause");
        return -1;
    }

    int width  = (int)cap.get(CAP_PROP_FRAME_WIDTH);
    int height = (int)cap.get(CAP_PROP_FRAME_HEIGHT);
    int fps    = 30;

    videoWriter.open(
        "output.avi",
        VideoWriter::fourcc('M','J','P','G'),
        fps,
        Size(width, height)
    );

    if (!videoWriter.isOpened()) {
        cerr << "Failed to open VideoWriter\n";
        return -1;
    }

    namedWindow("Video Capture");
    setMouseCallback("Video Capture", onMouse);

    thread t1(captureFrames, ref(cap));
    thread t2(recordVideo);

    int snapshotCount = 0;

    
    while (appRunning) {
        Mat display;

        {
            lock_guard<mutex> lock(frameMutex);
            if (sharedFrame.empty()) continue;
            sharedFrame.copyTo(display);
        }

        // Draw buttons
        drawButton(display, btnRecord,
            recording ? "STOP" : "REC",
            recording ? Scalar(0,0,200) : Scalar(0,200,0));

        drawButton(display, btnSnap, "SNAP", Scalar(200,120,0));
        drawButton(display, btnQuit, "QUIT", Scalar(50,50,200));

        // Status text
        putText(display,
            recording ? "RECORDING..." : "LIVE",
            Point(20, 40),
            FONT_HERSHEY_SIMPLEX,
            1.0,
            recording ? Scalar(0,0,255) : Scalar(0,255,0),
            2);

        // Snapshot handling
        if (takeSnapshot) {
            string name = "assets/snapshot_" + to_string(snapshotCount++) + ".png";
            imwrite(name, display);
            cout << "Snapshot saved: " << name << endl;
            takeSnapshot = false;
        }

        imshow("Video Capture", display);

        if (waitKey(1) == 27)  // ESC
            appRunning = false;
    }

  
    t1.join();
    t2.join();

    cap.release();
    videoWriter.release();
    destroyAllWindows();

    return 0;
}
