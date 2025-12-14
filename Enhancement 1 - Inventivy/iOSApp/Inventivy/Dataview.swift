//
//  Dataview.swift
//  Inventivy
//This replaces Dataactivity.java and the Activity_data.xml
//  Created by m1 on 16/11/2025.
//
import SwiftUI
import SwiftData

struct DataView: View {
    // Accesses the Database
    @Environment(\.modelContext) private var modelContext
    
    // Live database query
    // This @Query automatically fetches all InventoryItems
    // AND updates the UI whenever an item is added or deleted.
    // This replaces GetAllItems() and the entire InventoryAdapter.
    @Query(sort: \InventoryItem.itemName) private var items: [InventoryItem]
    
    // Delcare the variables for @state
    @State private var newItemName = ""
    @State private var newItemQuantity = ""
    
    // State to manage which item we are updating (replaces showUpdateDialog)
    @State private var itemToUpdate: InventoryItem?

    // Building the UI into the same file for simplicity unlike our android studio setup, it was a lot more effort connecting the dots between the two screens
    var body: some View {
        // NavigationStack allows for a title bar
        NavigationStack {
            VStack {
                // This HStsck replaces the inputLayout
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

                // This List replaces the RecyclerView
                List {
                    ForEach(items) { item in
                        // This HStack replaces list_item.xml
                        HStack {
                            Text(item.itemName)
                                .font(.headline)
                            Spacer() // Pushes items apart
                            Text("\(item.quantity)")
                                .font(.subheadline)
                        }
                        // This "swipe to delete" replaces the btnDelete
                        .swipeActions(edge: .trailing, allowsFullSwipe: false) {
                            Button("Delete", role: .destructive) {
                                deleteItem(item)
                            }
                        }
                        // This "tap to update" replaces the btnUpdate
                        .onTapGesture {
                            itemToUpdate = item // Show the update dialog
                        }
                    }
                }
            }
            .navigationTitle("Inventory")
            // This is the "Update Dialog", it replaces update_dialog.xml
            // It appears as a "sheet" from the bottom when itemToUpdate is set.
            .sheet(item: $itemToUpdate) { item in
                UpdateItemView(item: item)
            }
        }
    }

    // Replaces the datactivity.java
    
    // Replaces btnAddItem.setOnClickListener
    private func addItem() {
        guard !newItemName.isEmpty, let quantity = Int(newItemQuantity) else {
            print("Invalid input") // Add alert here later
            return
        }
        
        let newItem = InventoryItem(itemName: newItemName, quantity: quantity)
        modelContext.insert(newItem)
        
        // Clear the text fields
        newItemName = ""
        newItemQuantity = ""
    }
    
    // Replaces DeleteItem() in the adapter
    private func deleteItem(_ item: InventoryItem) {
        modelContext.delete(item)
    }
}

// A separate view for the Update Dialog
struct UpdateItemView: View {
    @Environment(\.modelContext) private var modelContext
    // This closes the sheet when we're done
    @Environment(\.dismiss) private var dismiss
    
    // The item we are editing
    @Bindable var item: InventoryItem
    
    // We use @State for the text fields
    @State private var updatingName: String
    @State private var updatingQuantity: String

    init(item: InventoryItem) {
        self.item = item
        // Pre-fill the state variables
        _updatingName = State(initialValue: item.itemName)
        _updatingQuantity = State(initialValue: "\(item.quantity)")
    }
    
    var body: some View {
        NavigationStack {
            VStack(spacing: 20) {
                TextField("Item Name", text: $updatingName)
                    .textFieldStyle(.roundedBorder)
                
                TextField("Quantity", text: $updatingQuantity)
                    .textFieldStyle(.roundedBorder)
                    .keyboardType(.numberPad)
                
                Button("Update") {
                    // Update the original item
                    if let newQuantity = Int(updatingQuantity) {
                        item.itemName = updatingName
                        item.quantity = newQuantity
                        
                        // No db.Update() needed! SwiftData saves automatically.
                        dismiss() // Close the sheet
                    }
                }
                .buttonStyle(.borderedProminent)
                
                Spacer()
            }
            .padding(30)
            .navigationTitle("Update Item")
            .toolbar {
                ToolbarItem(placement: .cancellationAction) {
                    Button("Cancel") {
                        dismiss()
                    }
                }
            }
        }
    }
}
