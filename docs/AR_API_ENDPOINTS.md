# AR Mobile Interface - API Endpoints Specification

## Base URL

```
Production: https://api.wildlife-monitor.com/v1/
Staging: https://staging-api.wildlife-monitor.com/v1/
Development: http://localhost:8000/v1/
```

## Authentication

All endpoints require JWT authentication unless marked as public.

```http
Authorization: Bearer {jwt_token}
```

## AR Species Identification Endpoints

### Get 3D Model Metadata

```http
GET /ar/models/{speciesId}
```

**Parameters:**
- `speciesId` (path): Species identifier (e.g., "white_tailed_deer")

**Response:**
```json
{
  "modelId": "model_001",
  "speciesId": "white_tailed_deer",
  "speciesName": "White-tailed Deer",
  "scientificName": "Odocoileus virginianus",
  "modelPath": "https://cdn.wildlife-monitor.com/models/deer_lod0.glb",
  "thumbnailPath": "https://cdn.wildlife-monitor.com/thumbnails/deer.jpg",
  "fileSize": 25600000,
  "lodLevels": [
    {
      "level": 0,
      "modelPath": "https://cdn.wildlife-monitor.com/models/deer_lod0.glb",
      "vertexCount": 75000,
      "triangleCount": 50000,
      "maxDistance": 5.0,
      "fileSize": 25600000
    },
    {
      "level": 1,
      "modelPath": "https://cdn.wildlife-monitor.com/models/deer_lod1.glb",
      "vertexCount": 30000,
      "triangleCount": 20000,
      "maxDistance": 20.0,
      "fileSize": 10240000
    }
  ],
  "animations": [
    {
      "animationId": "walk_001",
      "name": "Walking",
      "behaviorType": "WALKING",
      "duration": 2.5,
      "loopable": true,
      "animationPath": "https://cdn.wildlife-monitor.com/animations/deer_walk.bin"
    }
  ],
  "conservationStatus": "LEAST_CONCERN"
}
```

### Download Model File

```http
GET /ar/models/{speciesId}/download?lod={level}
```

**Parameters:**
- `speciesId` (path): Species identifier
- `lod` (query): LOD level (0-3, default: 0)

**Response:**
- Binary GLTF/GLB file
- Content-Type: `model/gltf-binary`

### Get Field Guide Entry

```http
GET /ar/field-guide/{speciesId}
```

**Response:**
```json
{
  "speciesId": "white_tailed_deer",
  "commonName": "White-tailed Deer",
  "scientificName": "Odocoileus virginianus",
  "description": "Medium-sized deer native to North America...",
  "habitat": "Forests, grasslands, and agricultural areas",
  "diet": "Herbivore - grasses, leaves, twigs, fruits",
  "behavior": [
    "Most active at dawn and dusk",
    "Lives in small herds",
    "Males are territorial during mating season"
  ],
  "model3DUrl": "https://cdn.wildlife-monitor.com/models/deer_lod0.glb",
  "audioCallUrls": [
    "https://cdn.wildlife-monitor.com/audio/deer_grunt.mp3",
    "https://cdn.wildlife-monitor.com/audio/deer_alarm.mp3"
  ],
  "conservationStatus": "LEAST_CONCERN",
  "identificationTips": [
    "White underside of tail visible when running",
    "Reddish-brown coat in summer, grayish-brown in winter",
    "Bucks have antlers that shed annually"
  ],
  "size": {
    "averageLength": 1.8,
    "averageHeight": 1.0,
    "averageWeight": 68.0,
    "maxLength": 2.3,
    "maxHeight": 1.2,
    "maxWeight": 136.0
  }
}
```

### Submit AR Detection

```http
POST /ar/detections
```

**Request:**
```json
{
  "species": "white_tailed_deer",
  "confidence": 0.92,
  "location": {
    "latitude": 40.7128,
    "longitude": -74.0060
  },
  "timestamp": "2024-10-13T10:30:00Z",
  "estimatedSize": 1.85,
  "estimatedAge": "ADULT",
  "behaviorPattern": "FEEDING",
  "imageUrl": "https://cdn.wildlife-monitor.com/detections/img_001.jpg",
  "environmentalContext": {
    "temperature": 18.5,
    "timeOfDay": "MORNING",
    "season": "FALL",
    "weather": "CLEAR"
  }
}
```

