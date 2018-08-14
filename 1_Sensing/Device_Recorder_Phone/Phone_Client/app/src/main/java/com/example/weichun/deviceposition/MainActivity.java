package com.example.weichun.deviceposition;

import android.app.AlertDialog;
import android.content.Context;
import android.content.DialogInterface;
import android.hardware.Sensor;
import android.hardware.SensorEvent;
import android.hardware.SensorEventListener;
import android.hardware.SensorManager;
import android.os.Bundle;
import android.os.Environment;
import android.provider.MediaStore;
import android.support.v7.app.AppCompatActivity;
import android.text.TextUtils;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.View;
import android.widget.Button;
import android.widget.CompoundButton;
import android.widget.EditText;
import android.widget.TextView;
import android.widget.Toast;
import android.widget.ToggleButton;

import java.io.BufferedInputStream;
import java.io.BufferedOutputStream;
import java.io.BufferedReader;
import java.io.DataOutputStream;
import java.io.File;
import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.io.OutputStream;
import java.net.HttpURLConnection;
import java.net.URL;
import java.text.SimpleDateFormat;
import java.util.Calendar;
import java.util.Date;
import java.util.GregorianCalendar;
import java.util.List;
import java.util.SimpleTimeZone;


public class MainActivity extends AppCompatActivity {

    private SensorManager mSensorManager;
    private Sensor mSensor;
    private Context mContext;

    private DataOutputStream out_acc;
    private TextView text;
    private static ToggleButton tbtn;
    private TextView message;
    private Button sbtn;

    private boolean startRec = false;

    private Calendar calendar;

    private File file_acc;
    private String name = "";
    private String ip = "";
    private String port = "8889";
    private float[] gravityValues = null;
    private float[] magneticValues = null;
    private float[] gyroscopeValues = null;
    private long diffmills = 0;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        inputIPName();

        // sync Time from server
        new Thread(){
            public void run(){

                do {
                    Date serverDate = syncTimefromServer();
                    if (serverDate != null) {
                        long curTime = System.currentTimeMillis(); // 獲取當前時間
                        diffmills = serverDate.getTime() - curTime;
                    }
                    try {
                        Thread.sleep(1000);
                    } catch (InterruptedException e) {
                        e.printStackTrace();
                    }
                }while (true);
            }
        }.start();

        mContext = this.getApplicationContext();
        text = (TextView)findViewById(R.id.log);
        tbtn = (ToggleButton)findViewById(R.id.tbtn);
        message = (TextView)findViewById(R.id.message);
        sbtn = (Button)findViewById(R.id.send);

        mSensorManager = (SensorManager) getSystemService(Context.SENSOR_SERVICE);
        List<Sensor> deviceSensors = mSensorManager.getSensorList(Sensor.TYPE_ALL);

        // Show all supported sensor
        for(int i = 0; i < deviceSensors.size(); i++) {
            Log.d("[SO]", deviceSensors.get(i).getName());
        }

        if ((mSensor = mSensorManager.getDefaultSensor(Sensor.TYPE_LINEAR_ACCELERATION)) != null){
            mSensorManager.registerListener(mSensorListener, mSensor, 10000);
        } else {
            Toast.makeText(mContext, "ACCELEROMETER is not supported!", Toast.LENGTH_SHORT).show();
        }

        if ((mSensor = mSensorManager.getDefaultSensor(Sensor.TYPE_GRAVITY)) != null){
            mSensorManager.registerListener(mSensorListener, mSensor, 10000);
        } else {
            Toast.makeText(mContext, "GYROSCOPE is not supported!", Toast.LENGTH_SHORT).show();
        }

        if ((mSensor = mSensorManager.getDefaultSensor(Sensor.TYPE_MAGNETIC_FIELD)) != null){
            mSensorManager.registerListener(mSensorListener, mSensor, 10000);
        } else {
            Toast.makeText(mContext, "MAGNETOMETER is not supported!", Toast.LENGTH_SHORT).show();
        }

