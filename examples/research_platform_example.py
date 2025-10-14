#!/usr/bin/env python3
"""
WildCAM ESP32 Research Platform Integration Example

Demonstrates how to use the scientific research platform for
collaborative wildlife research and conservation science.

Author: WildCAM Research Team
Date: 2025-10-14
"""

import requests
import json
from datetime import datetime, timedelta


class WildCAMResearchClient:
    """Client for WildCAM Research Platform API"""
    
    def __init__(self, base_url, access_token):
        self.base_url = base_url
        self.headers = {
            'Authorization': f'Bearer {access_token}',
            'Content-Type': 'application/json'
        }
    
    def register_university(self, university_data):
        """Register a university partner"""
        response = requests.post(
            f"{self.base_url}/api/research/universities",
            json=university_data,
            headers=self.headers
        )
        return response.json()
    
    def verify_researcher_credentials(self, researcher_id, credentials):
        """Verify researcher academic credentials"""
        response = requests.post(
            f"{self.base_url}/api/research/researchers/{researcher_id}/credentials",
            json=credentials,
            headers=self.headers
        )
        return response.json()
    
    def create_research_project(self, project_data):
        """Create a new research project"""
        response = requests.post(
            f"{self.base_url}/api/research/projects",
            json=project_data,
            headers=self.headers
        )
        return response.json()
    
    def create_longitudinal_study(self, study_data):
        """Create a longitudinal study"""
        response = requests.post(
            f"{self.base_url}/api/research/studies",
            json=study_data,
            headers=self.headers
        )
        return response.json()
    
    def add_observation(self, study_id, observation_data):
        """Add observation to study"""
        response = requests.post(
            f"{self.base_url}/api/research/studies/{study_id}/observations",
            json=observation_data,
            headers=self.headers
        )
        return response.json()
    
    def publish_fair_dataset(self, dataset_data):
        """Publish dataset following FAIR principles"""
        response = requests.post(
            f"{self.base_url}/api/research/datasets",
            json=dataset_data,
            headers=self.headers
        )
        return response.json()
    
    def analyze_behavioral_patterns(self, species, data_points):
        """Analyze wildlife behavioral patterns"""
        response = requests.post(
            f"{self.base_url}/api/research/analysis/behavior",
            json={'species': species, 'data_points': data_points},
            headers=self.headers
        )
        return response.json()
    
    def calculate_conservation_impact(self, project_id, metrics):
        """Calculate conservation intervention effectiveness"""
        response = requests.post(
            f"{self.base_url}/api/research/projects/{project_id}/impact",
            json={'metrics': metrics},
            headers=self.headers
        )
        return response.json()
    
    def predict_species_distribution(self, species, environmental_data, scenario):
        """Predict species distribution under climate change"""
        response = requests.post(
            f"{self.base_url}/api/research/prediction/distribution",
            json={
                'species': species,
                'environmental_data': environmental_data,
                'climate_scenario': scenario
            },
            headers=self.headers
        )
        return response.json()
    
    def analyze_food_web(self, ecosystem_id, interactions):
        """Analyze ecosystem food web"""
        response = requests.post(
            f"{self.base_url}/api/research/ecosystem/food-web",
            json={
                'ecosystem_id': ecosystem_id,
                'species_interactions': interactions
            },
            headers=self.headers
        )
        return response.json()


def example_1_university_registration():
    """Example 1: Register University and Verify Researcher"""
    print("\n=== Example 1: University Registration ===\n")
    
    client = WildCAMResearchClient(
        base_url='http://localhost:5000',
        access_token='your_admin_token_here'
    )
    
    # Register Stanford University
    university = {
        'id': 'stanford',
        'name': 'Stanford University',
        'country': 'USA',
        'research_domains': [
            'conservation biology',
            'AI for wildlife',
            'computer vision'
        ],
        'contact_email': 'woods-institute@stanford.edu'
    }
    
    result = client.register_university(university)
    print(f"University registration: {result}")
    
    # Verify researcher credentials
    credentials = {
        'university': 'stanford',
        'department': 'Woods Institute for the Environment',
        'position': 'Associate Professor',
        'orcid': '0000-0001-2345-6789',
        'publications': [
            'doi:10.1111/cobi.13892',
            'doi:10.1038/s41559-021-01234-5'
        ],
        'research_interests': [
            'wildlife monitoring',
            'machine learning',
            'conservation technology'
        ]
    }
    
    result = client.verify_researcher_credentials('researcher_001', credentials)
    print(f"Researcher verification: {result}")


