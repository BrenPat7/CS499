//
//  LoginView.swift
//  Inventivy
// This file is the Mainactivity.java and the activity_main.xml
//  Created by m1 on 16/11/2025.
//
import SwiftUI
import SwiftData

struct LoginView: View {
    //Access the database
    @Environment(\.modelContext) private var modelContext
    
    // State the variable that are used to store use input
    //This will replace the findViewbyID and the .gettext()
    @State private var username = ""
    @State private var password = ""
    // State for showing error alerts
    @State private var showingAlert = false
    @State private var alertMessage = ""

    // This variable will be set to 'true' on a successful login
    // to tell the app to navigate to the next screen.
    @Binding var isAuthenticated: Bool

    //the UI replaces activity_main.xml
    var body: some View {
    // VStack Vertical Stack arranges things from top to bottom.
        VStack(spacing: 20) {
            Image(systemName: "shippingbox") // A default icon (replaces ic_menu_gallery)
                .font(.system(size: 100))
                .padding(.bottom, 30)

                // Replaces etUsername
            TextField("Username", text: $username)
                .textFieldStyle(.roundedBorder)
                .autocapitalization(.none) // Don't auto-capitalize usernames

            // Replaces etPassword
            SecureField("Password", text: $password)
                .textFieldStyle(.roundedBorder)

            // Replaces buttonLogin
            Button("Login") {
                loginUser()
            }
            .buttonStyle(.borderedProminent) // A modern button style
            .padding(.top)

            // Replaces buttonCreate
            Button("New User") {
                createUser()
            }
        }
        .padding() // Replaces the android:layout_margin="16dp"
        // This is the equivalent of a Toast message.
        .alert(alertMessage, isPresented: $showingAlert) {
            Button("OK", role: .cancel) { }
        }
    }

    //This replaces button click listeners
        
    // Equivalent of loginDatabase.validateUser()
    private func loginUser() {
        if username.isEmpty || password.isEmpty {
            showAlert("Username and password cannot be empty.")
            return
        }
            
        // Create a query to find the user
        let descriptor = FetchDescriptor<User>(
            predicate: #Predicate { $0.username == username && $0.password == password }
        )
            
        // Try to fetch the user
        do {
            let users = try modelContext.fetch(descriptor)
            if let user = users.first {
            // Success
            print("Login successful for \(user.username)")
                isAuthenticated = true // This will trigger navigation
            } else {
            // Fail
                showAlert("Invalid username or password.")
            }
        } catch {
            showAlert("Database error: \(error.localizedDescription)")
        }
    }
        
    // Replaced loginDatabase.AddUser()
    private func createUser() {
        if username.isEmpty || password.isEmpty {
            showAlert("Username and password cannot be empty.")
            return
        }

        // Checks if user already exists because username must be unique
        let descriptor = FetchDescriptor<User>(
            predicate: #Predicate { $0.username == username }
        )
            
        do {
            let existingUsers = try modelContext.fetch(descriptor)
            guard existingUsers.isEmpty else {
                showAlert("Account with that username already exists.")
                return
            }
                
            // If not, create and insert the new user
            let newUser = User(username: username, password: password)
            modelContext.insert(newUser)
                
            // Added, make it auto-save
            try modelContext.save()
                
            print("Account created for \(newUser.username)")
            isAuthenticated = true // Log in and navigate
                
        } catch {
            showAlert("Database error: \(error.localizedDescription)")
        }
    }
        
    private func showAlert(_ message: String) {
        alertMessage = message
        showingAlert = true
    }}
