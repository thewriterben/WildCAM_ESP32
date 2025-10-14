# External Platform Integrations Guide

## Overview

This document provides integration specifications for connecting WildCAM with external platforms, including academic databases, cloud storage, communication tools, and research platforms.

## Academic Database Integrations

### GBIF (Global Biodiversity Information Facility)

**Purpose**: Share wildlife observations with global biodiversity database

**API Documentation**: https://www.gbif.org/developer/summary

**Integration Steps**:

```python
# backend/integrations/gbif.py
import requests
from datetime import datetime

class GBIFIntegration:
    """
    Integration with GBIF API for biodiversity data sharing
    """
    
    BASE_URL = "https://api.gbif.org/v1"
    
    def __init__(self, username, password):
        self.auth = (username, password)
    
    def submit_occurrence(self, detection_data):
        """
        Submit wildlife detection as occurrence record
        
        Args:
            detection_data: {
                'species': 'Ursus arctos',
                'latitude': 45.123,
                'longitude': -122.456,
                'timestamp': '2025-01-15T10:30:00Z',
                'image_url': 'https://...',
                'confidence': 0.95
            }
        """
        occurrence = {
            'scientificName': detection_data['species'],
            'decimalLatitude': detection_data['latitude'],
            'decimalLongitude': detection_data['longitude'],
            'eventDate': detection_data['timestamp'],
            'basisOfRecord': 'MACHINE_OBSERVATION',
            'identificationVerificationStatus': 'unverified',
            'occurrenceRemarks': f"Automated detection (confidence: {detection_data['confidence']})",
            'associatedMedia': detection_data.get('image_url')
        }
        
        response = requests.post(
            f"{self.BASE_URL}/occurrence",
            json=occurrence,
            auth=self.auth
        )
        
        return response.json()
    
    def search_species(self, species_name):
        """Search for species information"""
        response = requests.get(
            f"{self.BASE_URL}/species/search",
            params={'q': species_name}
        )
        return response.json()
```

**Configuration**:
```env
GBIF_USERNAME=your_username
GBIF_PASSWORD=your_password
GBIF_DATASET_KEY=your_dataset_key
```

### iNaturalist

**Purpose**: Citizen science observations and species identification

**API Documentation**: https://api.inaturalist.org/v1/docs/

**Integration Example**:

```python
# backend/integrations/inaturalist.py
class iNaturalistIntegration:
    BASE_URL = "https://api.inaturalist.org/v1"
    
    def create_observation(self, detection_data, user_token):
        """Create iNaturalist observation from detection"""
        observation = {
            'observation': {
                'species_guess': detection_data['species'],
                'observed_on_string': detection_data['timestamp'],
                'latitude': detection_data['latitude'],
                'longitude': detection_data['longitude'],
                'description': f"Automated camera trap detection\nConfidence: {detection_data['confidence']}",
                'tag_list': 'camera-trap,automated,wildcam'
            }
        }
        
        headers = {'Authorization': f'Bearer {user_token}'}
        response = requests.post(
            f"{self.BASE_URL}/observations",
            json=observation,
            headers=headers
        )
        
        return response.json()
```

### eBird

**Purpose**: Bird observation database

**API Documentation**: https://documenter.getpostman.com/view/664302/S1ENwy59

**Integration Example**:

```python
# backend/integrations/ebird.py
class eBirdIntegration:
    BASE_URL = "https://api.ebird.org/v2"
    
    def __init__(self, api_key):
        self.headers = {'X-eBirdApiToken': api_key}
    
    def get_species_in_region(self, latitude, longitude, radius_km=25):
        """Get recent bird sightings in area"""
        response = requests.get(
            f"{self.BASE_URL}/data/obs/geo/recent",
            params={
                'lat': latitude,
                'lng': longitude,
                'dist': radius_km
            },
            headers=self.headers
        )
        return response.json()
    
    def submit_checklist(self, bird_detections):
        """Submit checklist of bird observations"""
        # Requires eBird mobile app API access
        pass
```

