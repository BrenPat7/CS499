package com.example.brendan_clarke_module3;

import androidx.appcompat.app.AppCompatActivity;
import android.content.Intent;
import android.os.Bundle;
import android.view.View;
import android.widget.Button;
import android.widget.EditText;
import android.widget.Toast;

public class MainActivity extends AppCompatActivity {

    private EditText etUsername;
    private EditText etPassword;
    private Button buttonLogin;
    private Button buttonCreate;
    private LoginDatabase loginDatabase;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        etUsername = findViewById(R.id.etUsername);
        etPassword = findViewById(R.id.etPassword);
        buttonLogin = findViewById(R.id.buttonLogin);
        buttonCreate = findViewById(R.id.buttonCreate);

        loginDatabase = new LoginDatabase(this);

        buttonLogin.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                String username = etUsername.getText().toString();
                String password = etPassword.getText().toString();

                if (username.isEmpty() || password.isEmpty()) {
                    Toast.makeText(MainActivity.this, "Username and password cannot be empty.", Toast.LENGTH_SHORT).show();
                    return;
                }

                if (loginDatabase.validateUser(username, password)) {
                    Toast.makeText(MainActivity.this, "Login successful!", Toast.LENGTH_SHORT).show();

                    // Create an intent to open the DataActivity screen
                    Intent intent = new Intent(MainActivity.this, DataActivity.class);
                    startActivity(intent);
                } else {
                    Toast.makeText(MainActivity.this, "Invalid username or password.", Toast.LENGTH_SHORT).show();
                }
            }
        });

        buttonCreate.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                String username = etUsername.getText().toString();
                String password = etPassword.getText().toString();

                if (username.isEmpty() || password.isEmpty()) {
                    Toast.makeText(MainActivity.this, "Username and password cannot be empty.", Toast.LENGTH_SHORT).show();
                    return;
                }

                if (loginDatabase.AddUser(username, password)) {
                    Toast.makeText(MainActivity.this, "Account created successfully!", Toast.LENGTH_SHORT).show();

                    // Create an intent to open the DataActivity screen after a successful account creation
                    Intent intent = new Intent(MainActivity.this, DataActivity.class);
                    startActivity(intent);
                } else {
                    Toast.makeText(MainActivity.this, "Account with that username already exists.", Toast.LENGTH_SHORT).show();
                }
            }
        });
    }
}