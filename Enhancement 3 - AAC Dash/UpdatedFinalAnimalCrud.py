import pandas as pd
import dash
from dash import dcc, html
from dash import dash_table
from dash.dependencies import Input, Output
import plotly.express as px
import os

# =========================================
# DATA LOADING & PRE-PROCESSING
# =========================================

csv_path = r"C:\Users\Brend\OneDrive\Desktop\CS499Module5Milestone4\aac_shelter_outcomes.csv"

def load_data():
    if os.path.exists(csv_path):
        print(f"Loading data from {csv_path}...")
        df = pd.read_csv(csv_path)
        
        # 1. STANDARDIZE COLUMN NAMES
        # Converts "Breed" -> "breed", "Sex upon Outcome" -> "sex_upon_outcome"
        df.columns = [c.lower().replace(' ', '_') for c in df.columns]
        
        # 2. CONVERT DATE COLUMN
        # Ensure 'datetime' is actual date objects so the slider works
        if 'datetime' in df.columns:
            df['datetime'] = pd.to_datetime(df['datetime'], errors='coerce')
        
        # Drop rows with critical missing info
        df.dropna(subset=['animal_id'], inplace=True)
        
        print("Data loaded successfully!")
        return df
    else:
        print(f"ERROR: File not found at {csv_path}")
        return pd.DataFrame()

# Load the data once when app starts
global_df = load_data()

# Calculate min/max years for the slider
if not global_df.empty and 'datetime' in global_df.columns:
    min_year = int(global_df['datetime'].dt.year.min())
    max_year = int(global_df['datetime'].dt.year.max())
else:
    min_year = 2013
    max_year = 2023

# =========================================
# DASHBOARD APPLICATION
# =========================================
def get_filtered_data(filter_type, year_range):
    df = global_df.copy()
   
   
    if df.empty:
        return df
        
    #Apply time filter
    start_year, end_year = year_range
    # Filter by the year component of the datetime column
    df = df[
        (df['datetime'].dt.year >= start_year) &
        (df['datetime'].dt.year <= end_year)
    ]

    #APPLY RESCUE FILTER
    if filter_type == "water":
        df = df[
            (df['breed'].isin(["Labrador Retriever Mix", "Chesapeake Bay Retriever", "Newfoundland"])) &
            (df['sex_upon_outcome'] == "Intact Female") &
            (df['age_upon_outcome_in_weeks'] >= 26) & 
            (df['age_upon_outcome_in_weeks'] <= 156)
        ]
        
    elif filter_type == "mountain":
        df = df[
            (df['breed'].isin(["German Shepherd", "Alaskan Malamute", "Old English Sheepdog", "Siberian Husky", "Rottweiler"])) &
            (df['sex_upon_outcome'] == "Intact Male") &
            (df['age_upon_outcome_in_weeks'] >= 26) & 
            (df['age_upon_outcome_in_weeks'] <= 156)
        ]
        
    elif filter_type == "disaster":
        df = df[
            (df['breed'].isin(["Doberman Pinscher", "German Shepherd", "Golden Retriever", "Bloodhound", "Rottweiler"])) &
            (df['sex_upon_outcome'] == "Intact Male") &
            (df['age_upon_outcome_in_weeks'] >= 20) & 
            (df['age_upon_outcome_in_weeks'] <= 300)
        ]
    
    # If reset or no match, return a subset to keep browser fast
    if filter_type == "reset" or len(df) == 0:
        return df.head(1000) 
        
    return df

#Initialize dash
app = dash.Dash(__name__)