**Response:**
```json
{
  "detectionId": "det_abc123",
  "status": "accepted",
  "message": "Detection recorded successfully",
  "cloudAnchorId": "anchor_xyz789"
}
```

## AR Navigation Endpoints

### Get Terrain Data

```http
GET /ar/terrain
```

**Parameters:**
- `lat` (query): Center latitude
- `lon` (query): Center longitude
- `radius` (query): Radius in meters (default: 500, max: 5000)
- `resolution` (query): Meters per grid cell (default: 10)

**Response:**
```json
{
  "centerLocation": {
    "latitude": 40.7128,
    "longitude": -74.0060
  },
  "bounds": {
    "northEast": {"latitude": 40.7178, "longitude": -74.0010},
    "southWest": {"latitude": 40.7078, "longitude": -74.0110}
  },
  "resolution": 10.0,
  "elevationData": [
    [125.5, 126.2, 127.1, ...],
    [124.8, 125.5, 126.0, ...],
    ...
  ],
  "vegetationMap": {
    "vegetationTypes": {
      "0": "DENSE_FOREST",
      "1": "OPEN_FOREST",
      "2": "GRASSLAND"
    },
    "coverageGrid": [
      [0, 0, 1, ...],
      [0, 1, 1, ...],
      ...
    ]
  },
  "terrainMeshUrl": "https://cdn.wildlife-monitor.com/terrain/mesh_001.glb"
}
```

### Get Waypoints

```http
GET /ar/waypoints
```

**Parameters:**
- `bounds` (query): Bounding box "lat1,lon1,lat2,lon2"
- `type` (query): Waypoint type filter (optional)

**Response:**
```json
{
  "waypoints": [
    {
      "id": "wp_001",
      "name": "Camera A-01",
      "location": {"latitude": 40.7128, "longitude": -74.0060},
      "elevation": 125.5,
      "type": "CAMERA_LOCATION",
      "description": "Main trail camera",
      "cloudAnchorId": "anchor_001",
      "createdBy": "user_123",
      "createdDate": "2024-10-01T08:00:00Z",
      "visited": false
    }
  ]
}
```

### Add Waypoint

```http
POST /ar/waypoints
```

**Request:**
```json
{
  "name": "Water Source A",
  "location": {"latitude": 40.7128, "longitude": -74.0060},
  "elevation": 125.5,
  "type": "WATER_SOURCE",
  "description": "Small creek, flows year-round",
  "cloudAnchorId": "anchor_002"
}
```

**Response:**
```json
{
  "waypointId": "wp_002",
  "status": "created"
}
```

### Get Historical Sighting Heatmap

```http
GET /ar/sightings/heatmap
```

**Parameters:**
- `speciesId` (query): Species filter (optional, null for all)
- `bounds` (query): Bounding box "lat1,lon1,lat2,lon2"
- `startDate` (query): ISO 8601 date
- `endDate` (query): ISO 8601 date

**Response:**
```json
{
  "speciesId": "white_tailed_deer",
  "timeRange": {
    "start": "2024-01-01T00:00:00Z",
    "end": "2024-10-13T00:00:00Z"
  },
  "heatmapData": [
    {
      "location": {"latitude": 40.7128, "longitude": -74.0060},
      "intensity": 0.85,
      "count": 47,
      "lastSighting": "2024-10-12T14:30:00Z"
    }
  ],
  "intensityMax": 1.0,
  "overlayTextureUrl": "https://cdn.wildlife-monitor.com/heatmaps/heatmap_001.png"
}
```

### Get Weather Data

```http
GET /ar/weather
```

**Parameters:**
- `lat` (query): Latitude
- `lon` (query): Longitude

**Response:**
```json
{
  "location": {"latitude": 40.7128, "longitude": -74.0060},
  "temperature": 18.5,
  "humidity": 65.0,
  "windSpeed": 3.5,
  "windDirection": 180.0,
  "precipitation": 0.0,
  "condition": "CLEAR",
  "forecast": [
    {
      "time": "2024-10-13T13:00:00Z",
      "temperature": 20.0,
      "condition": "PARTLY_CLOUDY",
      "precipitationProbability": 0.15
    }
  ],
  "timestamp": "2024-10-13T10:00:00Z"
}
```

## Camera Placement Endpoints