        if ((mSensor = mSensorManager.getDefaultSensor(Sensor.TYPE_GYROSCOPE)) != null){
            mSensorManager.registerListener(mSensorListener, mSensor, 10000);
        } else {
            Toast.makeText(mContext, "GYROSCOPE is not supported!", Toast.LENGTH_SHORT).show();
        }


        try {
            String dir_path = Environment.getExternalStorageDirectory().getAbsolutePath() + "/AbsAccCollection";
            File dir = new File(dir_path);
            if(!dir.exists()) {
                dir.mkdir();
            }
            file_acc = new File(dir, "raw_acc.txt");

            if (file_acc.exists()) {
                file_acc.delete();
            }

            out_acc = new DataOutputStream(new FileOutputStream(file_acc, true));

        } catch (Exception e) {
            e.printStackTrace();
        }

        /* get time */
        SimpleTimeZone pdt = new SimpleTimeZone(8 * 60 * 60 * 1000, "Asia/Taipei");
        calendar = new GregorianCalendar(pdt);


        tbtn.setOnCheckedChangeListener(new CompoundButton.OnCheckedChangeListener() {
            public void onCheckedChanged(CompoundButton buttonView, boolean isChecked) {
                if (isChecked) {
                    startRec = true;

                    SimpleDateFormat formatter = new SimpleDateFormat("HH:mm:ss:SSS");
                    long curTime = System.currentTimeMillis(); // 獲取當前時間
                    String time = formatter.format(new Date(curTime));
                    double diffsec = diffmills / 1000;
                    text.append(" Start time: " + time + "+offset(sec):" + diffsec);
//                    Date trialTime = new Date();
//                    calendar.setTime(trialTime);
//                    text.append(" Start time: " + calendar.get(Calendar.HOUR_OF_DAY) + ":" +
//                            calendar.get(Calendar.MINUTE) + ":" +
//                            calendar.get(Calendar.SECOND));
                } else {
                    startRec = false;

                    SimpleDateFormat formatter = new SimpleDateFormat("HH:mm:ss:SSS");
                    long curTime = System.currentTimeMillis(); // 獲取當前時間
                    String time = formatter.format(new Date(curTime));
                    double diffsec = diffmills / 1000;
                    text.append(" End time: " + time + "+offset(sec):" + diffsec);

//                    Date trialTime = new Date();
//                    calendar.setTime(trialTime);
//                    text.append(" End time: " + calendar.get(Calendar.HOUR_OF_DAY) + ":" +
//                            calendar.get(Calendar.MINUTE) + ":" +
//                            calendar.get(Calendar.SECOND));
//                    try {
//                        out_acc.close();
//                    } catch (IOException e) {
//                        e.printStackTrace();
//                    }
                }
            }
        });

        sbtn.setOnClickListener(new Button.OnClickListener(){
            @Override
            public void onClick(View v){
                if(startRec) {
                    Toast.makeText(mContext, "Still recording", Toast.LENGTH_SHORT).show();
                } else {
                    new Thread(){
                        public void run(){
                            if (sendHttpRequest().equals("success")) {
//                                Toast.makeText(mContext, "Send Success", Toast.LENGTH_SHORT).show();
//                                sbtn.setVisibility(View.GONE);
                            }
                        }
                    }.start();

                }
            }
        });
    }

    private void inputIPName(){
        final View item = LayoutInflater.from(MainActivity.this).inflate(R.layout.foralertdialog, null);
        new AlertDialog.Builder(MainActivity.this)
                .setTitle("IP address / Name")
                .setView(item)
                .setPositiveButton("確定", new DialogInterface.OnClickListener() {
                    @Override
                    public void onClick(DialogInterface dialog, int which) {
                        EditText editText_ip = (EditText) item.findViewById(R.id.edit_text_ip);
                        EditText editText_name = (EditText) item.findViewById(R.id.edit_text_name);
                        ip = editText_ip.getText().toString();
                        name = editText_name.getText().toString();
                    }
                })
                .show();
    }

