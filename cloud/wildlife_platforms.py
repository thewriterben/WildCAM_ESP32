"""
Integration with specialized wildlife monitoring platforms
"""

class WildlifeInsightsAPI:
    """
    Google's Wildlife Insights Platform Integration
    https://wildlifeinsights.org/
    """
    
    def __init__(self, api_key):
        self.base_url = "https://api.wildlifeinsights.org/v1"
        self.api_key = api_key
        
    async def upload_batch(self, images, metadata):
        """Upload batch of camera trap images"""
        endpoint = f"{self.base_url}/projects/{project_id}/deployments/{deployment_id}/images"
        
        for image, meta in zip(images, metadata):
            payload = {
                'deployment_id': meta['deployment_id'],
                'date_time': meta['timestamp'],
                'location': meta['location'],
                'temperature': meta.get('temperature'),
                'humidity': meta.get('humidity'),
                'moon_phase': meta.get('moon_phase'),
                'flash': meta.get('flash', False)
            }
            
            # Upload to platform
            await self.upload_image(image, payload)
            
class TrappezeAPI:
    """
    Trappeze: AI-powered camera trap analysis
    Used by WWF and other conservation organizations
    """
    
    def __init__(self, credentials):
        self.api = TrappezeClient(credentials)
        
    def analyze_deployment(self, deployment_id):
        """Analyze entire camera deployment"""
        results = self.api.process_deployment(deployment_id)
        
        return {
            'species_detected': results['species'],
            'individual_counts': results['counts'],
            'activity_patterns': results['patterns'],
            'biodiversity_metrics': results['metrics']
        }

class INaturalistAPI:
    """
    iNaturalist Computer Vision API
    Community-driven species identification
    """
    
    def __init__(self):
        self.base_url = "https://api.inaturalist.org/v1"
        
    async def identify_species(self, image_path, location=None):
        """Get species suggestions from iNaturalist"""
        endpoint = f"{self.base_url}/computervision/score_image"
        
        with open(image_path, 'rb') as f:
            files = {'image': f}
            params = {
                'observed_on': datetime.now().isoformat(),
                'lat': location[0] if location else None,
                'lng': location[1] if location else None
            }
            
            response = await self.post(endpoint, files=files, params=params)
            
        # Parse top suggestions
        suggestions = response.json()['results']
        return [{
            'species': s['taxon']['name'],
            'common_name': s['taxon'].get('common_name'),
            'confidence': s['score']
        } for s in suggestions[:5]]