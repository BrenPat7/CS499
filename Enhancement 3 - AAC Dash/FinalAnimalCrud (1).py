import pandas as pd
import dash
from dash import dcc, html
from dash import dash_table
from dash.dependencies import Input, Output
import plotly.express as px
import os

# DATA LOADING (Direct from CSV)


csv_path = r"C:\Users\Brend\OneDrive\Desktop\CS499Module5Milestone4\aac_shelter_outcomes.csv"

def load_data():
    if os.path.exists(csv_path):
        print(f"Loading data from {csv_path}...")
        df = pd.read_csv(csv_path)
        # This converts "Breed" -> "breed", "Sex upon Outcome" -> "sex_upon_outcome"
        # It guarantees the code works regardless of how the CSV is formatted
        df.columns = [c.lower().replace(' ', '_') for c in df.columns]

        df.dropna(subset=['animal_id'], inplace=True)
        print("Data loaded successfully!")
        print(f"Columns found: {list(df.columns)}")
        return df
    else:
        print(f"ERROR: File not found at {csv_path}")
        return pd.DataFrame() # Return empty if fail

# Load the data once when app starts
global_df = load_data()
# DASHBOARD APPLICATION

def get_filtered_data(filter_type):
    df = global_df.copy()
    
    if df.empty:
        return df

    if filter_type == "water":
        #breeds- Labrador Retriever Mix, Chesapeake Bay Retriever, Newfoundland
        #sex, Intact Female
        #age, 26 - 156 weeks
        df = df[
            (df['breed'].isin(["Labrador Retriever Mix", "Chesapeake Bay Retriever", "Newfoundland"])) &
            (df['sex_upon_outcome'] == "Intact Female") &
            (df['age_upon_outcome_in_weeks'] >= 26) & 
            (df['age_upon_outcome_in_weeks'] <= 156)
        ]
        
    elif filter_type == "mountain":
        #breeds- German Shepherd, Alaskan Malamute, Old English Sheepdog, Siberian Husky, Rottweiler
        #sex, Intact Male
        #age, 26 - 156 weeks
        df = df[
            (df['breed'].isin(["German Shepherd", "Alaskan Malamute", "Old English Sheepdog", "Siberian Husky", "Rottweiler"])) &
            (df['sex_upon_outcome'] == "Intact Male") &
            (df['age_upon_outcome_in_weeks'] >= 26) & 
            (df['age_upon_outcome_in_weeks'] <= 156)
        ]
        
    elif filter_type == "disaster":
        # breeds - Doberman Pinscher, German Shepherd, Golden Retriever, Bloodhound, Rottweiler
        #sex, Intact Male
        #age, 20 - 300 weeks
        df = df[
            (df['breed'].isin(["Doberman Pinscher", "German Shepherd", "Golden Retriever", "Bloodhound", "Rottweiler"])) &
            (df['sex_upon_outcome'] == "Intact Male") &
            (df['age_upon_outcome_in_weeks'] >= 20) & 
            (df['age_upon_outcome_in_weeks'] <= 300)
        ]
    if filter_type == "reset" or len(df) == 0:
        return global_df.head(1000)
        
    return df

#dash initlization
app = dash.Dash(__name__)

#layout
app.layout = html.Div([
    html.Center(html.B(html.H1('Grazioso Salvare Dashboard (CSV Mode)'))),
    html.Hr(),
    
    #Radio items for filtering
    html.Div([
        html.Label("Select Rescue Filter:"),
        dcc.RadioItems(
            id='filter-type',
            options=[
                {'label': 'Water Rescue', 'value': 'water'},
                {'label': 'Mountain/Wilderness Rescue', 'value': 'mountain'},
                {'label': 'Disaster Rescue', 'value': 'disaster'},
                {'label': 'Reset (All)', 'value': 'reset'}
            ],
            value='reset',
            labelStyle={'display': 'inline-block', 'margin-right': '20px'}
        )
    ], style={'textAlign': 'center', 'padding': '10px'}),

    html.Hr(),
    
    
    #Data table
    dash_table.DataTable(
        id='datatable-id',
        columns=[{"name": i, "id": i, "deletable": False, "selectable": True} for i in global_df.columns],
        data=global_df.head(50).to_dict('records'), # Load small initial set
        row_selectable="single",
        selected_rows=[0],
        page_size=10,
        style_table={'overflowX': 'auto'},
        style_header={'backgroundColor': 'rgb(30, 30, 30)', 'color': 'white'},
        style_cell={'backgroundColor': 'rgb(50, 50, 50)', 'color': 'white'}
    ),
    
    
    
    html.Br(),
    html.Hr(),
    #Charts and map layout
    html.Div(className='row', style={'display': 'flex'}, children=[
        
        #Pie chart
        html.Div(
            id='graph-id',
            className='col s12 m6',
            style={'width': '50%'}
        ),
        #Map
        html.Div(
            id='map-id',
            className='col s12 m6',
            style={'width': '50%'}
        )
    ])
])
@app.callback(
    [Output('datatable-id', 'data'),
     Output('datatable-id', 'columns')],
    [Input('filter-type', 'value')]
)
def update_dashboard(filter_type):
    df_filtered = get_filtered_data(filter_type)
    columns = [{"name": i, "id": i} for i in df_filtered.columns]
    return df_filtered.to_dict('records'), columns

@app.callback(
    Output('graph-id', 'children'),
    [Input('datatable-id', 'derived_virtual_data')]
)
def update_graph(rows):
    if rows is None:
        dff = global_df.head(100)
    else:
        dff = pd.DataFrame(rows)
    if 'breed' in dff.columns:
        fig = px.pie(dff, names='breed', title='Breed Distribution')
        return dcc.Graph(figure=fig)
    return html.Div("No data for graph")


@app.callback(
    Output('map-id', 'children'),
    [Input('datatable-id', 'derived_virtual_data'),
     Input('datatable-id', 'derived_virtual_selected_rows')])
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

        #Highlights selected rows
        if selected_row_indices:
            selected_row = selected_row_indices[0]
            if selected_row < len(dff):
                center_lat = dff.iloc[selected_row]['location_lat']
                center_lon = dff.iloc[selected_row]['location_long']
                zoom = 15



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