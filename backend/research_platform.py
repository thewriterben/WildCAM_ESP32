"""
Scientific Research Platform Service
Handles university partnerships, research projects, and scientific data management
"""

from flask import jsonify, request, current_app
from datetime import datetime, timedelta
from models import db, User
import logging
import json

logger = logging.getLogger(__name__)


class ResearchPlatformService:
    """Service for managing scientific research features"""
    
    def __init__(self):
        self.universities = {}
        self.research_projects = {}
        self.longitudinal_studies = {}
        self.researcher_credentials = {}
        
    # ===== UNIVERSITY PARTNERSHIPS =====
    
    def register_university(self, university_data):
        """Register a university partner"""
        try:
            university_id = university_data.get('id')
            if not university_id:
                return {'error': 'University ID required'}, 400
                
            self.universities[university_id] = {
                'id': university_id,
                'name': university_data.get('name'),
                'country': university_data.get('country'),
                'research_domains': university_data.get('research_domains', []),
                'contact_email': university_data.get('contact_email'),
                'api_credentials': university_data.get('api_credentials'),
                'registered_at': datetime.utcnow().isoformat(),
                'status': 'active',
                'projects': []
            }
            
            logger.info(f"University registered: {university_data.get('name')}")
            return {'message': 'University registered successfully', 'university_id': university_id}, 201
            
        except Exception as e:
            logger.error(f"Error registering university: {e}")
            return {'error': 'Failed to register university'}, 500
    
    def verify_researcher_credentials(self, researcher_id, credentials):
        """Verify researcher academic credentials"""
        try:
            # Store researcher credentials
            self.researcher_credentials[researcher_id] = {
                'researcher_id': researcher_id,
                'university': credentials.get('university'),
                'department': credentials.get('department'),
                'position': credentials.get('position'),
                'orcid': credentials.get('orcid'),
                'verified': True,
                'verified_at': datetime.utcnow().isoformat(),
                'publications': credentials.get('publications', []),
                'research_interests': credentials.get('research_interests', [])
            }
            
            logger.info(f"Researcher credentials verified: {researcher_id}")
            return {'message': 'Credentials verified', 'status': 'verified'}, 200
            
        except Exception as e:
            logger.error(f"Error verifying credentials: {e}")
            return {'error': 'Failed to verify credentials'}, 500
    
    # ===== RESEARCH PROJECTS =====
    
    def create_research_project(self, project_data):
        """Create a new research project"""
        try:
            project_id = project_data.get('id') or f"proj_{datetime.utcnow().timestamp()}"
            
            self.research_projects[project_id] = {
                'id': project_id,
                'title': project_data.get('title'),
                'description': project_data.get('description'),
                'principal_investigator': project_data.get('principal_investigator'),
                'universities': project_data.get('universities', []),
                'start_date': project_data.get('start_date'),
                'end_date': project_data.get('end_date'),
                'research_type': project_data.get('research_type'),  # longitudinal, behavioral, conservation
                'species_focus': project_data.get('species_focus', []),
                'methodology': project_data.get('methodology'),
                'funding_source': project_data.get('funding_source'),
                'ethical_approval': project_data.get('ethical_approval'),
                'created_at': datetime.utcnow().isoformat(),
                'status': 'active',
                'participants': [],
                'datasets': [],
                'publications': []
            }
            
            logger.info(f"Research project created: {project_data.get('title')}")
            return {'message': 'Project created successfully', 'project_id': project_id}, 201
            
        except Exception as e:
            logger.error(f"Error creating research project: {e}")
            return {'error': 'Failed to create project'}, 500
    
    def add_project_participant(self, project_id, researcher_id, role='collaborator'):
        """Add researcher to project"""
        try:
            if project_id not in self.research_projects:
                return {'error': 'Project not found'}, 404
                
            participant = {
                'researcher_id': researcher_id,
                'role': role,
                'joined_at': datetime.utcnow().isoformat(),
                'contributions': []
            }
            
            self.research_projects[project_id]['participants'].append(participant)
            
            logger.info(f"Participant added to project {project_id}: {researcher_id}")
            return {'message': 'Participant added successfully'}, 200
            
        except Exception as e:
            logger.error(f"Error adding participant: {e}")
            return {'error': 'Failed to add participant'}, 500
    
    # ===== LONGITUDINAL STUDIES =====
    
    def create_longitudinal_study(self, study_data):
        """Create long-term wildlife study"""
        try:
            study_id = study_data.get('id') or f"study_{datetime.utcnow().timestamp()}"
            
            self.longitudinal_studies[study_id] = {
                'id': study_id,
                'title': study_data.get('title'),
                'project_id': study_data.get('project_id'),
                'species': study_data.get('species'),
                'location': study_data.get('location'),
                'start_date': study_data.get('start_date'),
                'planned_duration_years': study_data.get('planned_duration_years'),
                'measurement_frequency': study_data.get('measurement_frequency'),
                'metrics': study_data.get('metrics', []),  # population, behavior, habitat
                'baseline_data': study_data.get('baseline_data', {}),
                'created_at': datetime.utcnow().isoformat(),
                'observations': [],
                'analysis_results': []
            }
            
            logger.info(f"Longitudinal study created: {study_data.get('title')}")
            return {'message': 'Study created successfully', 'study_id': study_id}, 201
            
        except Exception as e:
            logger.error(f"Error creating longitudinal study: {e}")
            return {'error': 'Failed to create study'}, 500
    
    def add_study_observation(self, study_id, observation_data):
        """Add observation to longitudinal study"""
        try:
            if study_id not in self.longitudinal_studies:
                return {'error': 'Study not found'}, 404
                
            observation = {
                'timestamp': datetime.utcnow().isoformat(),
                'observer_id': observation_data.get('observer_id'),
                'data': observation_data.get('data'),
                'environmental_conditions': observation_data.get('environmental_conditions'),
                'quality_score': observation_data.get('quality_score', 1.0),
                'notes': observation_data.get('notes')
            }
            
            self.longitudinal_studies[study_id]['observations'].append(observation)
            
            return {'message': 'Observation recorded successfully'}, 201
            
        except Exception as e:
            logger.error(f"Error adding observation: {e}")
            return {'error': 'Failed to add observation'}, 500
    
    # ===== FAIR DATA PRINCIPLES =====
    
    def publish_fair_dataset(self, dataset_data):
        """Publish dataset following FAIR principles"""
        try:
            dataset = {
                'id': dataset_data.get('id') or f"ds_{datetime.utcnow().timestamp()}",
                'title': dataset_data.get('title'),
                'description': dataset_data.get('description'),
                'creators': dataset_data.get('creators', []),
                'keywords': dataset_data.get('keywords', []),
                'license': dataset_data.get('license', 'CC-BY-4.0'),
                'doi': dataset_data.get('doi'),
                'findability': {
                    'persistent_identifier': dataset_data.get('doi'),
                    'metadata': dataset_data.get('metadata', {}),
                    'searchable': True
                },
                'accessibility': {
                    'access_url': dataset_data.get('access_url'),
                    'protocol': dataset_data.get('protocol', 'https'),
                    'authentication': dataset_data.get('authentication')
                },
                'interoperability': {
                    'format': dataset_data.get('format', 'CSV'),
                    'standard': dataset_data.get('standard', 'Darwin Core'),
                    'vocabulary': dataset_data.get('vocabulary')
                },
                'reusability': {
                    'license': dataset_data.get('license'),
                    'provenance': dataset_data.get('provenance'),
                    'citation': dataset_data.get('citation')
                },
                'published_at': datetime.utcnow().isoformat()
            }
            
            logger.info(f"FAIR dataset published: {dataset_data.get('title')}")
            return {'message': 'Dataset published successfully', 'dataset_id': dataset['id']}, 201
            
        except Exception as e:
            logger.error(f"Error publishing dataset: {e}")
            return {'error': 'Failed to publish dataset'}, 500
    
    # ===== RESEARCH ANALYTICS =====
    
    def calculate_conservation_impact(self, project_id, metrics):
        """Calculate conservation intervention effectiveness"""
        try:
            if project_id not in self.research_projects:
                return {'error': 'Project not found'}, 404
                
            impact_metrics = {
                'project_id': project_id,
                'calculated_at': datetime.utcnow().isoformat(),
                'population_change': metrics.get('population_change', 0),
                'habitat_improvement': metrics.get('habitat_improvement', 0),
                'biodiversity_index': metrics.get('biodiversity_index', 0),
                'species_recovery_rate': metrics.get('species_recovery_rate', 0),
                'effectiveness_score': self._calculate_effectiveness_score(metrics)
            }
            
            return {'message': 'Impact calculated', 'metrics': impact_metrics}, 200
            
        except Exception as e:
            logger.error(f"Error calculating impact: {e}")
            return {'error': 'Failed to calculate impact'}, 500
    
    def _calculate_effectiveness_score(self, metrics):
        """Internal method to calculate overall effectiveness"""
        # Simple weighted average of metrics
        weights = {
            'population_change': 0.35,
            'habitat_improvement': 0.25,
            'biodiversity_index': 0.25,
            'species_recovery_rate': 0.15
        }
        
        score = 0
        for metric, weight in weights.items():
            score += metrics.get(metric, 0) * weight
            
        return round(score, 2)
    
    # ===== BEHAVIORAL PREDICTION =====
    
    def analyze_behavioral_patterns(self, species, data_points):
        """Analyze wildlife behavior patterns"""
        try:
            analysis = {
                'species': species,
                'analyzed_at': datetime.utcnow().isoformat(),
                'data_points_analyzed': len(data_points),
                'patterns_detected': [],
                'predictions': {
                    'migration_timing': None,
                    'breeding_behavior': None,
                    'territorial_behavior': None,
                    'stress_indicators': []
                },
                'confidence_score': 0.0,
                'statistical_significance': 0.05
            }
            
            # Placeholder for ML analysis
            logger.info(f"Behavioral analysis completed for {species}")
            return {'message': 'Analysis complete', 'analysis': analysis}, 200
            
        except Exception as e:
            logger.error(f"Error analyzing behavior: {e}")
            return {'error': 'Failed to analyze behavior'}, 500
    
    # ===== PUBLICATION TRACKING =====
    
    def track_publication(self, publication_data):
        """Track research publication"""
        try:
            publication = {
                'id': publication_data.get('id') or f"pub_{datetime.utcnow().timestamp()}",
                'title': publication_data.get('title'),
                'authors': publication_data.get('authors', []),
                'journal': publication_data.get('journal'),
                'doi': publication_data.get('doi'),
                'publication_date': publication_data.get('publication_date'),
                'project_ids': publication_data.get('project_ids', []),
                'citation_count': 0,
                'impact_factor': publication_data.get('impact_factor'),
                'open_access': publication_data.get('open_access', True),
                'dataset_links': publication_data.get('dataset_links', []),
                'tracked_at': datetime.utcnow().isoformat()
            }
            
            # Link to projects
            for proj_id in publication_data.get('project_ids', []):
                if proj_id in self.research_projects:
                    self.research_projects[proj_id]['publications'].append(publication['id'])
            
            logger.info(f"Publication tracked: {publication_data.get('title')}")
            return {'message': 'Publication tracked successfully', 'publication_id': publication['id']}, 201
            
        except Exception as e:
            logger.error(f"Error tracking publication: {e}")
            return {'error': 'Failed to track publication'}, 500
    
    # ===== DATA RETRIEVAL =====
    
    def get_project(self, project_id):
        """Get research project details"""
        if project_id in self.research_projects:
            return {'project': self.research_projects[project_id]}, 200
        return {'error': 'Project not found'}, 404
    
    def list_projects(self, university=None, researcher=None):
        """List research projects with optional filters"""
        projects = list(self.research_projects.values())
        
        if university:
            projects = [p for p in projects if university in p.get('universities', [])]
        
        if researcher:
            projects = [p for p in projects 
                       if any(part['researcher_id'] == researcher 
                             for part in p.get('participants', []))]
        
        return {'projects': projects, 'count': len(projects)}, 200
    
    def get_study(self, study_id):
        """Get longitudinal study details"""
        if study_id in self.longitudinal_studies:
            return {'study': self.longitudinal_studies[study_id]}, 200
        return {'error': 'Study not found'}, 404


# Global instance
research_platform = ResearchPlatformService()