## Cloud Storage Integrations

### Google Drive

**Purpose**: Backup images and data to Google Drive

**Setup**:

```bash
pip install google-auth google-auth-oauthlib google-auth-httplib2 google-api-python-client
```

**Integration**:

```python
# backend/integrations/google_drive.py
from google.oauth2.credentials import Credentials
from googleapiclient.discovery import build
from googleapiclient.http import MediaFileUpload

class GoogleDriveIntegration:
    def __init__(self, credentials_json):
        self.creds = Credentials.from_authorized_user_info(credentials_json)
        self.service = build('drive', 'v3', credentials=self.creds)
    
    def upload_image(self, file_path, folder_id=None):
        """Upload image to Google Drive"""
        file_metadata = {
            'name': os.path.basename(file_path),
            'parents': [folder_id] if folder_id else []
        }
        
        media = MediaFileUpload(file_path, mimetype='image/jpeg')
        
        file = self.service.files().create(
            body=file_metadata,
            media_body=media,
            fields='id,webViewLink'
        ).execute()
        
        return file
    
    def create_shared_folder(self, folder_name):
        """Create shared folder for team"""
        file_metadata = {
            'name': folder_name,
            'mimeType': 'application/vnd.google-apps.folder'
        }
        
        folder = self.service.files().create(
            body=file_metadata,
            fields='id'
        ).execute()
        
        # Make folder shared
        permission = {
            'type': 'anyone',
            'role': 'reader'
        }
        self.service.permissions().create(
            fileId=folder['id'],
            body=permission
        ).execute()
        
        return folder
```

### AWS S3

**Purpose**: Scalable object storage for images and videos

**Integration**:

```python
# backend/integrations/aws_s3.py
import boto3
from botocore.exceptions import ClientError

class S3Integration:
    def __init__(self, bucket_name, region='us-west-2'):
        self.s3_client = boto3.client('s3', region_name=region)
        self.bucket_name = bucket_name
    
    def upload_detection_image(self, file_path, detection_id):
        """Upload detection image to S3"""
        object_name = f"detections/{detection_id}/{os.path.basename(file_path)}"
        
        try:
            self.s3_client.upload_file(
                file_path,
                self.bucket_name,
                object_name,
                ExtraArgs={
                    'ContentType': 'image/jpeg',
                    'Metadata': {
                        'detection_id': str(detection_id),
                        'uploaded_at': datetime.utcnow().isoformat()
                    }
                }
            )
            
            # Generate presigned URL
            url = self.s3_client.generate_presigned_url(
                'get_object',
                Params={'Bucket': self.bucket_name, 'Key': object_name},
                ExpiresIn=3600
            )
            
            return url
        except ClientError as e:
            print(f"Error uploading to S3: {e}")
            return None
```

## Communication Platform Integrations

### Slack

**Purpose**: Team notifications and chat integration

**Setup**:

```bash
pip install slack-sdk
```

**Integration**:

```python
# backend/integrations/slack.py
from slack_sdk import WebClient
from slack_sdk.errors import SlackApiError

class SlackIntegration:
    def __init__(self, bot_token):
        self.client = WebClient(token=bot_token)
    
    def send_detection_alert(self, channel, detection):
        """Send detection alert to Slack channel"""
        try:
            response = self.client.chat_postMessage(
                channel=channel,
                text=f"🐾 New {detection['species']} detected!",
                blocks=[
                    {
                        "type": "header",
                        "text": {
                            "type": "plain_text",
                            "text": f"🐾 {detection['species']} Detected"
                        }
                    },
                    {
                        "type": "section",
                        "fields": [
                            {
                                "type": "mrkdwn",
                                "text": f"*Camera:*\n{detection['camera_name']}"
                            },
                            {
                                "type": "mrkdwn",
                                "text": f"*Confidence:*\n{detection['confidence']*100:.1f}%"
                            },
                            {
                                "type": "mrkdwn",
                                "text": f"*Time:*\n{detection['timestamp']}"
                            }
                        ]
                    },
                    {
                        "type": "image",
                        "image_url": detection['image_url'],
                        "alt_text": f"{detection['species']} detection"
                    },
                    {
                        "type": "actions",
                        "elements": [
                            {
                                "type": "button",
                                "text": {
                                    "type": "plain_text",
                                    "text": "View Details"
                                },
                                "url": f"https://wildcam.app/detection/{detection['id']}"
                            }
                        ]
                    }
                ]
            )
            return response
        except SlackApiError as e:
            print(f"Error sending Slack message: {e}")
    
    def create_discussion_thread(self, channel, detection_id):
        """Create discussion thread for detection"""
        response = self.client.chat_postMessage(
            channel=channel,
            text=f"Discussion thread for detection #{detection_id}"
        )
        return response['ts']  # Thread timestamp
```