### Analyze Camera Placement

```http
POST /ar/placement/analyze
```

**Request:**
```json
{
  "location": {"latitude": 40.7128, "longitude": -74.0060},
  "position": {"x": 0, "y": 1.5, "z": 0},
  "orientation": {"yaw": 180, "pitch": 0, "roll": 0},
  "cameraModel": "ESP32-CAM-OV2640",
  "targetSpecies": ["white_tailed_deer", "black_bear"]
}
```

**Response:**
```json
{
  "score": 0.87,
  "fovVisualization": {
    "fovAngleHorizontal": 62.2,
    "fovAngleVertical": 48.8,
    "detectionRange": 10.0,
    "coverageArea": 85.3
  },
  "recommendations": [
    "Good placement for target species",
    "Consider rotating 15° clockwise for better trail coverage",
    "Adequate solar exposure for panels"
  ],
  "hazards": [
    {
      "type": "HIGH_WIND_AREA",
      "severity": "MODERATE",
      "description": "Area exposed to prevailing winds",
      "mitigation": "Use reinforced mounting hardware"
    }
  ],
  "networkConnectivity": {
    "signalStrength": -65.0,
    "networkType": "WIFI",
    "reliability": 0.92
  },
  "solarGuidance": {
    "optimalAzimuth": 180.0,
    "optimalTilt": 40.0,
    "expectedDailyEnergy": 18.5
  }
}
```

### Get Placement Recommendations

```http
POST /ar/placement/recommendations
```

**Request:**
```json
{
  "area": {
    "northEast": {"latitude": 40.7178, "longitude": -74.0010},
    "southWest": {"latitude": 40.7078, "longitude": -74.0110}
  },
  "targetSpecies": ["white_tailed_deer"],
  "existingCameras": [
    {
      "cameraId": "cam_001",
      "location": {"latitude": 40.7128, "longitude": -74.0060}
    }
  ],
  "maxRecommendations": 5
}
```

**Response:**
```json
{
  "recommendations": [
    {
      "recommendationId": "rec_001",
      "location": {"latitude": 40.7140, "longitude": -74.0075},
      "position": {"x": 0, "y": 1.5, "z": 0},
      "orientation": {"yaw": 165, "pitch": 0, "roll": 0},
      "score": 0.92,
      "reason": "High activity area based on historical data, excellent coverage",
      "heightAboveGround": 1.5,
      "expectedCoverage": 0.85
    }
  ],
  "coverageAnalysis": {
    "totalCoverage": 145.7,
    "overlapAreas": [],
    "blindSpots": [
      {
        "spotId": "blind_001",
        "area": 25.3,
        "priority": "MEDIUM"
      }
    ],
    "coverageEfficiency": 0.78
  }
}
```

## Collaborative AR Endpoints

### Create AR Session

```http
POST /ar/sessions
```

**Request:**
```json
{
  "name": "Field Survey - October 2024",
  "hostUserId": "user_123",
  "participants": [
    {"userId": "user_456", "role": "RESEARCHER"}
  ]
}
```

**Response:**
```json
{
  "sessionId": "session_abc123",
  "cloudAnchorIds": [],
  "webSocketUrl": "wss://api.wildlife-monitor.com/ar/sessions/session_abc123/ws"
}
```

### Join AR Session

```http
POST /ar/sessions/{sessionId}/join
```

**Request:**
```json
{
  "userId": "user_789",
  "userName": "Jane Researcher",
  "role": "OBSERVER"
}
```

**Response:**
```json
{
  "status": "joined",
  "participants": [
    {"userId": "user_123", "userName": "John Host", "role": "HOST"},
    {"userId": "user_456", "userName": "Mike Researcher", "role": "RESEARCHER"},
    {"userId": "user_789", "userName": "Jane Researcher", "role": "OBSERVER"}
  ],
  "sharedAnchors": [],
  "sharedAnnotations": []
}
```

### Add Annotation

```http
POST /ar/sessions/{sessionId}/annotations
```

**Request:**
```json
{
  "createdBy": "user_123",
  "position": {"x": 1.5, "y": 0.5, "z": 3.2},
  "cloudAnchorId": "anchor_001",
  "type": "TEXT_NOTE",
  "content": {
    "text": "Deer trail observed here",
    "color": 0xFF00FF00
  },
  "visibility": "TEAM_ONLY"
}
```

