package com.dongnao.facedetection;

import android.app.Activity;
import android.app.ProgressDialog;
import android.content.Intent;
import android.database.Cursor;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.net.Uri;
import android.os.AsyncTask;
import android.os.Build;
import android.os.Bundle;
import android.os.Environment;
import android.provider.MediaStore;
import android.text.TextUtils;
import android.util.Log;
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

    private Bitmap bm;
    private ProgressDialog pd;
    private final static String TAG = "MainActivity";

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
                    File file1 = new File(dir, "haarcascade_frontalface_alt.xml");
                    loadModel(file1.getAbsolutePath());
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
        Intent intent;
        if (Build.VERSION.SDK_INT < Build.VERSION_CODES.KITKAT) {
            intent = new Intent();
            intent.setAction(Intent.ACTION_GET_CONTENT);
        } else {
            intent = new Intent(Intent.ACTION_PICK,
                    MediaStore.Images.Media.EXTERNAL_CONTENT_URI);
        }
        intent.setType("image/*");
        //使用选取器并自定义标题
        startActivityForResult(Intent.createChooser(intent, "选择待识别图片"), 100);
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();
        destory();
        safeRecycled();
    }

    public static Bitmap toBitmap(String pathName) {
        BitmapFactory.Options o = new BitmapFactory.Options();
        o.inJustDecodeBounds = true;
        BitmapFactory.decodeFile(pathName, o);
        int width_tmp = o.outWidth, height_tmp = o.outHeight;
        int scale = 1;
        while (true) {
            if (width_tmp <= 640 && height_tmp <= 480) {
                break;
            }
            width_tmp /= 2;
            height_tmp /= 2;
            scale *= 2;
        }
        BitmapFactory.Options opts = new BitmapFactory.Options();
        opts.inSampleSize = scale;
        opts.outHeight = height_tmp;
        opts.outWidth = width_tmp;
        return BitmapFactory.decodeFile(pathName, opts);
    }

    private void getResult(Uri uri) {
        safeRecycled();
        String imagePath = null;
        if (null != uri) {
            //在我们的魅族测试手机上发现有一个相册管家 从这里选取图片会得到类似
            //file:///storage/emulated/0/tencent/MicroMsg/WeiXin/mmexport1474966179606.jpg的uri
            if ("file".equals(uri.getScheme())) {
                Log.i(TAG, "path uri 获得图片");
                imagePath = uri.getPath();
            } else if ("content".equals(uri.getScheme())) {
                Log.i(TAG, "content uri 获得图片");
                String[] filePathColumns = {MediaStore.Images.Media.DATA};
                Cursor c = getContentResolver().query(uri, filePathColumns, null, null, null);
                if (null != c) {
                    if (c.moveToFirst()) {
                        int columnIndex = c.getColumnIndex(filePathColumns[0]);
                        imagePath = c.getString(columnIndex);
                    }
                    c.close();
                }
            }
        }
        if (!TextUtils.isEmpty(imagePath)) {
            bm = toBitmap(imagePath);
            safeProcess();
        }
    }

    @Override
    protected void onActivityResult(int requestCode, int resultCode, Intent data) {
        super.onActivityResult(requestCode, resultCode, data);
        if (requestCode == 100 && null != data) {
            getResult(data.getData());
        }
    }

    public native void loadModel(String detectModel);

    private native void setSurface(Surface surface, int w, int h);

    public native boolean process(Bitmap bitmap);

    public native void destory();

    public void safeProcess() {
        if (null != bm && !bm.isRecycled()) {
            process(bm);
        }
    }

    public void safeRecycled() {
        if (null != bm && !bm.isRecycled()) {
            bm.recycle();
        }
        bm = null;
    }

    @Override
    public void surfaceCreated(SurfaceHolder surfaceHolder) {

    }

    @Override
    public void surfaceChanged(SurfaceHolder surfaceHolder, int i, int i1, int i2) {
        setSurface(surfaceHolder.getSurface(), 640, 480);
        safeProcess();
    }

    @Override
    public void surfaceDestroyed(SurfaceHolder surfaceHolder) {

    }

}