def example_2_longitudinal_study():
    """Example 2: Create and Manage Longitudinal Study"""
    print("\n=== Example 2: Longitudinal Study ===\n")
    
    client = WildCAMResearchClient(
        base_url='http://localhost:5000',
        access_token='your_researcher_token_here'
    )
    
    # Create research project
    project = {
        'id': 'tiger_conservation_2025',
        'title': 'Long-term Tiger Population Dynamics Study',
        'description': '20-year study of tiger populations in protected areas',
        'principal_investigator': 'researcher_001',
        'universities': ['stanford', 'oxford', 'nus'],
        'start_date': '2025-01-01',
        'end_date': '2045-01-01',
        'research_type': 'longitudinal',
        'species_focus': ['Panthera tigris'],
        'methodology': 'Camera trap monitoring with AI-based identification',
        'funding_source': 'NSF Grant #ABC123456',
        'ethical_approval': 'Stanford IRB-2024-001'
    }
    
    result = client.create_research_project(project)
    print(f"Project created: {result}")
    
    # Create longitudinal study
    study = {
        'id': 'tiger_study_bhutan',
        'title': 'Bhutan Tiger Population Study',
        'project_id': 'tiger_conservation_2025',
        'species': 'Panthera tigris',
        'location': {
            'lat': 27.5,
            'lon': 88.5,
            'region': 'Royal Manas National Park, Bhutan'
        },
        'start_date': '2025-01-01',
        'planned_duration_years': 20,
        'measurement_frequency': 'monthly',
        'metrics': [
            'population',
            'behavior',
            'habitat_quality',
            'breeding_success'
        ],
        'baseline_data': {
            'estimated_population': 103,
            'breeding_pairs': 28,
            'territory_size_km2': 450,
            'prey_density_per_km2': 12.5
        }
    }
    
    result = client.create_longitudinal_study(study)
    print(f"Study created: {result}")
    
    # Add monthly observations
    for month in range(1, 13):
        observation = {
            'observer_id': 'researcher_001',
            'data': {
                'individuals_detected': 5 + month % 3,
                'breeding_pairs': 2,
                'cubs_observed': 3 if month in [4, 5, 6] else 0,
                'territory_overlap': 0.15,
                'prey_encounters': 8 + month % 5
            },
            'environmental_conditions': {
                'temperature': 15 + month * 2,
                'humidity': 0.70 + (month % 4) * 0.05,
                'season': ['winter', 'spring', 'summer', 'fall'][month // 3]
            },
            'quality_score': 0.95,
            'notes': f'Month {month} observation - clear conditions'
        }
        
        result = client.add_observation('tiger_study_bhutan', observation)
        print(f"Month {month} observation recorded: {result['message']}")


def example_3_fair_data_publication():
    """Example 3: Publish FAIR Dataset"""
    print("\n=== Example 3: FAIR Data Publication ===\n")
    
    client = WildCAMResearchClient(
        base_url='http://localhost:5000',
        access_token='your_researcher_token_here'
    )
    
    # Prepare FAIR dataset
    dataset = {
        'id': 'tiger_camera_trap_2025_q1',
        'title': 'Bengal Tiger Camera Trap Dataset - Q1 2025',
        'description': '''
        Annotated camera trap images of Bengal tigers (Panthera tigris) collected
        from Royal Manas National Park, Bhutan during Q1 2025. Dataset includes
        individual identification, behavioral annotations, and environmental metadata.
        ''',
        'creators': [
            'Dr. Jane Smith (Stanford University)',
            'Dr. John Doe (Oxford WildCRU)',
            'Dr. Wei Chen (National University of Singapore)'
        ],
        'keywords': [
            'Panthera tigris',
            'Bengal tiger',
            'camera trap',
            'conservation',
            'Bhutan',
            'individual identification',
            'behavioral ecology'
        ],
        'license': 'CC-BY-4.0',
        'doi': '10.5061/dryad.tiger2025q1',
        'access_url': 'https://data.wildcam.org/datasets/tiger_2025_q1',
        'protocol': 'https',
        'format': 'Darwin Core Archive',
        'standard': 'Darwin Core',
        'vocabulary': 'GBIF Species Backbone',
        'provenance': 'Collected via WildCAM ESP32 camera traps',
        'citation': 'Smith et al. (2025). Bengal Tiger Camera Trap Dataset - Q1 2025. doi:10.5061/dryad.tiger2025q1',
        'metadata': {
            'temporal_coverage': '2025-01-01 to 2025-03-31',
            'geographic_coverage': 'Royal Manas National Park, Bhutan',
            'taxonomic_coverage': 'Panthera tigris tigris',
            'image_count': 12458,
            'individual_tigers': 42,
            'camera_nights': 2700
        }
    }
    
    result = client.publish_fair_dataset(dataset)
    print(f"Dataset published: {result}")


def example_4_conservation_impact():
    """Example 4: Calculate Conservation Impact"""
    print("\n=== Example 4: Conservation Impact Analysis ===\n")
    
    client = WildCAMResearchClient(
        base_url='http://localhost:5000',
        access_token='your_researcher_token_here'
    )
    
    # Calculate impact metrics
    metrics = {
        'population_change': 0.25,  # 25% increase
        'habitat_improvement': 0.40,  # 40% habitat restoration
        'biodiversity_index': 0.35,  # 35% biodiversity increase
        'species_recovery_rate': 0.20  # 20% recovery rate
    }
    
    result = client.calculate_conservation_impact(
        'tiger_conservation_2025',
        metrics
    )
    print(f"Conservation impact: {json.dumps(result, indent=2)}")


def example_5_predictive_modeling():
    """Example 5: Predictive Species Distribution Modeling"""
    print("\n=== Example 5: Predictive Distribution Modeling ===\n")
    
    client = WildCAMResearchClient(
        base_url='http://localhost:5000',
        access_token='your_researcher_token_here'
    )
    
    # Predict future distribution under climate change
    environmental_data = {
        'current_temperature_mean': 22.5,
        'current_precipitation_mm': 2500,
        'elevation_range': [500, 3000],
        'forest_cover_percent': 75,
        'human_population_density': 45
    }
    
    result = client.predict_species_distribution(
        species='Panthera tigris',
        environmental_data=environmental_data,
        scenario='rcp8.5'  # High emissions scenario
    )
    print(f"Distribution prediction: {json.dumps(result, indent=2)}")


def example_6_ecosystem_analysis():
    """Example 6: Ecosystem Food Web Analysis"""
    print("\n=== Example 6: Ecosystem Food Web Analysis ===\n")
    
    client = WildCAMResearchClient(
        base_url='http://localhost:5000',
        access_token='your_researcher_token_here'
    )
    
    # Define species interactions
    interactions = [
        {
            'predator': 'Panthera tigris',
            'prey': 'Cervus elaphus',
            'interaction_strength': 0.8
        },
        {
            'predator': 'Panthera tigris',
            'prey': 'Sus scrofa',
            'interaction_strength': 0.6
        },
        {
            'predator': 'Canis lupus',
            'prey': 'Cervus elaphus',
            'interaction_strength': 0.7
        },
        {
            'herbivore': 'Cervus elaphus',
            'plant': 'Quercus spp.',
            'interaction_strength': 0.9
        }
    ]
    
    result = client.analyze_food_web(
        ecosystem_id='himalayan_temperate_forest',
        interactions=interactions
    )
    print(f"Food web analysis: {json.dumps(result, indent=2)}")


def main():
    """Run all examples"""
    print("=" * 70)
    print("WildCAM ESP32 Research Platform Integration Examples")
    print("=" * 70)
    
    print("\nNOTE: Replace 'your_admin_token_here' and 'your_researcher_token_here'")
    print("      with actual JWT tokens obtained from /api/auth/login")
    print("\nNOTE: Ensure backend server is running at http://localhost:5000")
    
    # Uncomment to run examples
    # example_1_university_registration()
    # example_2_longitudinal_study()
    # example_3_fair_data_publication()
    # example_4_conservation_impact()
    # example_5_predictive_modeling()
    # example_6_ecosystem_analysis()
    
    print("\n" + "=" * 70)
    print("Examples completed successfully!")
    print("=" * 70)


if __name__ == '__main__':
    main()
