package com.ysag.dirac;

import org.opencv.android.*;
import org.opencv.core.*;
import org.opencv.imgproc.*;
import org.opencv.android.CameraBridgeViewBase.CvCameraViewListener2;
import org.opencv.android.CameraBridgeViewBase.CvCameraViewFrame;

import android.content.res.*;
import android.app.Activity;
import android.os.Bundle;
import android.util.Log;
import android.widget.*;
import android.media.*;
import android.support.v4.view.*;
import android.view.*;


import java.io.*;
import java.util.*;

public class MainActivity extends Activity implements CvCameraViewListener2 {
    private static final int NUM_COMPARTMENTS = 12;
    private CameraBridgeViewBase cameraView = null;
    private BlurType blurType = BlurType.GAUSSIAN;
    private byte state = 1;
    private SoundPool soundPool = new SoundPool.Builder()
                                      .setMaxStreams(NUM_COMPARTMENTS)
                                      .build();

    private static final String[] SOUND_FILES = new String[] {
        "piano-a.wav",
        "piano-bb.wav",
        "piano-b.wav",
        "piano-c.wav",
        "piano-cs.wav",
        "piano-d.wav",
        "piano-eb.wav",
        "piano-e.wav",
        "piano-f.wav",
        "piano-fs.wav",
        "piano-g.wav",
        "piano-gs.wav",
    };

    private int[] soundIds = new int[NUM_COMPARTMENTS];

    
    // To store the current status of compartments.
    private boolean[] compartmentStatuses = new boolean[NUM_COMPARTMENTS];

    /** Called when the activity is first created. */
    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        getWindow().addFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);
        setContentView(R.layout.main);

        cameraView = (CameraBridgeViewBase) findViewById(R.id.cameraview);
        cameraView.setVisibility(SurfaceView.VISIBLE);
        cameraView.setCvCameraViewListener(this);

        for (int i = 0; i < NUM_COMPARTMENTS; ++i) {
            String soundFile = SOUND_FILES[i];
            try {
                AssetFileDescriptor afd = getAssets().openFd(soundFile);
                soundIds[i] = soundPool.load(afd, 1);
            } catch (IOException ioe) {
                Util.log("Error loading asset: " + ioe);
            }

        }
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
        soundPool.release();
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
            case R.id.main:
                state = 0;
                return true;
            case R.id.test:
                state = 1;
                return true;
            default:
                return super.onOptionsItemSelected(item);
        }
    }

    public Mat onCameraFrame(CvCameraViewFrame inputFrame) {
        Mat rgba = inputFrame.rgba();
        Util.log("Got: " + rgba.cols() + "," + rgba.rows());
        switch (state) {
            case 0:
                MatOfInt result = new MatOfInt();
                process(rgba.nativeObj, result.nativeObj, true, 0.0f, 0.0f);
                Set<Integer> indices = new HashSet(result.toList());
                Util.log("Got " + indices.size() + " indices");
                playSounds(indices);
                break;

            case 1:
                find(rgba.nativeObj);
                break;
        }
        Util.log("Going to return: " + rgba.cols() + "," + rgba.rows());
        return rgba;
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

    private void playSounds(Set<Integer> activeCompartments) {
        for (int i = 0; i < NUM_COMPARTMENTS; ++i) {
            // If the compartment was not initially active but is now active
            if (!compartmentStatuses[i] && activeCompartments.contains(i)) {
                compartmentStatuses[i] = true;
                playSound(i);
            }
            if (!activeCompartments.contains(i)) {
                compartmentStatuses[i] = false;
            }
        }
    }

    private void playSound(int index) {
        int soundId = soundIds[index];
        soundPool.play(soundId, 1.0f, 1.0f, 0, 0, 1.0f);
    }

    // Returns the index of the currently active compartment.
    private native int process(long ptrToImgMat,
                               long ptrToResultMat,
                               boolean onlyRects,
                               float minArea,
                               float maxArea);

    private native int find(long ptrToImgMat);

}
