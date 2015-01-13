package com.ysag.dirac;

import org.opencv.android.*;
import org.opencv.core.*;
import org.opencv.imgproc.*;
import org.opencv.android.CameraBridgeViewBase.CvCameraViewListener2;
import org.opencv.android.CameraBridgeViewBase.CvCameraViewFrame;

import android.app.Activity;
import android.os.Bundle;
import android.util.Log;
import android.widget.*;
import android.media.*;
import android.support.v4.view.*;
import android.view.*;


import java.util.*;

public class MainActivity extends Activity implements CvCameraViewListener2 {

    private CameraBridgeViewBase cameraView = null;
    private BlurType blurType = BlurType.GAUSSIAN;

    /** Called when the activity is first created. */
    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        getWindow().addFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);
        setContentView(R.layout.main);

        cameraView = (CameraBridgeViewBase) findViewById(R.id.cameraview);
        cameraView.setVisibility(SurfaceView.VISIBLE);
        cameraView.setCvCameraViewListener(this);
    }

    @Override
    public void onResume() {
        super.onResume();
        OpenCVLoader.initAsync(OpenCVLoader.OPENCV_VERSION_2_4_3, this, opencvLoderCallback);
    }

    @Override
    public void onPause() {
        super.onPause();
        if (cameraView != null) {
            cameraView.disableView();
        }
    }

    @Override
    public void onDestroy() {
        if (cameraView != null) {
            cameraView.disableView();
        }
        super.onDestroy();
    }

    @Override
    public boolean onCreateOptionsMenu(Menu menu) {
        MenuInflater menuInflater = getMenuInflater();
        menuInflater.inflate(R.menu.menu, menu);
        return true;
    }

    @Override
    public boolean onOptionsItemSelected(MenuItem item) {
        switch (item.getItemId()) {
            case R.id.gaussian:
                blurType = BlurType.GAUSSIAN;
                return true;
            case R.id.median:
                blurType = BlurType.MEDIAN;
                return true;
            default:
                return super.onOptionsItemSelected(item);
        }
    }

    public Mat onCameraFrame(CvCameraViewFrame inputFrame) {
        if (true) {
            Mat rgba = inputFrame.rgba();
            process(rgba.nativeObj);
            return rgba;
        }
        Mat gray = inputFrame.gray();
        switch (blurType) {
            case GAUSSIAN:
                Imgproc.GaussianBlur(gray, gray, new Size(15, 15), 0.0, 0.0);
                break;
            case MEDIAN:
                Imgproc.medianBlur(gray, gray, 15);
                break;
        }
        return gray;
    }

    public void onCameraViewStarted(int width, int height) {

    }

    public void onCameraViewStopped() {

    }

    private BaseLoaderCallback opencvLoderCallback = new BaseLoaderCallback(this) {
        @Override
        public void onManagerConnected(int status) {
            switch (status) {
                case LoaderCallbackInterface.SUCCESS:
                    System.loadLibrary("dirac");
                    cameraView.enableView();
                    break;

            default:
                super.onManagerConnected(status);
                break;
            }
        }
    };

    private static enum BlurType {
        GAUSSIAN,
        MEDIAN
    }

    private native void process(long ptrToMat);

}
