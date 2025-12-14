//
//  ContentView.swift
//  Inventivy
//
//  Created by m1 on 16/11/2025.
//

import SwiftUI
import SwiftData

// This is your inventory list screen
struct ContentView: View {
    @Environment(\.modelContext) private var modelContext
    
    // This @Query *correctly* asks for InventoryItem
    @Query(sort: \InventoryItem.itemName) private var items: [InventoryItem]
    
    @State private var newItemName = ""
    @State private var newItemQuantity = ""
    @State private var itemToUpdate: InventoryItem?

    var body: some View {
        NavigationStack {
            VStack {
                // Input form
                HStack {
                    TextField("Item Name", text: $newItemName)
                        .textFieldStyle(.roundedBorder)
                    
                    TextField("Qty", text: $newItemQuantity)
                        .textFieldStyle(.roundedBorder)
                        .keyboardType(.numberPad)
                        .frame(width: 60)
                    
                    Button("Add") {
                        addItem()
                    }
                    .buttonStyle(.bordered)
                }
                .padding()

            
                List {
                    ForEach(items) { item in
                        HStack {
                            Text(item.itemName)
                                .font(.headline)
                            Spacer()
                            Text("\(item.quantity)")
                                .font(.subheadline)
                        }
                        .swipeActions(edge: .trailing, allowsFullSwipe: false) {
                            Button("Delete", role: .destructive) {
                                deleteItem(item)
                            }
                        }
                        .onTapGesture {
                            itemToUpdate = item
                        }
                    }
                }
            }
            .navigationTitle("Inventory")
            .sheet(item: $itemToUpdate) { item in
                UpdateItemView(item: item)
            }
        }
    }

    private func addItem() {
        guard !newItemName.isEmpty, let quantity = Int(newItemQuantity) else {
            print("Invalid input")
            return
        }
        
        // This creates an InventoryItem
        let newItem = InventoryItem(itemName: newItemName, quantity: quantity)
        modelContext.insert(newItem)
        
        newItemName = ""
        newItemQuantity = ""
    }
    
    private func deleteItem(_ item: InventoryItem) {
        modelContext.delete(item)
    }
}


