//
//  LoginDatabase.swift
//  Inventivy
//
//  Created by m1 on 16/11/2025.
//

import Foundation
import SwiftData
// This macro tells swift data to create a data table
//and this class will replace the users table from android studio
@Model
final class User {
    //Username "text unique" column
    @Attribute(.unique) var username: String
    var password: String
    //Class constructor
    init(username: String, password: String) {
        self.username = username
        self.password = password
    }
}
//This model will replace the inventory items table
@Model
final class InventoryItem {
    //We dont need an ID as swift automatically gives each item an ID
    var itemName: String
    var quantity: Int
    //Constructor or initlaizer
    init(itemName: String, quantity: Int) {
        self.itemName = itemName
        self.quantity = quantity
    }
}
