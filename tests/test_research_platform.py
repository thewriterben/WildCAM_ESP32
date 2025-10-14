"""
Unit tests for Research Platform Services
Tests university partnerships, research projects, and scientific data management
"""

import unittest
from datetime import datetime
import sys
import os

# Add backend to path
sys.path.insert(0, os.path.join(os.path.dirname(__file__), '../backend'))

from research_platform import ResearchPlatformService
from research_analytics import ResearchAnalyticsService


class TestResearchPlatformService(unittest.TestCase):
    """Test cases for Research Platform Service"""
    
    def setUp(self):
        """Set up test fixtures"""
        self.service = ResearchPlatformService()
    
    def tearDown(self):
        """Clean up after tests"""
        self.service.universities.clear()
        self.service.research_projects.clear()
        self.service.longitudinal_studies.clear()
    
    def test_register_university(self):
        """Test university registration"""
        university_data = {
            'id': 'stanford',
            'name': 'Stanford University',
            'country': 'USA',
            'research_domains': ['conservation biology', 'AI'],
            'contact_email': 'research@stanford.edu'
        }
        
        result, status = self.service.register_university(university_data)
        
        self.assertEqual(status, 201)
        self.assertIn('university_id', result)
        self.assertEqual(result['university_id'], 'stanford')
        self.assertIn('stanford', self.service.universities)
    
    def test_verify_researcher_credentials(self):
        """Test researcher credential verification"""
        credentials = {
            'university': 'stanford',
            'department': 'Woods Institute',
            'position': 'Professor',
            'orcid': '0000-0001-2345-6789',
            'publications': ['doi:10.1234/example'],
            'research_interests': ['wildlife monitoring']
        }
        
        result, status = self.service.verify_researcher_credentials(
            'researcher_001',
            credentials
        )
        
        self.assertEqual(status, 200)
        self.assertEqual(result['status'], 'verified')
        self.assertIn('researcher_001', self.service.researcher_credentials)
    
    def test_create_research_project(self):
        """Test research project creation"""
        project_data = {
            'id': 'test_project',
            'title': 'Test Wildlife Study',
            'description': 'Test description',
            'principal_investigator': 'researcher_001',
            'universities': ['stanford', 'oxford'],
            'start_date': '2025-01-01',
            'research_type': 'longitudinal',
            'species_focus': ['Panthera tigris'],
            'funding_source': 'NSF'
        }
        
        result, status = self.service.create_research_project(project_data)
        
        self.assertEqual(status, 201)
        self.assertIn('project_id', result)
        self.assertIn('test_project', self.service.research_projects)
    
    def test_add_project_participant(self):
        """Test adding participant to project"""
        # First create a project
        project_data = {
            'id': 'test_project',
            'title': 'Test Project'
        }
        self.service.create_research_project(project_data)
        
        # Add participant
        result, status = self.service.add_project_participant(
            'test_project',
            'researcher_002',
            'collaborator'
        )
        
        self.assertEqual(status, 200)
        project = self.service.research_projects['test_project']
        self.assertEqual(len(project['participants']), 1)
        self.assertEqual(project['participants'][0]['researcher_id'], 'researcher_002')
    
    def test_create_longitudinal_study(self):
        """Test longitudinal study creation"""
        study_data = {
            'id': 'test_study',
            'title': 'Test Longitudinal Study',
            'project_id': 'test_project',
            'species': 'Panthera tigris',
            'start_date': '2025-01-01',
            'planned_duration_years': 10,
            'metrics': ['population', 'behavior']
        }
        
        result, status = self.service.create_longitudinal_study(study_data)
        
        self.assertEqual(status, 201)
        self.assertIn('study_id', result)
        self.assertIn('test_study', self.service.longitudinal_studies)
    
    def test_add_study_observation(self):
        """Test adding observation to study"""
        # Create study first
        study_data = {
            'id': 'test_study',
            'title': 'Test Study'
        }
        self.service.create_longitudinal_study(study_data)
        
        # Add observation
        observation_data = {
            'observer_id': 'researcher_001',
            'data': {'individuals': 5},
            'environmental_conditions': {'temperature': 22.5}
        }
        
        result, status = self.service.add_study_observation('test_study', observation_data)
        
        self.assertEqual(status, 201)
        study = self.service.longitudinal_studies['test_study']
        self.assertEqual(len(study['observations']), 1)
    
    def test_publish_fair_dataset(self):
        """Test FAIR dataset publication"""
        dataset_data = {
            'id': 'test_dataset',
            'title': 'Test Dataset',
            'description': 'Test description',
            'creators': ['researcher_001'],
            'keywords': ['wildlife', 'camera trap'],
            'license': 'CC-BY-4.0',
            'doi': '10.1234/test'
        }
        
        result, status = self.service.publish_fair_dataset(dataset_data)
        
        self.assertEqual(status, 201)
        self.assertIn('dataset_id', result)
    
    def test_calculate_conservation_impact(self):
        """Test conservation impact calculation"""
        # Create project first
        project_data = {'id': 'impact_project', 'title': 'Impact Test'}
        self.service.create_research_project(project_data)
        
        metrics = {
            'population_change': 0.25,
            'habitat_improvement': 0.40,
            'biodiversity_index': 0.35,
            'species_recovery_rate': 0.20
        }
        
        result, status = self.service.calculate_conservation_impact(
            'impact_project',
            metrics
        )
        
        self.assertEqual(status, 200)
        self.assertIn('metrics', result)
        self.assertIn('effectiveness_score', result['metrics'])
    
    def test_track_publication(self):
        """Test publication tracking"""
        publication_data = {
            'id': 'test_pub',
            'title': 'Test Publication',
            'authors': ['Smith, J.', 'Doe, A.'],
            'journal': 'Conservation Biology',
            'doi': '10.1111/test.12345',
            'publication_date': '2025-12-15',
            'project_ids': [],
            'open_access': True
        }
        
        result, status = self.service.track_publication(publication_data)
        
        self.assertEqual(status, 201)
        self.assertIn('publication_id', result)
    
    def test_get_project(self):
        """Test retrieving project details"""
        # Create project
        project_data = {'id': 'get_test', 'title': 'Get Test'}
        self.service.create_research_project(project_data)
        
        result, status = self.service.get_project('get_test')
        
        self.assertEqual(status, 200)
        self.assertIn('project', result)
        self.assertEqual(result['project']['title'], 'Get Test')
    
    def test_list_projects(self):
        """Test listing projects with filters"""
        # Create multiple projects
        for i in range(3):
            project_data = {
                'id': f'list_test_{i}',
                'title': f'List Test {i}',
                'universities': ['stanford'] if i % 2 == 0 else ['oxford']
            }
            self.service.create_research_project(project_data)
        
        result, status = self.service.list_projects()
        
        self.assertEqual(status, 200)
        self.assertEqual(result['count'], 3)
        
        # Test filtering by university
        result, status = self.service.list_projects(university='stanford')
        self.assertEqual(result['count'], 2)


