package com.dongnao.facetrack;

import android.app.Activity;
import android.app.ProgressDialog;
import android.os.AsyncTask;
import android.os.Bundle;
import android.os.Environment;
import android.view.View;

import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;

public class MainActivity extends Activity {


    private ProgressDialog pd;
    private DisplaySurfaceView surfaceView;

    private void showLoading() {
        if (null == pd) {
            pd = new ProgressDialog(this);
            pd.setIndeterminate(true);
        }
        pd.show();
    }

    private void dismissLoading() {
        if (null != pd) {
            pd.dismiss();
        }
    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        surfaceView = (DisplaySurfaceView) findViewById(R.id.surfaceView);
        new AsyncTask<Void, Void, Void>() {

            @Override
            protected Void doInBackground(Void... params) {
                try {
                    File dir = new File(Environment.getExternalStorageDirectory(), "face");
                    File haar = copyAssetsFile("haarcascade_frontalface_alt.xml", dir);
                    FaceHelper.loadModel(haar.getAbsolutePath());
                    FaceHelper.startTracking();
                } catch (IOException e) {
                    e.printStackTrace();
                }
                return null;
            }

            @Override
            protected void onPreExecute() {
                showLoading();
            }

            @Override
            protected void onPostExecute(Void aVoid) {
                dismissLoading();
            }
        }.execute();
    }


    private File copyAssetsFile(String name, File dir) throws IOException {
        if (!dir.exists()) {
            dir.mkdirs();
        }
        File file = new File(dir, name);
        if (!file.exists()) {
            InputStream is = getAssets().open(name);
            FileOutputStream fos = new FileOutputStream(file);
            int len;
            byte[] buffer = new byte[2048];
            while ((len = is.read(buffer)) != -1)
                fos.write(buffer, 0, len);
            fos.close();
            is.close();
        }
        return file;
    }


    @Override
    protected void onDestroy() {
        super.onDestroy();
        FaceHelper.destory();
    }

    public void switchCamera(View view) {
        surfaceView.switchCamera();
    }

}