**Webhook Configuration**:
```python
# For incoming webhooks
def send_webhook_notification(webhook_url, message):
    """Send notification via Slack webhook"""
    payload = {
        "text": message,
        "username": "WildCAM Bot",
        "icon_emoji": ":camera:"
    }
    requests.post(webhook_url, json=payload)
```

### Discord

**Purpose**: Gaming/community-style team communication

**Integration**:

```python
# backend/integrations/discord.py
import discord
from discord import Webhook
import aiohttp

class DiscordIntegration:
    def __init__(self, webhook_url):
        self.webhook_url = webhook_url
    
    async def send_detection_alert(self, detection):
        """Send detection alert to Discord via webhook"""
        async with aiohttp.ClientSession() as session:
            webhook = Webhook.from_url(self.webhook_url, session=session)
            
            embed = discord.Embed(
                title=f"🐾 {detection['species']} Detected",
                color=discord.Color.green(),
                timestamp=datetime.fromisoformat(detection['timestamp'])
            )
            
            embed.add_field(
                name="Camera",
                value=detection['camera_name'],
                inline=True
            )
            embed.add_field(
                name="Confidence",
                value=f"{detection['confidence']*100:.1f}%",
                inline=True
            )
            embed.set_image(url=detection['image_url'])
            embed.set_footer(text="WildCAM ESP32")
            
            await webhook.send(embed=embed, username="WildCAM Bot")
```

### Microsoft Teams

**Purpose**: Enterprise team collaboration

**Integration**:

```python
# backend/integrations/teams.py
import requests

class TeamsIntegration:
    def __init__(self, webhook_url):
        self.webhook_url = webhook_url
    
    def send_adaptive_card(self, detection):
        """Send detection as adaptive card to Teams"""
        card = {
            "type": "message",
            "attachments": [
                {
                    "contentType": "application/vnd.microsoft.card.adaptive",
                    "content": {
                        "type": "AdaptiveCard",
                        "version": "1.4",
                        "body": [
                            {
                                "type": "TextBlock",
                                "text": f"🐾 {detection['species']} Detected",
                                "size": "Large",
                                "weight": "Bolder"
                            },
                            {
                                "type": "FactSet",
                                "facts": [
                                    {
                                        "title": "Camera:",
                                        "value": detection['camera_name']
                                    },
                                    {
                                        "title": "Confidence:",
                                        "value": f"{detection['confidence']*100:.1f}%"
                                    },
                                    {
                                        "title": "Time:",
                                        "value": detection['timestamp']
                                    }
                                ]
                            },
                            {
                                "type": "Image",
                                "url": detection['image_url'],
                                "size": "Large"
                            }
                        ],
                        "actions": [
                            {
                                "type": "Action.OpenUrl",
                                "title": "View Details",
                                "url": f"https://wildcam.app/detection/{detection['id']}"
                            }
                        ]
                    }
                }
            ]
        }
        
        response = requests.post(self.webhook_url, json=card)
        return response.status_code == 200
```

## Research Platform Integrations

### GitHub

**Purpose**: Code repository integration for research scripts

**Integration**:

```python
# backend/integrations/github.py
from github import Github

class GitHubIntegration:
    def __init__(self, access_token):
        self.g = Github(access_token)
    
    def create_research_repo(self, org_name, repo_name, description):
        """Create repository for research project"""
        org = self.g.get_organization(org_name)
        repo = org.create_repo(
            name=repo_name,
            description=description,
            private=True,
            has_issues=True,
            has_wiki=True
        )
        return repo
    
    def upload_analysis_script(self, repo_name, file_path, commit_message):
        """Upload analysis script to repository"""
        repo = self.g.get_repo(repo_name)
        
        with open(file_path, 'r') as f:
            content = f.read()
        
        repo.create_file(
            path=os.path.basename(file_path),
            message=commit_message,
            content=content
        )
    
    def create_issue_for_detection(self, repo_name, detection):
        """Create GitHub issue for interesting detection"""
        repo = self.g.get_repo(repo_name)
        
        issue_body = f"""
        **Species**: {detection['species']}
        **Camera**: {detection['camera_name']}
        **Confidence**: {detection['confidence']*100:.1f}%
        **Timestamp**: {detection['timestamp']}
        
        ![Detection Image]({detection['image_url']})
        
        [View in WildCAM](https://wildcam.app/detection/{detection['id']})
        """
        
        issue = repo.create_issue(
            title=f"Review: {detection['species']} detection",
            body=issue_body,
            labels=['detection', 'needs-review']
        )
        
        return issue
```

### Jupyter Hub

**Purpose**: Collaborative data analysis notebooks

**Integration**:

```python
# backend/integrations/jupyter.py
import requests

class JupyterHubIntegration:
    def __init__(self, hub_url, api_token):
        self.hub_url = hub_url
        self.headers = {'Authorization': f'token {api_token}'}
    
    def create_user_server(self, username):
        """Create Jupyter server for user"""
        response = requests.post(
            f"{self.hub_url}/hub/api/users/{username}/server",
            headers=self.headers
        )
        return response.json()
    
    def share_notebook(self, notebook_path, users):
        """Share notebook with team members"""
        # Implementation depends on JupyterHub setup
        pass
```

### Zotero (Citation Management)

**Purpose**: Manage research references and citations

**Integration**:

```python
# backend/integrations/zotero.py
from pyzotero import zotero

class ZoteroIntegration:
    def __init__(self, library_id, library_type, api_key):
        self.zot = zotero.Zotero(library_id, library_type, api_key)
    
    def add_detection_reference(self, detection, notes):
        """Add detection as reference item"""
        item = {
            'itemType': 'document',
            'title': f"{detection['species']} observation - {detection['timestamp']}",
            'abstractNote': notes,
            'url': f"https://wildcam.app/detection/{detection['id']}",
            'accessDate': datetime.utcnow().isoformat(),
            'tags': [
                {'tag': 'camera-trap'},
                {'tag': detection['species'].lower()},
                {'tag': 'automated-detection'}
            ]
        }
        
        created_item = self.zot.create_items([item])
        return created_item
```

## Video Conferencing Integrations

### Zoom

**Purpose**: Team meetings and video calls

**Integration**:

```python
# backend/integrations/zoom.py
import jwt
import requests
from datetime import datetime, timedelta

class ZoomIntegration:
    def __init__(self, api_key, api_secret):
        self.api_key = api_key
        self.api_secret = api_secret
        self.base_url = "https://api.zoom.us/v2"
    
    def generate_jwt_token(self):
        """Generate JWT token for Zoom API"""
        payload = {
            'iss': self.api_key,
            'exp': datetime.utcnow() + timedelta(hours=1)
        }
        return jwt.encode(payload, self.api_secret, algorithm='HS256')
    
    def create_meeting(self, topic, start_time, duration_minutes=60):
        """Create Zoom meeting for team discussion"""
        headers = {
            'Authorization': f'Bearer {self.generate_jwt_token()}',
            'Content-Type': 'application/json'
        }
        
        meeting_data = {
            'topic': topic,
            'type': 2,  # Scheduled meeting
            'start_time': start_time,
            'duration': duration_minutes,
            'timezone': 'UTC',
            'settings': {
                'join_before_host': True,
                'waiting_room': False,
                'recording': 'cloud'
            }
        }
        
        response = requests.post(
            f"{self.base_url}/users/me/meetings",
            json=meeting_data,
            headers=headers
        )
        
        return response.json()
```