**Response:**
```json
{
  "annotationId": "anno_001",
  "status": "created",
  "syncedToParticipants": 3
}
```

### Add Bookmark

```http
POST /ar/sessions/{sessionId}/bookmarks
```

**Request:**
```json
{
  "name": "Interesting Tree Hollow",
  "description": "Potential den site",
  "location": {"latitude": 40.7128, "longitude": -74.0060},
  "cloudAnchorId": "anchor_002",
  "category": "IMPORTANT_FINDING",
  "tags": ["den", "habitat"]
}
```

**Response:**
```json
{
  "bookmarkId": "bookmark_001",
  "status": "created"
}
```

## Education Endpoints

### Get Tutorials

```http
GET /ar/tutorials
```

**Parameters:**
- `category` (query): Filter by category (optional)
- `difficulty` (query): Filter by difficulty (optional)

**Response:**
```json
{
  "tutorials": [
    {
      "tutorialId": "tut_001",
      "title": "Camera Placement Basics",
      "description": "Learn optimal camera placement techniques",
      "category": "CAMERA_PLACEMENT",
      "difficulty": "BEGINNER",
      "estimatedDuration": 15,
      "lessonsCount": 5,
      "certificateAwarded": false
    }
  ]
}
```

### Get Tutorial Details

```http
GET /ar/tutorials/{tutorialId}
```

**Response:**
```json
{
  "tutorialId": "tut_001",
  "title": "Camera Placement Basics",
  "lessons": [
    {
      "lessonNumber": 1,
      "title": "Understanding Field of View",
      "objectives": [
        "Understand camera FOV",
        "Learn to visualize coverage area"
      ],
      "content": {
        "text": "Field of view (FOV) is...",
        "images": ["https://cdn.wildlife-monitor.com/tutorials/fov_diagram.jpg"],
        "videos": ["https://cdn.wildlife-monitor.com/tutorials/fov_video.mp4"]
      },
      "quiz": {
        "quizId": "quiz_001",
        "questions": [
          {
            "questionId": "q_001",
            "question": "What is the FOV of ESP32-CAM OV2640?",
            "questionType": "MULTIPLE_CHOICE",
            "options": ["45°", "62.2°", "90°", "120°"],
            "correctAnswer": "62.2°"
          }
        ],
        "passingScore": 0.7
      }
    }
  ]
}
```

### Submit Quiz Answer

```http
POST /ar/tutorials/{tutorialId}/quizzes/{quizId}/submit
```

**Request:**
```json
{
  "userId": "user_123",
  "answers": {
    "q_001": "62.2°",
    "q_002": "WATER_SOURCE"
  }
}
```

**Response:**
```json
{
  "score": 0.85,
  "passed": true,
  "feedback": {
    "q_001": {"correct": true, "explanation": "Correct!"},
    "q_002": {"correct": false, "explanation": "The correct answer is CAMERA_LOCATION"}
  }
}
```

### Get Challenges

```http
GET /ar/challenges
```

**Parameters:**
- `difficulty` (query): Filter by difficulty (optional)
- `active` (query): Show only active challenges (boolean)

**Response:**
```json
{
  "challenges": [
    {
      "challengeId": "challenge_001",
      "name": "Species Spotter",
      "description": "Identify 10 different species in AR mode",
      "targetSpecies": ["white_tailed_deer", "black_bear", "raccoon"],
      "difficulty": "MEDIUM",
      "timeLimit": 3600,
      "pointsReward": 500,
      "badgeReward": {
        "badgeId": "badge_001",
        "name": "Species Expert",
        "rarity": "RARE"
      },
      "leaderboard": true
    }
  ]
}
```

### Submit Challenge Completion

```http
POST /ar/challenges/{challengeId}/complete
```

**Request:**
```json
{
  "userId": "user_123",
  "score": 0.92,
  "timeSpent": 2450,
  "evidenceUrls": [
    "https://cdn.wildlife-monitor.com/challenges/proof_001.jpg"
  ]
}
```

**Response:**
```json
{
  "status": "completed",
  "pointsAwarded": 460,
  "badgeAwarded": {
    "badgeId": "badge_001",
    "name": "Species Expert"
  },
  "leaderboardRank": 15
}
```

### Get User Progress

