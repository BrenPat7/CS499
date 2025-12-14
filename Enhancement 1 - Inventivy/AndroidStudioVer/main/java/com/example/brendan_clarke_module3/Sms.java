package com.example.brendan_clarke_module3;

import android.Manifest;
import android.content.pm.PackageManager;
import android.os.Bundle;
import android.telephony.SmsManager;
import android.view.View;
import android.widget.Button;
import android.widget.TextView;
import android.widget.Toast;

import androidx.annotation.NonNull;
import androidx.appcompat.app.AppCompatActivity;
import androidx.core.app.ActivityCompat;
import androidx.core.content.ContextCompat;

public class Sms extends AppCompatActivity {

    private static final int SMS_PERMISSION_CODE = 1;

    private TextView textViewStatus;
    private Button buttonEnableSms;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_sms);

        textViewStatus = findViewById(R.id.textViewStatus);
        buttonEnableSms = findViewById(R.id.buttonEnableSms);

        // Check and update the UI on startup
        updatePermissionStatus();

        buttonEnableSms.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                // Check if we already have the permission
                if (ContextCompat.checkSelfPermission(Sms.this, Manifest.permission.SEND_SMS) == PackageManager.PERMISSION_GRANTED) {
                    // Permission is already granted, so send the SMS
                    sendSmsNotification();
                } else {
                    // Permission is not granted, so request it
                    ActivityCompat.requestPermissions(Sms.this, new String[]{Manifest.permission.SEND_SMS}, SMS_PERMISSION_CODE);
                }
            }
        });
    }

    private void updatePermissionStatus() {
        if (ContextCompat.checkSelfPermission(this, Manifest.permission.SEND_SMS) == PackageManager.PERMISSION_GRANTED) {
            textViewStatus.setText("SMS notifications are enabled.");
            buttonEnableSms.setText("Send Test SMS");
        } else {
            textViewStatus.setText("SMS notifications are disabled.");
            buttonEnableSms.setText("Enable SMS Notifications");
        }
    }

    private void sendSmsNotification() {
        try {
            SmsManager smsManager = SmsManager.getDefault();
            // Replace "5551234" with a valid phone number for testing
            smsManager.sendTextMessage("5551234", null, "Your inventory is running low!", null, null);
            Toast.makeText(this, "Test SMS sent successfully!", Toast.LENGTH_SHORT).show();
        } catch (Exception e) {
            Toast.makeText(this, "Failed to send SMS.", Toast.LENGTH_SHORT).show();
            e.printStackTrace();
        }
    }

    @Override
    public void onRequestPermissionsResult(int requestCode, @NonNull String[] permissions, @NonNull int[] grantResults) {
        super.onRequestPermissionsResult(requestCode, permissions, grantResults);
        if (requestCode == SMS_PERMISSION_CODE) {
            if (grantResults.length > 0 && grantResults[0] == PackageManager.PERMISSION_GRANTED) {
                // Permission was granted
                Toast.makeText(this, "SMS permission granted!", Toast.LENGTH_SHORT).show();
                updatePermissionStatus();
            } else {
                // Permission was denied
                Toast.makeText(this, "SMS permission denied. Notifications are disabled.", Toast.LENGTH_LONG).show();
                updatePermissionStatus();
            }
        }
    }
}