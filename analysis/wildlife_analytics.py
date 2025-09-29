"""
Advanced analytics for wildlife monitoring data
"""
import pandas as pd
import numpy as np
import plotly.graph_objects as go
from scipy import stats
import networkx as nx
from sklearn.cluster import DBSCAN

class WildlifeAnalytics:
    """
    Comprehensive analytics suite for wildlife data
    """
    
    def __init__(self, database_connection):
        self.db = database_connection
        
    def generate_activity_patterns(self, species, date_range):
        """Generate circadian activity patterns"""
        query = f"""
        SELECT 
            EXTRACT(hour FROM timestamp) as hour,
            COUNT(*) as detections,
            AVG(confidence) as avg_confidence
        FROM detections
        WHERE species = %s 
            AND timestamp BETWEEN %s AND %s
        GROUP BY hour
        ORDER BY hour
        """
        
        data = pd.read_sql(query, self.db, params=[species, *date_range])
        
        # Create circular plot for 24-hour activity
        fig = go.Figure()
        
        theta = np.linspace(0, 360, 24, endpoint=False)
        r = data['detections'].values
        
        fig.add_trace(go.Scatterpolar(
            r=r,
            theta=theta,
            mode='lines+markers',
            name=species,
            fill='toself'
        ))
        
        fig.update_layout(
            polar=dict(
                radialaxis=dict(
                    visible=True,
                    range=[0, max(r)*1.1]
                ),
                angularaxis=dict(
                    tickmode='array',
                    tickvals=list(range(0, 360, 15)),
                    ticktext=[str(i) for i in range(24)]
                )
            ),
            showlegend=True,
            title=f"24-Hour Activity Pattern: {species}"
        )
        
        return fig
        
    def calculate_biodiversity_indices(self, site_id, period):
        """Calculate biodiversity metrics"""
        # Get species abundance data
        query = """
        SELECT species, COUNT(*) as abundance
        FROM detections
        WHERE site_id = %s AND timestamp > NOW() - INTERVAL %s
        GROUP BY species
        """
        
        data = pd.read_sql(query, self.db, params=[site_id, period])
        abundances = data['abundance'].values
        
        # Shannon Diversity Index
        proportions = abundances / abundances.sum()
        shannon = -np.sum(proportions * np.log(proportions))
        
        # Simpson's Diversity Index
        simpson = 1 - np.sum(proportions ** 2)
        
        # Species Richness
        richness = len(abundances)
        
        # Pielou's Evenness
        evenness = shannon / np.log(richness) if richness > 1 else 0
        
        return {
            'shannon_index': shannon,
            'simpson_index': simpson,
            'species_richness': richness,
            'evenness': evenness,
            'total_detections': abundances.sum()
        }
        
    def detect_migration_patterns(self, species, locations):
        """Analyze migration patterns from multiple camera locations"""
        # Build temporal-spatial network
        G = nx.DiGraph()
        
        for location in locations:
            detections = self.get_detections(species, location)
            
            for i, detection in enumerate(detections[:-1]):
                next_detection = detections[i + 1]
                
                # Add edge between consecutive detections
                G.add_edge(
                    (location, detection['timestamp']),
                    (next_detection['location'], next_detection['timestamp']),
                    weight=1/(next_detection['timestamp'] - detection['timestamp']).days
                )
        
        # Analyze movement patterns
        migration_routes = nx.all_shortest_paths(G, weight='weight')
        centrality = nx.betweenness_centrality(G)
        
        return {
            'routes': list(migration_routes),
            'key_locations': sorted(centrality.items(), 
                                   key=lambda x: x[1], reverse=True)[:10]
        }