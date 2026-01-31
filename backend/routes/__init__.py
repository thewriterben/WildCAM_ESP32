# Routes package
from .alert_routes import alert_bp
from .push_routes import push_routes
from .research_routes import research_bp
from .satellite_routes import satellite_routes
from .population_routes import population_routes

__all__ = [
    'alert_bp',
    'push_routes',
    'research_bp',
    'satellite_routes',
    'population_routes'
]