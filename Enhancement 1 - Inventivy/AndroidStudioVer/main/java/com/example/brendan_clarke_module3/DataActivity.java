package com.example.brendan_clarke_module3;

import android.app.AlertDialog;
import android.content.Context;
import android.content.DialogInterface;
import android.database.Cursor;
import android.os.Bundle;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.Button;
import android.widget.EditText;
import android.widget.ImageButton;
import android.widget.TextView;
import android.widget.Toast;

import androidx.annotation.NonNull;
import androidx.appcompat.app.AppCompatActivity;
import androidx.recyclerview.widget.LinearLayoutManager;
import androidx.recyclerview.widget.RecyclerView;

public class DataActivity extends AppCompatActivity {

    private EditText etItemName, etQuantity;
    private Button btnAddItem;
    private RecyclerView rvDataGrid;
    private LoginDatabase dbHelper;
    private InventoryAdapter adapter;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_data);

        etItemName = findViewById(R.id.etItemName);
        etQuantity = findViewById(R.id.etQuantity);
        btnAddItem = findViewById(R.id.btnAddItem);
        rvDataGrid = findViewById(R.id.rvDataGrid);

        dbHelper = new LoginDatabase(this);

        rvDataGrid.setLayoutManager(new LinearLayoutManager(this));

        displayItems();

        btnAddItem.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                String itemName = etItemName.getText().toString();
                String quantityStr = etQuantity.getText().toString();

                if (itemName.isEmpty() || quantityStr.isEmpty()) {
                    Toast.makeText(DataActivity.this, "Please fill in all fields.", Toast.LENGTH_SHORT).show();
                    return;
                }

                int quantity = Integer.parseInt(quantityStr);

                boolean isAdded = dbHelper.AddItem(itemName, quantity);
                if (isAdded) {
                    Toast.makeText(DataActivity.this, "Item added successfully!", Toast.LENGTH_SHORT).show();
                    etItemName.setText("");
                    etQuantity.setText("");
                    displayItems();
                } else {
                    Toast.makeText(DataActivity.this, "Error adding item.", Toast.LENGTH_SHORT).show();
                }
            }
        });
    }

    public void displayItems() {
        Cursor cursor = dbHelper.GetAllItems();
        if (adapter == null) {
            adapter = new InventoryAdapter(cursor);
            rvDataGrid.setAdapter(adapter);
        } else {
            adapter.changeCursor(cursor);
        }
    }

    public void showUpdateDialog(final int itemId, String currentItemName, int currentQuantity) {
        AlertDialog.Builder builder = new AlertDialog.Builder(this);
        builder.setTitle("Update Item");

        final View dialogView = getLayoutInflater().inflate(R.layout.update_dialog, null);
        final EditText etNewItemName = dialogView.findViewById(R.id.etNewItemName);
        final EditText etNewQuantity = dialogView.findViewById(R.id.etNewQuantity);

        etNewItemName.setText(currentItemName);
        etNewQuantity.setText(String.valueOf(currentQuantity));

        builder.setView(dialogView);

        builder.setPositiveButton("Update", new DialogInterface.OnClickListener() {
            @Override
            public void onClick(DialogInterface dialog, int which) {
                String newItemName = etNewItemName.getText().toString();
                String newQuantityStr = etNewQuantity.getText().toString();

                if (newItemName.isEmpty() || newQuantityStr.isEmpty()) {
                    Toast.makeText(DataActivity.this, "Please fill in all fields.", Toast.LENGTH_SHORT).show();
                    return;
                }

                int newQuantity = Integer.parseInt(newQuantityStr);

                boolean isUpdated = dbHelper.UpdateItemQuantity(itemId, newQuantity);
                if (isUpdated) {
                    Toast.makeText(DataActivity.this, "Item updated successfully!", Toast.LENGTH_SHORT).show();
                    displayItems();
                } else {
                    Toast.makeText(DataActivity.this, "Error updating item.", Toast.LENGTH_SHORT).show();
                }
            }
        });

        builder.setNegativeButton("Cancel", null);

        builder.create().show();
    }
}

class InventoryAdapter extends RecyclerView.Adapter<InventoryAdapter.ViewHolder> {

    private Cursor cursor;

    public InventoryAdapter(Cursor cursor) {
        this.cursor = cursor;
    }

    @NonNull
    @Override
    public ViewHolder onCreateViewHolder(@NonNull ViewGroup parent, int viewType) {
        View view = LayoutInflater.from(parent.getContext()).inflate(R.layout.list_item, parent, false);
        return new ViewHolder(view);
    }

    @Override
    public void onBindViewHolder(@NonNull ViewHolder holder, int position) {
        if (cursor.moveToPosition(position)) {
            final int itemId = cursor.getInt(cursor.getColumnIndexOrThrow("id"));
            final String itemName = cursor.getString(cursor.getColumnIndexOrThrow("item_name"));
            final int quantity = cursor.getInt(cursor.getColumnIndexOrThrow("quantity"));
            holder.tvItemName.setText(itemName);
            holder.tvQuantity.setText(String.valueOf(quantity));

            holder.btnDelete.setOnClickListener(new View.OnClickListener() {
                @Override
                public void onClick(View v) {
                    LoginDatabase dbHelper = new LoginDatabase(v.getContext());
                    dbHelper.DeleteItem(itemId);
                    ((DataActivity) v.getContext()).displayItems();
                }
            });

            holder.btnUpdate.setOnClickListener(new View.OnClickListener() {
                @Override
                public void onClick(View v) {
                    ((DataActivity) v.getContext()).showUpdateDialog(itemId, itemName, quantity);
                }
            });
        }
    }

    @Override
    public int getItemCount() {
        return cursor != null ? cursor.getCount() : 0;
    }

    public void changeCursor(Cursor newCursor) {
        if (cursor != null) {
            cursor.close();
        }
        cursor = newCursor;
        if (newCursor != null) {
            this.notifyDataSetChanged();
        }
    }

    static class ViewHolder extends RecyclerView.ViewHolder {
        TextView tvItemName;
        TextView tvQuantity;
        ImageButton btnUpdate;
        ImageButton btnDelete;

        ViewHolder(@NonNull View itemView) {
            super(itemView);
            tvItemName = itemView.findViewById(R.id.tvItemName);
            tvQuantity = itemView.findViewById(R.id.tvQuantity);
            btnUpdate = itemView.findViewById(R.id.btnUpdate);
            btnDelete = itemView.findViewById(R.id.btnDelete);
        }
    }
}