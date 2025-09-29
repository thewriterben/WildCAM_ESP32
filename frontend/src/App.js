import React, { useState, useEffect } from 'react';
import axios from 'axios';
import { MapContainer, TileLayer, Marker } from 'react-leaflet';
import { Chart } from 'react-chartjs-2';

// Main Dashboard Component
function WildlifeDashboard() {
  const [cameras, setCameras] = useState([]);
  const [captures, setCaptures] = useState([]);
  const [selectedCamera, setSelectedCamera] = useState(null);
  
  useEffect(() => {
    fetchCameras();
    fetchRecentCaptures();
  }, []);
  
  const fetchCameras = async () => {
    const response = await axios.get('/api/cameras');
    setCameras(response.data);
  };
  
  const fetchRecentCaptures = async () => {
    const response = await axios.get('/api/captures/recent');
    setCaptures(response.data);
  };
  
  return (
    <div className="dashboard">
      <CameraMap cameras={cameras} onCameraSelect={setSelectedCamera} />
      <CameraDetails camera={selectedCamera} />
      <RecentCaptures captures={captures} />
      <AnalyticsDashboard />
    </div>
  );
}

// Camera Map Component
function CameraMap({ cameras, onCameraSelect }) {
  return (
    <MapContainer center={[0, 0]} zoom={2}>
      <TileLayer url="https://{s}.tile.openstreetmap.org/{z}/{x}/{y}.png" />
      {cameras.map(camera => (
        <Marker 
          key={camera.id}
          position={[camera.latitude, camera.longitude]}
          eventHandlers={{
            click: () => onCameraSelect(camera)
          }}
        />
      ))}
    </MapContainer>
  );
}

// Gallery Component
function CaptureGallery({ captures }) {
  const [filter, setFilter] = useState('all');
  const [selectedImage, setSelectedImage] = useState(null);
  
  return (
    <div className="gallery">
      <FilterBar onFilterChange={setFilter} />
      <div className="image-grid">
        {captures
          .filter(capture => filterCapture(capture, filter))
          .map(capture => (
            <ImageCard 
              key={capture.id}
              capture={capture}
              onClick={() => setSelectedImage(capture)}
            />
          ))}
      </div>
      {selectedImage && (
        <ImageModal 
          capture={selectedImage}
          onClose={() => setSelectedImage(null)}
        />
      )}
    </div>
  );
}