## Integration Configuration

### Environment Variables

```env
# Academic Databases
GBIF_USERNAME=your_username
GBIF_PASSWORD=your_password
INATURALIST_TOKEN=your_token
EBIRD_API_KEY=your_key

# Cloud Storage
GOOGLE_DRIVE_CREDENTIALS=credentials.json
AWS_ACCESS_KEY_ID=your_key
AWS_SECRET_ACCESS_KEY=your_secret
AWS_S3_BUCKET=wildcam-storage

# Communication
SLACK_BOT_TOKEN=xoxb-your-token
SLACK_WEBHOOK_URL=https://hooks.slack.com/services/...
DISCORD_WEBHOOK_URL=https://discord.com/api/webhooks/...
TEAMS_WEBHOOK_URL=https://outlook.office.com/webhook/...

# Research Platforms
GITHUB_ACCESS_TOKEN=your_token
JUPYTERHUB_URL=https://jupyter.yourorg.edu
JUPYTERHUB_API_TOKEN=your_token
ZOTERO_API_KEY=your_key
ZOTERO_LIBRARY_ID=your_library_id

# Video Conferencing
ZOOM_API_KEY=your_key
ZOOM_API_SECRET=your_secret
```

### API Endpoint for Integration Management

```python
# backend/app.py
@app.route('/api/integrations', methods=['GET'])
@jwt_required()
def get_integrations():
    """Get list of configured integrations"""
    user_id = get_jwt_identity()
    
    integrations = {
        'gbif': {'enabled': bool(os.getenv('GBIF_USERNAME'))},
        'inaturalist': {'enabled': bool(os.getenv('INATURALIST_TOKEN'))},
        'slack': {'enabled': bool(os.getenv('SLACK_BOT_TOKEN'))},
        'github': {'enabled': bool(os.getenv('GITHUB_ACCESS_TOKEN'))},
        # ... more integrations
    }
    
    return jsonify({'integrations': integrations}), 200

@app.route('/api/integrations/<integration_name>/sync', methods=['POST'])
@jwt_required()
@require_permission(Permission.EXPORT_DATA)
def sync_integration(integration_name):
    """Trigger sync with external integration"""
    # Implementation for syncing data to external platform
    pass
```

## Best Practices

1. **Rate Limiting**: Respect API rate limits of external services
2. **Error Handling**: Implement robust error handling and retries
3. **Credentials Security**: Store API keys securely (environment variables, secrets manager)
4. **Data Privacy**: Ensure sensitive wildlife location data is protected
5. **User Consent**: Get user permission before sharing data externally
6. **Audit Logging**: Log all external data sharing for compliance
7. **Async Processing**: Use background tasks for slow integrations

## Testing Integrations

```python
# tests/test_integrations.py
import pytest
from backend.integrations.slack import SlackIntegration

def test_slack_notification():
    slack = SlackIntegration(os.getenv('SLACK_BOT_TOKEN'))
    
    test_detection = {
        'id': 123,
        'species': 'Test Species',
        'camera_name': 'Test Camera',
        'confidence': 0.95,
        'timestamp': '2025-01-15T10:30:00Z',
        'image_url': 'https://example.com/image.jpg'
    }
    
    response = slack.send_detection_alert('#test-channel', test_detection)
    assert response['ok'] == True
```

## Future Integration Opportunities

- **Movebank**: Animal movement data
- **NCBI GenBank**: Genetic data integration
- **Trello/Asana**: Project management
- **Tableau/PowerBI**: Advanced analytics
- **Zapier/IFTTT**: No-code automation
- **Weather APIs**: Environmental context
- **Satellite Imagery**: Habitat monitoring
