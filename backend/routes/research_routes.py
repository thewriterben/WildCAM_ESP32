"""
Research Platform API Routes
Endpoints for scientific research, university partnerships, and data management
"""

from flask import Blueprint, request, jsonify
from auth import researcher_required, admin_required
from research_platform import research_platform
from research_analytics import research_analytics
import logging

logger = logging.getLogger(__name__)

research_bp = Blueprint('research', __name__, url_prefix='/api/research')


# ===== UNIVERSITY PARTNERSHIPS =====

@research_bp.route('/universities', methods=['POST'])
@admin_required
def register_university():
    """Register a university partner"""
    data = request.get_json()
    result, status = research_platform.register_university(data)
    return jsonify(result), status


@research_bp.route('/researchers/<researcher_id>/credentials', methods=['POST'])
@researcher_required
def verify_researcher_credentials(researcher_id):
    """Verify researcher academic credentials"""
    data = request.get_json()
    result, status = research_platform.verify_researcher_credentials(researcher_id, data)
    return jsonify(result), status


# ===== RESEARCH PROJECTS =====

@research_bp.route('/projects', methods=['POST'])
@researcher_required
def create_project():
    """Create a new research project"""
    data = request.get_json()
    result, status = research_platform.create_research_project(data)
    return jsonify(result), status


@research_bp.route('/projects', methods=['GET'])
@researcher_required
def list_projects():
    """List research projects with optional filters"""
    university = request.args.get('university')
    researcher = request.args.get('researcher')
    result, status = research_platform.list_projects(university, researcher)
    return jsonify(result), status


@research_bp.route('/projects/<project_id>', methods=['GET'])
@researcher_required
def get_project(project_id):
    """Get research project details"""
    result, status = research_platform.get_project(project_id)
    return jsonify(result), status


@research_bp.route('/projects/<project_id>/participants', methods=['POST'])
@researcher_required
def add_participant(project_id):
    """Add researcher to project"""
    data = request.get_json()
    researcher_id = data.get('researcher_id')
    role = data.get('role', 'collaborator')
    result, status = research_platform.add_project_participant(project_id, researcher_id, role)
    return jsonify(result), status


# ===== LONGITUDINAL STUDIES =====

@research_bp.route('/studies', methods=['POST'])
@researcher_required
def create_longitudinal_study():
    """Create a new longitudinal study"""
    data = request.get_json()
    result, status = research_platform.create_longitudinal_study(data)
    return jsonify(result), status


@research_bp.route('/studies/<study_id>', methods=['GET'])
@researcher_required
def get_study(study_id):
    """Get longitudinal study details"""
    result, status = research_platform.get_study(study_id)
    return jsonify(result), status


@research_bp.route('/studies/<study_id>/observations', methods=['POST'])
@researcher_required
def add_observation(study_id):
    """Add observation to longitudinal study"""
    data = request.get_json()
    result, status = research_platform.add_study_observation(study_id, data)
    return jsonify(result), status


# ===== FAIR DATA MANAGEMENT =====

@research_bp.route('/datasets', methods=['POST'])
@researcher_required
def publish_dataset():
    """Publish dataset following FAIR principles"""
    data = request.get_json()
    result, status = research_platform.publish_fair_dataset(data)
    return jsonify(result), status


# ===== CONSERVATION IMPACT =====

@research_bp.route('/projects/<project_id>/impact', methods=['POST'])
@researcher_required
def calculate_impact(project_id):
    """Calculate conservation intervention effectiveness"""
    data = request.get_json()
    metrics = data.get('metrics', {})
    result, status = research_platform.calculate_conservation_impact(project_id, metrics)
    return jsonify(result), status


# ===== BEHAVIORAL ANALYSIS =====

@research_bp.route('/analysis/behavior', methods=['POST'])
@researcher_required
def analyze_behavior():
    """Analyze wildlife behavioral patterns"""
    data = request.get_json()
    species = data.get('species')
    data_points = data.get('data_points', [])
    result, status = research_platform.analyze_behavioral_patterns(species, data_points)
    return jsonify(result), status


# ===== PUBLICATION TRACKING =====

@research_bp.route('/publications', methods=['POST'])
@researcher_required
def track_publication():
    """Track research publication"""
    data = request.get_json()
    result, status = research_platform.track_publication(data)
    return jsonify(result), status


# ===== RESEARCH STATISTICS =====

@research_bp.route('/statistics', methods=['GET'])
@researcher_required
def get_research_statistics():
    """Get platform-wide research statistics"""
    stats = {
        'total_projects': len(research_platform.research_projects),
        'total_universities': len(research_platform.universities),
        'active_studies': len(research_platform.longitudinal_studies),
        'verified_researchers': len(research_platform.researcher_credentials)
    }
    return jsonify(stats), 200


