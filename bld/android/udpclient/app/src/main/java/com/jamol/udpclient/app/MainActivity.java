package com.jamol.udpclient.app;

import android.support.v7.app.ActionBarActivity;
import android.os.Bundle;
import android.util.Log;
import android.view.Menu;
import android.view.MenuItem;
import android.view.View;
import android.widget.Button;
import android.widget.CompoundButton;
import android.widget.EditText;
import android.widget.Switch;

import java.net.Inet4Address;
import java.net.InetAddress;
import java.net.NetworkInterface;
import java.net.SocketException;
import java.util.Enumeration;


public class MainActivity extends ActionBarActivity {

    public native long getTickCount1();
    public native long getTickCount2();
    public native void startTest(String myIp, int myPort, String peerIp, int peerPort,
                                 int bw, int timeSlice, int burstBytes);
    public native void stopTest();

    private boolean mStarted = false;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        //long tick = getTickCount1();
        //tick = getTickCount2();
        EditText editMyIp = (EditText)findViewById(R.id.editTextMyIp);
        editMyIp.setText(getLocalIpAddress());

        Switch switchBurst = (Switch)findViewById(R.id.switchBurst);
        switchBurst.setChecked(false);
        switchBurst.setOnCheckedChangeListener(new CompoundButton.OnCheckedChangeListener() {
            @Override
            public void onCheckedChanged(CompoundButton buttonView,
                                         boolean isChecked) {
                EditText editBurstBytes = (EditText)findViewById(R.id.editTextBurstBytes);
                editBurstBytes.setEnabled(isChecked);
            }
        });

        Button btnStart = (Button)findViewById(R.id.buttonStart);
        btnStart.setOnClickListener(new View.OnClickListener(){
            @Override
            public void onClick(View v) {
                switch(v.getId())
                {
                    case R.id.buttonStart:
                        Button btn = (Button)v;
                        if(!mStarted){
                            mStarted = true;
                            onStartTest();
                            btn.setText("Stop");
                        } else {
                            mStarted = false;
                            onStopTest();
                            btn.setText("Start");
                        }
                        break;
                    default:
                        break;
                }
            }
        });
    }

    public void onStartTest(){
        EditText editMyIp = (EditText)findViewById(R.id.editTextMyIp);
        EditText editMyPort = (EditText)findViewById(R.id.editTextMyPort);
        EditText editPeerIp = (EditText)findViewById(R.id.editTextPeerIp);
        EditText editPeerPort = (EditText)findViewById(R.id.editTextPeerPort);
        EditText editBandwidth = (EditText)findViewById(R.id.editTextBandwidth);
        EditText editTimeSlice = (EditText)findViewById(R.id.editTextTimeSlice);
        EditText editBurstBytes = (EditText)findViewById(R.id.editTextBurstBytes);
        int burstBytes = Integer.parseInt(editBurstBytes.getText().toString());
        Switch switchBurst = (Switch)findViewById(R.id.switchBurst);
        if(!switchBurst.isChecked()){
            burstBytes = 0;
        }
        startTest(editMyIp.getText().toString(),
                Integer.parseInt(editMyPort.getText().toString()),
                editPeerIp.getText().toString(),
                Integer.parseInt(editPeerPort.getText().toString()),
                Integer.parseInt(editBandwidth.getText().toString()),
                Integer.parseInt(editTimeSlice.getText().toString()),
                burstBytes
        );
    }

    public void onStopTest() {
        stopTest();
    }

    @Override
    public boolean onCreateOptionsMenu(Menu menu) {
        // Inflate the menu; this adds items to the action bar if it is present.
        getMenuInflater().inflate(R.menu.main, menu);
        return true;
    }

    @Override
    public boolean onOptionsItemSelected(MenuItem item) {
        // Handle action bar item clicks here. The action bar will
        // automatically handle clicks on the Home/Up button, so long
        // as you specify a parent activity in AndroidManifest.xml.
        int id = item.getItemId();
        if (id == R.id.action_settings) {
            return true;
        }
        return super.onOptionsItemSelected(item);
    }

    /**
     * get local ip
     *
     * @return
     */
    private String getLocalIpAddress() {
        try {
            for (Enumeration<NetworkInterface> en = NetworkInterface.getNetworkInterfaces();
                 en.hasMoreElements();) {
                NetworkInterface intf = en.nextElement();
                for (Enumeration<InetAddress> enumIpAddr = intf.getInetAddresses();
                     enumIpAddr.hasMoreElements();) {
                    InetAddress inetAddress = enumIpAddr.nextElement();
                    if (!inetAddress.isLoopbackAddress() && (inetAddress instanceof Inet4Address)) {
                        return inetAddress.getHostAddress().toString();
                    }
                }
            }
        } catch (SocketException ex) {
            Log.e("getLocalIpAddress", ex.toString());
        }
        return "";
    }

    static {
        System.loadLibrary("c++_shared");
        System.loadLibrary("udpclient");
    }
}