//    private void inputName(){
//        final View item = LayoutInflater.from(MainActivity.this).inflate(R.layout.foralertdialog, null);
//        new AlertDialog.Builder(MainActivity.this)
//                .setTitle("Name")
//                .setView(item)
//                .setPositiveButton("確定", new DialogInterface.OnClickListener() {
//                    @Override
//                    public void onClick(DialogInterface dialog, int which) {
//                        EditText editText = (EditText) item.findViewById(R.id.edit_text_name);
//                        name = editText.getText().toString();
//                    }
//                })
//                .show();
//    }

    private SensorEventListener mSensorListener = new SensorEventListener(){

        public final void onSensorChanged(SensorEvent event) {
            String time;
            String raw;
            if(startRec) {
                if ((gravityValues != null) && (magneticValues != null) && (gyroscopeValues != null)
                        && (event.sensor.getType() == Sensor.TYPE_LINEAR_ACCELERATION)) {
                    try {

                        float[] deviceRelativeAcceleration = new float[4];
                        deviceRelativeAcceleration[0] = event.values[0];
                        deviceRelativeAcceleration[1] = event.values[1];
                        deviceRelativeAcceleration[2] = event.values[2];
                        deviceRelativeAcceleration[3] = 0;

                        // Change the device relative acceleration values to earth relative values
                        // X axis -> East
                        // Y axis -> North Pole
                        // Z axis -> Sky

                        float[] R = new float[16], I = new float[16], earthAcc = new float[16];

                        SensorManager.getRotationMatrix(R, I, gravityValues, magneticValues);

                        float[] inv = new float[16];

                        android.opengl.Matrix.invertM(inv, 0, R, 0);
                        android.opengl.Matrix.multiplyMV(earthAcc, 0, inv, 0, deviceRelativeAcceleration, 0);
                        //Log.d("EAcc", "Values: (" + earthAcc[0] + ", " + earthAcc[1] + ", " + earthAcc[2] + ")");
                        //Log.d("Acc", "Values: (" + deviceRelativeAcceleration[0] + ", " + deviceRelativeAcceleration[1] + ", " + deviceRelativeAcceleration[2] + ")");
                        //text.append("Values: (" + earthAcc[0] + ", " + earthAcc[1] + ", " + earthAcc[2] + ")\n");

                        SimpleDateFormat formatter = new SimpleDateFormat("HH:mm:ss:SSS");
                        long curTime = System.currentTimeMillis(); // 獲取當前時間
                        long serverTime = curTime + diffmills;
                        time = formatter.format(new Date(serverTime));

                        raw = String.valueOf(earthAcc[0]) + "," + String.valueOf(earthAcc[1]) + "," + String.valueOf(earthAcc[2]) + "," +
                                String.valueOf(deviceRelativeAcceleration[0]) + "," + String.valueOf(deviceRelativeAcceleration[1]) + "," + String.valueOf(deviceRelativeAcceleration[2]) + "," +
                                String.valueOf(gyroscopeValues[0]) + "," + String.valueOf(gyroscopeValues[1]) + "," + String.valueOf(gyroscopeValues[2]) + "," +
                                time + "\n";
//                        raw_datas += String.valueOf(earthAcc[0]) + "," + String.valueOf(earthAcc[1]) + "," + String.valueOf(earthAcc[2]) + "," +
//                                String.valueOf(deviceRelativeAcceleration[0]) + "," + String.valueOf(deviceRelativeAcceleration[1]) + "," + String.valueOf(deviceRelativeAcceleration[2]) + "," +
//                                String.valueOf(gyroscopeValues[0]) + "," + String.valueOf(gyroscopeValues[1]) + "," + String.valueOf(gyroscopeValues[2]) + "," +
//                                time + "\n";
//                        message.setText("earthX:" + String.valueOf(earthAcc[0]) + "\n" +
////                                "earthY:" + String.valueOf(earthAcc[1]) + "\n" +
////                                "earthZ:" + String.valueOf(earthAcc[2]) + "\n" +
////                                "accX:" + String.valueOf(deviceRelativeAcceleration[0]) + "\n" +
////                                "accY:" + String.valueOf(deviceRelativeAcceleration[1]) + "\n" +
////                                "accZ:" + String.valueOf(deviceRelativeAcceleration[2]) + "\n" +
////                                "gyroX:" + String.valueOf(gyroscopeValues[0]) + "\n" +
////                                "gyroY:" + String.valueOf(gyroscopeValues[1]) + "\n" +
////                                "gyroZ:" + String.valueOf(gyroscopeValues[2]) + "\n" +
////                                time + "\n");
                        out_acc.write(raw.getBytes());
                    } catch (Exception e) {
                        e.printStackTrace();
                    }
                } else if (event.sensor.getType() == Sensor.TYPE_GRAVITY) {
                    gravityValues = event.values;
                } else if (event.sensor.getType() == Sensor.TYPE_MAGNETIC_FIELD) {
                    magneticValues = event.values;
                } else if (event.sensor.getType() == Sensor.TYPE_GYROSCOPE) {
                    gyroscopeValues = event.values;
                }
            }
        }

        @Override
        public final void onAccuracyChanged(Sensor sensor, int accuracy) {
            // Do something here if sensor accuracy changes.
        }
    };

    private String sendHttpRequest(){
        HttpURLConnection urlConnection = null;
        String ret = "";
        try {
            URL url = new URL( "http://" + this.ip + ":" + this.port + "/phone/" + this.name );
            urlConnection = (HttpURLConnection) url.openConnection();
            urlConnection.setRequestMethod("POST");
            urlConnection.setDoOutput(true);
            urlConnection.setDoInput(true);
            urlConnection.setUseCaches(false);
            OutputStream out = urlConnection.getOutputStream();

            int size = (int) file_acc.length();
            byte[] bytes = new byte[size];
            try {
                BufferedInputStream buf = new BufferedInputStream(new FileInputStream(file_acc));
                buf.read(bytes, 0, bytes.length);
                buf.close();
            } catch (FileNotFoundException e) {
                // TODO Auto-generated catch block
                e.printStackTrace();
            } catch (IOException e) {
                // TODO Auto-generated catch block
                e.printStackTrace();
            }

            out.write(bytes);
            out.flush();

            if (urlConnection.getResponseCode() == 200) {
                ret = "success";
            }
        } catch (Exception e){
            message.setText(e.toString());
            ret = "error";
        }
        //urlConnection.disconnect();
        return ret;
    }

    private Date syncTimefromServer(){
        HttpURLConnection urlConnection = null;
        Date ret = null;
        try {
            URL url = new URL( "http://" + this.ip + ":" + this.port + "/time");
            urlConnection = (HttpURLConnection) url.openConnection();
            urlConnection.setRequestMethod("GET");

            if (urlConnection.getResponseCode() == 200) {
                InputStream inputStream = urlConnection.getInputStream();
                BufferedReader  bufferedReader  = new BufferedReader(new InputStreamReader(inputStream));

                String tempStrTime;
                StringBuffer stringBuffer = new StringBuffer();
                while( ( tempStrTime = bufferedReader.readLine() ) != null ) {
                    stringBuffer.append( tempStrTime );
                }
                String serverDate = stringBuffer.toString();

                Log.d("[Server Time]", serverDate);
                SimpleDateFormat format = new SimpleDateFormat("HH:mm:ss:SSS");
                Date serverTime = format.parse(serverDate);
                ret = serverTime;
            }
        } catch (Exception e){
            ret = null;
        }
        //urlConnection.disconnect();
        return ret;
    }

    @Override
    protected void onResume() {
        super.onResume();
        mSensorManager.registerListener(mSensorListener, mSensor, SensorManager.SENSOR_DELAY_NORMAL);
    }

    @Override
    protected void onPause() {
        super.onPause();
        mSensorManager.unregisterListener(mSensorListener);
    }
}