# ===== ECOSYSTEM DYNAMICS =====

@research_bp.route('/ecosystem/food-web', methods=['POST'])
@researcher_required
def analyze_food_web():
    """Analyze food web structure"""
    data = request.get_json()
    ecosystem_id = data.get('ecosystem_id')
    species_interactions = data.get('species_interactions', [])
    result, status = research_analytics.analyze_food_web(ecosystem_id, species_interactions)
    return jsonify(result), status


@research_bp.route('/ecosystem/connectivity', methods=['POST'])
@researcher_required
def assess_habitat_connectivity():
    """Assess habitat connectivity"""
    data = request.get_json()
    region_data = data.get('region', {})
    corridors = data.get('corridors', [])
    result, status = research_analytics.assess_habitat_connectivity(region_data, corridors)
    return jsonify(result), status


@research_bp.route('/ecosystem/pollination', methods=['POST'])
@researcher_required
def analyze_pollination_network():
    """Analyze pollination networks"""
    data = request.get_json()
    plants = data.get('plant_species', [])
    pollinators = data.get('pollinator_species', [])
    observations = data.get('observations', [])
    result, status = research_analytics.analyze_pollination_network(plants, pollinators, observations)
    return jsonify(result), status


# ===== PREDICTIVE MODELING =====

@research_bp.route('/prediction/distribution', methods=['POST'])
@researcher_required
def predict_species_distribution():
    """Predict species distribution under climate scenarios"""
    data = request.get_json()
    species = data.get('species')
    environmental_data = data.get('environmental_data', {})
    climate_scenario = data.get('climate_scenario', 'rcp4.5')
    result, status = research_analytics.predict_species_distribution(
        species, environmental_data, climate_scenario
    )
    return jsonify(result), status


@research_bp.route('/prediction/extinction-risk', methods=['POST'])
@researcher_required
def predict_extinction_risk():
    """Predict extinction risk assessment"""
    data = request.get_json()
    species = data.get('species')
    population_data = data.get('population_data', {})
    threats = data.get('threats', [])
    result, status = research_analytics.predict_extinction_risk(species, population_data, threats)
    return jsonify(result), status


@research_bp.route('/prediction/migration', methods=['POST'])
@researcher_required
def predict_migration_timing():
    """Predict migration timing"""
    data = request.get_json()
    species = data.get('species')
    historical_data = data.get('historical_data', [])
    climate_data = data.get('climate_data', {})
    result, status = research_analytics.predict_migration_timing(species, historical_data, climate_data)
    return jsonify(result), status


# ===== BEHAVIORAL ECOLOGY =====

@research_bp.route('/behavior/social-structure', methods=['POST'])
@researcher_required
def analyze_social_structure():
    """Analyze social structure and networks"""
    data = request.get_json()
    species = data.get('species')
    individual_sightings = data.get('individual_sightings', [])
    associations = data.get('associations', [])
    result, status = research_analytics.analyze_social_structure(
        species, individual_sightings, associations
    )
    return jsonify(result), status


@research_bp.route('/behavior/stress-indicators', methods=['POST'])
@researcher_required
def analyze_stress_indicators():
    """Analyze stress indicators in wildlife"""
    data = request.get_json()
    species = data.get('species')
    observations = data.get('observations', [])
    environmental_factors = data.get('environmental_factors', [])
    result, status = research_analytics.analyze_stress_indicators(
        species, observations, environmental_factors
    )
    return jsonify(result), status


# ===== CLIMATE CHANGE IMPACT =====

@research_bp.route('/climate/impact-assessment', methods=['POST'])
@researcher_required
def assess_climate_impact():
    """Assess climate change impacts"""
    data = request.get_json()
    region = data.get('region')
    species_list = data.get('species_list', [])
    climate_data = data.get('climate_data', {})
    result, status = research_analytics.assess_climate_impact(region, species_list, climate_data)
    return jsonify(result), status


# ===== STATISTICAL ANALYSIS =====

@research_bp.route('/statistics/power-analysis', methods=['POST'])
@researcher_required
def perform_power_analysis():
    """Perform statistical power analysis"""
    data = request.get_json()
    study_design = data.get('study_design', {})
    result, status = research_analytics.perform_power_analysis(study_design)
    return jsonify(result), status


@research_bp.route('/statistics/outlier-detection', methods=['POST'])
@researcher_required
def detect_outliers():
    """Detect statistical outliers"""
    data = request.get_json()
    dataset_id = data.get('dataset_id')
    variable = data.get('variable')
    method = data.get('method', 'iqr')
    result, status = research_analytics.detect_outliers(dataset_id, variable, method)
    return jsonify(result), status


def create_research_routes(app):
    """Register research routes with Flask app"""
    app.register_blueprint(research_bp)
    logger.info("Research platform routes registered")