#Layout
app.layout = html.Div([
    html.Center(html.B(html.H1('Grazioso Salvare Dashboard'))),
    html.P("Analyze Shelter Data by Year, Rescue Type, and Visualization.", style={'textAlign': 'center'}),
    html.Hr(),
    
    #Controls
    html.Div(style={'display': 'flex', 'flex-direction': 'row', 'padding': '20px', 'backgroundColor': '#f9f9f9'}, children=[
        
        #Visualization dropdown
        html.Div(style={'width': '40%', 'marginRight': '5%'}, children=[
            html.Label("Select Visualization Graph:"),
            dcc.Dropdown(
                id='graph-selector',
                options=[
                    {'label': 'Pie Chart (Top 10 Breeds)', 'value': 'pie'},
                    {'label': 'Bar Chart (Outcomes)', 'value': 'bar'},
                    {'label': 'Scatter Plot (Age vs Type)', 'value': 'scatter'}
                ],
                value='pie',
                clearable=False
            )
        ]),
        
        #Time period slider
        html.Div(style={'width': '55%'}, children=[
            html.Label(f"Filter by Year Range: {min_year} - {max_year}"),
            dcc.RangeSlider(
                id='year-slider',
                min=min_year,
                max=max_year,
                value=[min_year, max_year], # Default to full range
                step=1,
                marks={str(year): str(year) for year in range(min_year, max_year + 1)},
                tooltip={"placement": "bottom", "always_visible": True}
            )]),]),
    
    
    
    html.Hr(),
    
    #Rescue type filter 
    html.Div([
        html.Label("Select Rescue Filter Logic:"),
        dcc.RadioItems(
            id='filter-type',
            options=[
                {'label': 'Water Rescue', 'value': 'water'},
                {'label': 'Mountain/Wilderness Rescue', 'value': 'mountain'},
                {'label': 'Disaster Rescue', 'value': 'disaster'},
                {'label': 'Reset (All Data)', 'value': 'reset'}
            ],
            value='reset',
            labelStyle={'display': 'inline-block', 'margin-right': '20px'}
        )
    ], style={'textAlign': 'center', 'padding': '10px'}),

    #Data Table
    dash_table.DataTable(id='datatable-id',
        columns=[{"name": i, "id": i, "deletable": False, "selectable": True} for i in global_df.columns],
        data=global_df.head(50).to_dict('records'),
        row_selectable="single",
        selected_rows=[0],
        page_size=10,
        style_table={'overflowX': 'auto'},
        style_header={'backgroundColor': 'rgb(30, 30, 30)', 'color': 'white'},
        style_cell={'backgroundColor': 'rgb(50, 50, 50)', 'color': 'white'}
    ),
    
    
    html.Br(),
    html.Hr(),
    
    #Output visualization
    html.Div(className='row', style={'display': 'flex'}, children=[
        #Dynamic chart 
        html.Div(
            id='dynamic-chart-output',
            className='col s12 m6',
            style={'width': '50%', 'padding': '10px'}
        ),
        
        
        
        #Map
        html.Div(
            id='map-id',
            className='col s12 m6',
            style={'width': '50%', 'padding': '10px'}
        )
    ])
])

#CALLBACKS

#To upadte the table based on the slider selection with a filter
@app.callback(
    [Output('datatable-id', 'data'),
     Output('datatable-id', 'columns')],
    [Input('filter-type', 'value'),
     Input('year-slider', 'value')]
)
def update_table(filter_type, year_range):
    df_filtered = get_filtered_data(filter_type, year_range)
    columns = [{"name": i, "id": i} for i in df_filtered.columns]
    return df_filtered.to_dict('records'), columns

#Update graph based on Table Data and the dropdown selection
@app.callback(
    Output('dynamic-chart-output', 'children'),
    [Input('datatable-id', 'derived_virtual_data'),
     Input('graph-selector', 'value')]
)
def update_dynamic_graph(rows, selected_graph):
    if rows is None:
        dff = global_df.head(100)
    else:
        dff = pd.DataFrame(rows)
    
    if dff.empty:
        return html.Div("No data available for graph.")

    #Dropdown selection to choose chart
    if selected_graph == 'pie':
        #limit to top 10 breeds to keep the pie chart readable
        breed_counts = dff['breed'].value_counts().nlargest(10).reset_index()
        breed_counts.columns = ['breed', 'count']
        fig = px.pie(breed_counts, names='breed', values='count', title='Top 10 Breeds in Selection')
        
    elif selected_graph == 'bar':
        outcome_counts = dff['outcome_type'].value_counts().reset_index()
        outcome_counts.columns = ['outcome_type', 'count']
        fig = px.bar(outcome_counts, x='outcome_type', y='count', title='Outcomes by Type')
        
    elif selected_graph == 'scatter':
        #numeric X and category Y
        fig = px.scatter(
            dff, 
            x='age_upon_outcome_in_weeks', 
            y='animal_type', 
            color='sex_upon_outcome',
            title='Age vs. Animal Type',
            labels={'age_upon_outcome_in_weeks': 'Age (Weeks)'}
        )
    else:
        fig = px.pie(dff, names='breed', title='Breed Distribution')

    return dcc.Graph(figure=fig)

#Update Map based on Table Data AND Selected Row
@app.callback(
    Output('map-id', 'children'),
    [Input('datatable-id', 'derived_virtual_data'),
     Input('datatable-id', 'derived_virtual_selected_rows')]
)
def update_map(rows, selected_row_indices):
    if rows is None:
        dff = global_df.head(100)
    else:
        dff = pd.DataFrame(rows)

    if 'location_lat' in dff.columns and 'location_long' in dff.columns:
        #center
        center_lat = dff['location_lat'].mean()
        center_lon = dff['location_long'].mean()
        zoom = 10

        # Highlights selected row
        if selected_row_indices:
            selected_row = selected_row_indices[0]
            if selected_row < len(dff):
                center_lat = dff.iloc[selected_row]['location_lat']
                center_lon = dff.iloc[selected_row]['location_long']
                zoom = 14

        fig = px.scatter_mapbox(
            dff,
            lat="location_lat",
            lon="location_long",
            hover_name="animal_id",
            hover_data=["breed", "sex_upon_outcome"],
            zoom=zoom,
            center={"lat": center_lat, "lon": center_lon}
        )
        
        fig.update_layout(mapbox_style="open-street-map")
        return dcc.Graph(figure=fig)
    
    return html.Div("No location data available")

if __name__ == '__main__':
    app.run(port=8051, debug=True)