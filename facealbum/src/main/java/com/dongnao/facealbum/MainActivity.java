package com.dongnao.facealbum;

import android.app.Activity;
import android.app.ProgressDialog;
import android.content.Intent;
import android.database.Cursor;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.net.Uri;
import android.os.AsyncTask;
import android.os.Bundle;
import android.os.Environment;
import android.provider.MediaStore;
import android.view.Surface;
import android.view.SurfaceHolder;
import android.view.SurfaceView;
import android.view.View;

import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;

public class MainActivity extends Activity implements SurfaceHolder.Callback {


    static {
        System.loadLibrary("opencv_java3");
        System.loadLibrary("native-lib");
    }


    private ProgressDialog pd;

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
        SurfaceView surfaceView = (SurfaceView) findViewById(R.id.surfaceView);
        surfaceView.getHolder().addCallback(this);

        new AsyncTask<Void, Void, Void>() {

            @Override
            protected Void doInBackground(Void... params) {
                try {
                    File dir = new File(Environment.getExternalStorageDirectory(), "face");
                    copyAssetsFile("haarcascade_frontalface_alt.xml", dir);
                    copyAssetsFile("shape_predictor_68_face_landmarks.dat", dir);
                    File file1 = new File(dir, "haarcascade_frontalface_alt.xml");
                    File file2 = new File(dir, "shape_predictor_68_face_landmarks.dat");
                    loadModel(file1.getAbsolutePath(), file2.getAbsolutePath());
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

    private void copyAssetsFile(String name, File dir) throws IOException {
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
    }

    public void from_album(View view) {
        Intent intent = new Intent(Intent.ACTION_PICK, null);
        intent.setDataAndType(
                MediaStore.Images.Media.EXTERNAL_CONTENT_URI,
                "image/*");
        // 调用剪切功能

        startActivityForResult(intent, 100);
    }

    @Override
    protected void onActivityResult(int requestCode, int resultCode, Intent data) {
        super.onActivityResult(requestCode, resultCode, data);
        if (requestCode == 100) {
            Uri selectedImage = data.getData();
            if (null != selectedImage) {
                String[] filePathColumns = {MediaStore.Images.Media.DATA};
                Cursor c = getContentResolver().query(selectedImage, filePathColumns, null, null,
                        null);
                c.moveToFirst();
                int columnIndex = c.getColumnIndex(filePathColumns[0]);
                String imagePath = c.getString(columnIndex);
                Bitmap bm = BitmapFactory.decodeFile(imagePath);
                process(bm);
                c.close();
            }
        }
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();
        destory();
    }

    @Override
    public void surfaceCreated(SurfaceHolder surfaceHolder) {

    }

    @Override
    public void surfaceChanged(SurfaceHolder surfaceHolder, int format, int width, int height) {
        setSurface(surfaceHolder.getSurface(), width, height);
    }

    @Override
    public void surfaceDestroyed(SurfaceHolder surfaceHolder) {

    }

    public native void loadModel(String detectModel, String landmarkModel);

    private native void setSurface(Surface surface, int w, int h);

    public native void process(Bitmap bitmap);

    public native void destory();

}