class TestResearchAnalyticsService(unittest.TestCase):
    """Test cases for Research Analytics Service"""
    
    def setUp(self):
        """Set up test fixtures"""
        self.service = ResearchAnalyticsService()
    
    def test_analyze_food_web(self):
        """Test food web analysis"""
        interactions = [
            {
                'predator': 'Panthera tigris',
                'prey': 'Cervus elaphus',
                'interaction_strength': 0.8
            }
        ]
        
        result, status = self.service.analyze_food_web(
            'test_ecosystem',
            interactions
        )
        
        self.assertEqual(status, 200)
        self.assertIn('analysis', result)
        self.assertIn('trophic_levels', result['analysis'])
    
    def test_assess_habitat_connectivity(self):
        """Test habitat connectivity assessment"""
        region_data = {
            'name': 'Test Region',
            'area': 1000
        }
        corridors = [
            {
                'id': 'corridor_1',
                'width': 500,
                'length': 10,
                'threats': []
            }
        ]
        
        result, status = self.service.assess_habitat_connectivity(
            region_data,
            corridors
        )
        
        self.assertEqual(status, 200)
        self.assertIn('assessment', result)
    
    def test_predict_species_distribution(self):
        """Test species distribution prediction"""
        environmental_data = {
            'temperature_mean': 22.5,
            'precipitation_mm': 2500
        }
        
        result, status = self.service.predict_species_distribution(
            'Panthera tigris',
            environmental_data,
            'rcp4.5'
        )
        
        self.assertEqual(status, 200)
        self.assertIn('prediction', result)
    
    def test_predict_extinction_risk(self):
        """Test extinction risk assessment"""
        population_data = {
            'current_population': 200,
            'trend': 'declining'
        }
        threats = ['habitat_loss', 'poaching']
        
        result, status = self.service.predict_extinction_risk(
            'Panthera tigris',
            population_data,
            threats
        )
        
        self.assertEqual(status, 200)
        self.assertIn('assessment', result)
        self.assertIn('iucn_criteria', result['assessment'])
    
    def test_analyze_social_structure(self):
        """Test social structure analysis"""
        individual_sightings = [
            {'individual_id': 'A', 'location': [0, 0]},
            {'individual_id': 'B', 'location': [0.1, 0.1]}
        ]
        associations = [
            {'individual_a': 'A', 'individual_b': 'B', 'strength': 0.8}
        ]
        
        result, status = self.service.analyze_social_structure(
            'Panthera tigris',
            individual_sightings,
            associations
        )
        
        self.assertEqual(status, 200)
        self.assertIn('analysis', result)
    
    def test_analyze_stress_indicators(self):
        """Test stress indicator analysis"""
        observations = [
            {'behavior': 'vigilant', 'duration': 300}
        ]
        environmental_factors = [
            {'factor': 'human_disturbance', 'impact': 'negative'}
        ]
        
        result, status = self.service.analyze_stress_indicators(
            'Ursus arctos',
            observations,
            environmental_factors
        )
        
        self.assertEqual(status, 200)
        self.assertIn('analysis', result)
        self.assertIn('stressors_identified', result['analysis'])
    
    def test_assess_climate_impact(self):
        """Test climate impact assessment"""
        species_list = ['Panthera tigris', 'Ursus arctos']
        climate_data = {
            'temp_change': 2.5,
            'precip_change': -10
        }
        
        result, status = self.service.assess_climate_impact(
            'himalayan_region',
            species_list,
            climate_data
        )
        
        self.assertEqual(status, 200)
        self.assertIn('assessment', result)
    
    def test_perform_power_analysis(self):
        """Test statistical power analysis"""
        study_design = {
            'type': 'before_after',
            'sample_size': 30,
            'expected_effect_size': 0.5,
            'alpha': 0.05
        }
        
        result, status = self.service.perform_power_analysis(study_design)
        
        self.assertEqual(status, 200)
        self.assertIn('analysis', result)
        self.assertIn('power', result['analysis'])
    
    def test_detect_outliers(self):
        """Test outlier detection"""
        result, status = self.service.detect_outliers(
            'test_dataset',
            'population_count',
            'iqr'
        )
        
        self.assertEqual(status, 200)
        self.assertIn('results', result)


def run_tests():
    """Run all tests"""
    # Create test suite
    loader = unittest.TestLoader()
    suite = unittest.TestSuite()
    
    # Add test cases
    suite.addTests(loader.loadTestsFromTestCase(TestResearchPlatformService))
    suite.addTests(loader.loadTestsFromTestCase(TestResearchAnalyticsService))
    
    # Run tests
    runner = unittest.TextTestRunner(verbosity=2)
    result = runner.run(suite)
    
    # Return exit code
    return 0 if result.wasSuccessful() else 1


if __name__ == '__main__':
    exit(run_tests())
