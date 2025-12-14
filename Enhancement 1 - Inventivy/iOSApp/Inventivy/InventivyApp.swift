//
//  InventivyApp.swift
//  Inventivy
//
//  Created by m1 on 16/11/2025.
//

import SwiftUI
import SwiftData

@main
struct InventivvApp: App {
    // This state variable will track if the user is logged in
    @State private var isAuthenticated = false

    var body: some Scene {
        WindowGroup {
            // This is the logic to show the correct view
            if isAuthenticated {
                // If logged in, show the inventory list
                ContentView()
            } else {
                // If not logged in, show the LoginView
                // We pass a "binding" so LoginView can tell us
                // when the user is successfully logged in.
                LoginView(isAuthenticated: $isAuthenticated)
            }
        }
      
        .modelContainer(for: [User.self, InventoryItem.self])
    }
}