```http
GET /ar/users/{userId}/progress
```

**Response:**
```json
{
  "userId": "user_123",
  "totalPoints": 2500,
  "level": 5,
  "badges": [
    {"badgeId": "badge_001", "name": "Species Expert", "earnedDate": "2024-10-01T12:00:00Z"}
  ],
  "completedTutorials": ["tut_001", "tut_002"],
  "certifications": [
    {
      "certificationId": "cert_001",
      "name": "Field Researcher Basic",
      "level": "BASIC",
      "issued": "2024-09-15T10:00:00Z"
    }
  ],
  "statistics": {
    "speciesIdentified": 25,
    "camerasPlaced": 8,
    "fieldHours": 45.5,
    "dataSubmitted": 120
  }
}
```

## AI Field Assistant Endpoints

### Analyze Behavior

```http
POST /ar/ai/behavior/analyze
```

**Request:**
```json
{
  "species": "white_tailed_deer",
  "currentBehavior": "ALERT",
  "recentHistory": ["FEEDING", "WALKING", "ALERT"],
  "environmentalContext": {
    "timeOfDay": "DUSK",
    "weather": "CLEAR",
    "temperature": 15.5
  }
}
```

**Response:**
```json
{
  "behavior": "ALERT",
  "confidence": 0.88,
  "nextLikelyBehavior": "FLEEING",
  "timeToNextBehavior": 45,
  "activityLevel": "HIGH_ACTIVITY",
  "captureRecommendation": {
    "shouldCapture": true,
    "reason": "Animal alert behavior, may flee soon",
    "captureType": "BURST_PHOTO",
    "confidenceScore": 0.85
  }
}
```

### Generate Field Note

```http
POST /ar/ai/fieldnotes/generate
```

**Request:**
```json
{
  "audioTranscript": "Observed white-tailed deer at water source, appears to be adult male with large antlers. Behavior calm, feeding on vegetation near creek.",
  "location": {"latitude": 40.7128, "longitude": -74.0060},
  "weather": "CLEAR",
  "timestamp": "2024-10-13T14:30:00Z"
}
```

**Response:**
```json
{
  "noteId": "note_001",
  "timestamp": "2024-10-13T14:30:00Z",
  "observations": [
    {
      "type": "SPECIES_IDENTIFICATION",
      "description": "White-tailed deer, adult male",
      "confidence": 0.92
    },
    {
      "type": "BEHAVIOR",
      "description": "Calm feeding behavior near water source",
      "confidence": 0.88
    }
  ],
  "detections": ["white_tailed_deer"],
  "tags": ["deer", "water_source", "feeding", "male"],
  "confidence": 0.90
}
```

## Rate Limiting

- Authenticated: 1000 requests/hour
- Model downloads: 100 requests/hour
- Real-time endpoints (WebSocket): No limit
- Burst: 20 requests/second

## Error Responses

```json
{
  "error": {
    "code": "INVALID_REQUEST",
    "message": "Species ID not found",
    "details": {
      "speciesId": "unknown_species"
    }
  }
}
```

**Error Codes:**
- `INVALID_REQUEST`: Malformed request
- `UNAUTHORIZED`: Invalid or missing token
- `FORBIDDEN`: Insufficient permissions
- `NOT_FOUND`: Resource not found
- `RATE_LIMIT_EXCEEDED`: Too many requests
- `SERVER_ERROR`: Internal server error

## WebSocket Real-Time Collaboration

### Connection

```javascript
const ws = new WebSocket('wss://api.wildlife-monitor.com/ar/sessions/session_abc123/ws');
ws.send(JSON.stringify({
  type: 'authenticate',
  token: 'jwt_token'
}));
```

### Messages

**Annotation Created:**
```json
{
  "type": "annotation_created",
  "annotation": {
    "annotationId": "anno_001",
    "createdBy": "user_123",
    "position": {"x": 1.5, "y": 0.5, "z": 3.2}
  }
}
```

**Participant Joined:**
```json
{
  "type": "participant_joined",
  "participant": {
    "userId": "user_456",
    "userName": "Jane Doe",
    "role": "RESEARCHER"
  }
}
```

**Position Update:**
```json
{
  "type": "position_update",
  "userId": "user_123",
  "location": {"latitude": 40.7128, "longitude": -74.0060},
  "heading": 180.0
}
```
