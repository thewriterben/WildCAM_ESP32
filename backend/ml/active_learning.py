#!/usr/bin/env python3
"""
Active Learning System for Wildlife Classification

Implements uncertainty sampling and expert validation for continuous
improvement of wildlife classification models through active learning.
"""

import os
import json
import logging
import numpy as np
from pathlib import Path
from typing import Dict, List, Tuple, Optional, Callable
from datetime import datetime
import sqlite3
from dataclasses import dataclass

import tensorflow as tf
from sklearn.metrics import entropy
from PIL import Image

# Configure logging
logging.basicConfig(level=logging.INFO)
logger = logging.getLogger(__name__)

@dataclass
class UncertainSample:
    """Structure for uncertain samples requiring expert validation"""
    sample_id: str
    image_path: str
    predicted_class: int
    confidence_scores: List[float]
    uncertainty_score: float
    acquisition_method: str
    timestamp: datetime
    expert_validated: bool = False
    true_label: Optional[int] = None
    expert_confidence: Optional[float] = None

@dataclass
class ExpertFeedback:
    """Structure for expert validation feedback"""
    sample_id: str
    true_label: int
    confidence: float
    feedback_notes: str
    expert_id: str
    validation_timestamp: datetime

class ActiveLearningManager:
    """
    Manages active learning process for wildlife classification including
    uncertainty sampling, expert validation, and model improvement.
    """
    
    def __init__(self, model_path: str, database_path: str, output_dir: str):
        """
        Initialize active learning manager
        
        Args:
            model_path: Path to current TensorFlow Lite model
            database_path: Path to SQLite database for sample storage
            output_dir: Directory for output files and logs
        """
        self.model_path = Path(model_path)
        self.database_path = Path(database_path)
        self.output_dir = Path(output_dir)
        self.output_dir.mkdir(parents=True, exist_ok=True)
        
        # Load model
        self.interpreter = None
        self.input_details = None
        self.output_details = None
        self._load_model()
        
        # Initialize database
        self._init_database()
        
        # Configuration
        self.config = {
            "uncertainty_threshold": 0.3,
            "max_samples_per_batch": 100,
            "min_confidence_gap": 0.1,
            "entropy_threshold": 1.0,
            "acquisition_methods": ["entropy", "confidence_gap", "margin_sampling"],
            "expert_validation_timeout": 7 * 24 * 3600,  # 7 days in seconds
            "batch_size": 32,
            "image_size": [224, 224]
        }
        
        # Statistics
        self.stats = {
            "total_samples_processed": 0,
            "uncertain_samples_found": 0,
            "expert_validations_received": 0,
            "model_improvements": 0,
            "accuracy_gains": []
        }
        
        logger.info(f"Active learning manager initialized")
        logger.info(f"Model: {self.model_path}")
        logger.info(f"Database: {self.database_path}")
    
    def _load_model(self):
        """Load TensorFlow Lite model"""
        try:
            self.interpreter = tf.lite.Interpreter(model_path=str(self.model_path))
            self.interpreter.allocate_tensors()
            
            self.input_details = self.interpreter.get_input_details()
            self.output_details = self.interpreter.get_output_details()
            
            logger.info(f"Model loaded successfully")
            logger.info(f"Input shape: {self.input_details[0]['shape']}")
            logger.info(f"Output shape: {self.output_details[0]['shape']}")
            
        except Exception as e:
            logger.error(f"Failed to load model: {e}")
            raise
    
    def _init_database(self):
        """Initialize SQLite database for sample storage"""
        try:
            conn = sqlite3.connect(self.database_path)
            cursor = conn.cursor()
            
            # Create tables
            cursor.execute('''
                CREATE TABLE IF NOT EXISTS uncertain_samples (
                    sample_id TEXT PRIMARY KEY,
                    image_path TEXT NOT NULL,
                    predicted_class INTEGER,
                    confidence_scores TEXT,
                    uncertainty_score REAL,
                    acquisition_method TEXT,
                    timestamp TEXT,
                    expert_validated INTEGER DEFAULT 0,
                    true_label INTEGER,
                    expert_confidence REAL
                )
            ''')
            
            cursor.execute('''
                CREATE TABLE IF NOT EXISTS expert_feedback (
                    id INTEGER PRIMARY KEY AUTOINCREMENT,
                    sample_id TEXT,
                    true_label INTEGER,
                    confidence REAL,
                    feedback_notes TEXT,
                    expert_id TEXT,
                    validation_timestamp TEXT,
                    FOREIGN KEY (sample_id) REFERENCES uncertain_samples (sample_id)
                )
            ''')
            
            cursor.execute('''
                CREATE TABLE IF NOT EXISTS active_learning_stats (
                    timestamp TEXT PRIMARY KEY,
                    total_samples INTEGER,
                    uncertain_samples INTEGER,
                    validated_samples INTEGER,
                    accuracy_improvement REAL
                )
            ''')
            
            conn.commit()
            conn.close()
            
            logger.info("Database initialized successfully")
            
        except Exception as e:
            logger.error(f"Failed to initialize database: {e}")
            raise
    
    def process_batch(self, image_paths: List[str], batch_id: str = None) -> List[UncertainSample]:
        """
        Process a batch of images and identify uncertain samples
        
        Args:
            image_paths: List of paths to images to process
            batch_id: Optional batch identifier
            
        Returns:
            List of uncertain samples requiring expert validation
        """
        if batch_id is None:
            batch_id = datetime.now().strftime("%Y%m%d_%H%M%S")
        
        logger.info(f"Processing batch {batch_id} with {len(image_paths)} images")
        
        uncertain_samples = []
        
        for i, image_path in enumerate(image_paths):
            try:
                # Run inference
                predictions = self._run_inference(image_path)
                if predictions is None:
                    continue
                
                # Calculate uncertainty metrics
                uncertainty_metrics = self._calculate_uncertainty(predictions)
                
                # Check if sample is uncertain
                if self._is_uncertain(uncertainty_metrics):
                    sample = UncertainSample(
                        sample_id=f"{batch_id}_{i:04d}",
                        image_path=image_path,
                        predicted_class=int(np.argmax(predictions)),
                        confidence_scores=predictions.tolist(),
                        uncertainty_score=uncertainty_metrics["entropy"],
                        acquisition_method=uncertainty_metrics["method"],
                        timestamp=datetime.now()
                    )
                    
                    uncertain_samples.append(sample)
                    self._store_uncertain_sample(sample)
                
                self.stats["total_samples_processed"] += 1
                
                if (i + 1) % 100 == 0:
                    logger.info(f"Processed {i + 1}/{len(image_paths)} images")
                    
            except Exception as e:
                logger.error(f"Error processing image {image_path}: {e}")
                continue
        
        self.stats["uncertain_samples_found"] += len(uncertain_samples)
        
        logger.info(f"Batch processing completed: {len(uncertain_samples)} uncertain samples found")
        
        return uncertain_samples
    
    def _run_inference(self, image_path: str) -> Optional[np.ndarray]:
        """Run inference on a single image"""
        try:
            # Load and preprocess image
            image = Image.open(image_path).convert('RGB')
            image = image.resize(self.config["image_size"])
            
            # Convert to numpy array and normalize
            image_array = np.array(image).astype(np.float32)
            
            if self.input_details[0]['dtype'] == np.uint8:
                image_array = image_array.astype(np.uint8)
            else:
                image_array = image_array / 255.0
            
            # Add batch dimension
            input_data = np.expand_dims(image_array, axis=0)
            
            # Run inference
            self.interpreter.set_tensor(self.input_details[0]['index'], input_data)
            self.interpreter.invoke()
            
            # Get predictions
            predictions = self.interpreter.get_tensor(self.output_details[0]['index'])[0]
            
            # Apply softmax if needed
            if np.max(predictions) > 1.0:
                predictions = self._softmax(predictions)
            
            return predictions
            
        except Exception as e:
            logger.error(f"Inference failed for {image_path}: {e}")
            return None
    
    def _softmax(self, x: np.ndarray) -> np.ndarray:
        """Apply softmax function"""
        exp_x = np.exp(x - np.max(x))
        return exp_x / np.sum(exp_x)
    
    def _calculate_uncertainty(self, predictions: np.ndarray) -> Dict:
        """Calculate various uncertainty metrics"""
        # Sort predictions in descending order
        sorted_preds = np.sort(predictions)[::-1]
        
        # Entropy-based uncertainty
        entropy_score = entropy(predictions)
        
        # Confidence gap (difference between top 2 predictions)
        confidence_gap = sorted_preds[0] - sorted_preds[1] if len(sorted_preds) > 1 else 1.0
        
        # Margin sampling (uncertainty based on margin between classes)
        margin = 1 - sorted_preds[0]
        
        # Determine acquisition method
        method = "entropy"
        if entropy_score > self.config["entropy_threshold"]:
            method = "entropy"
        elif confidence_gap < self.config["min_confidence_gap"]:
            method = "confidence_gap"
        else:
            method = "margin_sampling"
        
        return {
            "entropy": float(entropy_score),
            "confidence_gap": float(confidence_gap),
            "margin": float(margin),
            "max_confidence": float(sorted_preds[0]),
            "method": method
        }
    
    def _is_uncertain(self, uncertainty_metrics: Dict) -> bool:
        """Determine if a sample is uncertain and needs expert validation"""
        entropy_uncertain = uncertainty_metrics["entropy"] > self.config["uncertainty_threshold"]
        confidence_uncertain = uncertainty_metrics["confidence_gap"] < self.config["min_confidence_gap"]
        margin_uncertain = uncertainty_metrics["margin"] > self.config["uncertainty_threshold"]
        
        return entropy_uncertain or confidence_uncertain or margin_uncertain
    
    def _store_uncertain_sample(self, sample: UncertainSample):
        """Store uncertain sample in database"""
        try:
            conn = sqlite3.connect(self.database_path)
            cursor = conn.cursor()
            
            cursor.execute('''
                INSERT OR REPLACE INTO uncertain_samples 
                (sample_id, image_path, predicted_class, confidence_scores, 
                 uncertainty_score, acquisition_method, timestamp)
                VALUES (?, ?, ?, ?, ?, ?, ?)
            ''', (
                sample.sample_id,
                sample.image_path,
                sample.predicted_class,
                json.dumps(sample.confidence_scores),
                sample.uncertainty_score,
                sample.acquisition_method,
                sample.timestamp.isoformat()
            ))
            
            conn.commit()
            conn.close()
            
        except Exception as e:
            logger.error(f"Failed to store uncertain sample: {e}")
    
    def get_samples_for_validation(self, limit: int = 50, 
                                  method: str = None) -> List[UncertainSample]:
        """
        Get uncertain samples that need expert validation
        
        Args:
            limit: Maximum number of samples to return
            method: Filter by acquisition method
            
        Returns:
            List of uncertain samples
        """
        try:
            conn = sqlite3.connect(self.database_path)
            cursor = conn.cursor()
            
            query = '''
                SELECT sample_id, image_path, predicted_class, confidence_scores,
                       uncertainty_score, acquisition_method, timestamp
                FROM uncertain_samples 
                WHERE expert_validated = 0
            '''
            params = []
            
            if method:
                query += ' AND acquisition_method = ?'
                params.append(method)
            
            query += ' ORDER BY uncertainty_score DESC LIMIT ?'
            params.append(limit)
            
            cursor.execute(query, params)
            rows = cursor.fetchall()
            
            samples = []
            for row in rows:
                sample = UncertainSample(
                    sample_id=row[0],
                    image_path=row[1],
                    predicted_class=row[2],
                    confidence_scores=json.loads(row[3]),
                    uncertainty_score=row[4],
                    acquisition_method=row[5],
                    timestamp=datetime.fromisoformat(row[6])
                )
                samples.append(sample)
            
            conn.close()
            
            logger.info(f"Retrieved {len(samples)} samples for validation")
            return samples
            
        except Exception as e:
            logger.error(f"Failed to get samples for validation: {e}")
            return []
    
    def submit_expert_feedback(self, feedback: ExpertFeedback):
        """
        Submit expert validation feedback
        
        Args:
            feedback: Expert feedback for a sample
        """
        try:
            conn = sqlite3.connect(self.database_path)
            cursor = conn.cursor()
            
            # Store expert feedback
            cursor.execute('''
                INSERT INTO expert_feedback 
                (sample_id, true_label, confidence, feedback_notes, expert_id, validation_timestamp)
                VALUES (?, ?, ?, ?, ?, ?)
            ''', (
                feedback.sample_id,
                feedback.true_label,
                feedback.confidence,
                feedback.feedback_notes,
                feedback.expert_id,
                feedback.validation_timestamp.isoformat()
            ))
            
            # Update uncertain sample
            cursor.execute('''
                UPDATE uncertain_samples 
                SET expert_validated = 1, true_label = ?, expert_confidence = ?
                WHERE sample_id = ?
            ''', (feedback.true_label, feedback.confidence, feedback.sample_id))
            
            conn.commit()
            conn.close()
            
            self.stats["expert_validations_received"] += 1
            
            logger.info(f"Expert feedback submitted for sample {feedback.sample_id}")
            
        except Exception as e:
            logger.error(f"Failed to submit expert feedback: {e}")
    
    def get_validated_samples(self, min_confidence: float = 0.8) -> List[Tuple[str, int, float]]:
        """
        Get validated samples for model retraining
        
        Args:
            min_confidence: Minimum expert confidence threshold
            
        Returns:
            List of (image_path, true_label, confidence) tuples
        """
        try:
            conn = sqlite3.connect(self.database_path)
            cursor = conn.cursor()
            
            cursor.execute('''
                SELECT image_path, true_label, expert_confidence
                FROM uncertain_samples 
                WHERE expert_validated = 1 AND expert_confidence >= ?
            ''', (min_confidence,))
            
            rows = cursor.fetchall()
            conn.close()
            
            logger.info(f"Retrieved {len(rows)} validated samples for retraining")
            return rows
            
        except Exception as e:
            logger.error(f"Failed to get validated samples: {e}")
            return []
    
    def generate_training_data(self, output_path: str, min_samples_per_class: int = 10):
        """
        Generate training data from validated samples
        
        Args:
            output_path: Path to save training data
            min_samples_per_class: Minimum samples required per class
        """
        try:
            validated_samples = self.get_validated_samples()
            
            # Group by class
            class_samples = {}
            for image_path, true_label, confidence in validated_samples:
                if true_label not in class_samples:
                    class_samples[true_label] = []
                class_samples[true_label].append((image_path, confidence))
            
            # Filter classes with enough samples
            training_data = []
            for class_id, samples in class_samples.items():
                if len(samples) >= min_samples_per_class:
                    training_data.extend([(path, class_id, conf) for path, conf in samples])
            
            # Save training data
            output_path = Path(output_path)
            with open(output_path, 'w') as f:
                json.dump({
                    "training_samples": [
                        {
                            "image_path": path,
                            "label": int(label),
                            "confidence": float(conf)
                        }
                        for path, label, conf in training_data
                    ],
                    "generation_date": datetime.now().isoformat(),
                    "total_samples": len(training_data),
                    "classes": list(set([label for _, label, _ in training_data]))
                }, f, indent=2)
            
            logger.info(f"Training data generated: {len(training_data)} samples")
            logger.info(f"Saved to: {output_path}")
            
            return str(output_path)
            
        except Exception as e:
            logger.error(f"Failed to generate training data: {e}")
            return None
    
    def calculate_model_improvement(self, validation_accuracy: float) -> float:
        """
        Calculate model improvement from active learning
        
        Args:
            validation_accuracy: Current model accuracy on validation set
            
        Returns:
            Accuracy improvement
        """
        if not self.stats["accuracy_gains"]:
            baseline_accuracy = validation_accuracy
        else:
            baseline_accuracy = self.stats["accuracy_gains"][0]
        
        improvement = validation_accuracy - baseline_accuracy
        self.stats["accuracy_gains"].append(validation_accuracy)
        
        if improvement > 0:
            self.stats["model_improvements"] += 1
        
        logger.info(f"Model improvement: {improvement:.4f} (accuracy: {validation_accuracy:.4f})")
        
        return improvement
    
    def get_statistics(self) -> Dict:
        """Get active learning statistics"""
        return {
            **self.stats,
            "validation_rate": (
                self.stats["expert_validations_received"] / 
                max(1, self.stats["uncertain_samples_found"])
            ),
            "improvement_rate": (
                self.stats["model_improvements"] / 
                max(1, len(self.stats["accuracy_gains"]))
            )
        }
    
    def export_validation_interface(self, output_dir: str, samples_limit: int = 100):
        """
        Export samples for expert validation interface
        
        Args:
            output_dir: Directory to export validation files
            samples_limit: Maximum number of samples to export
        """
        output_path = Path(output_dir)
        output_path.mkdir(parents=True, exist_ok=True)
        
        # Get samples for validation
        samples = self.get_samples_for_validation(limit=samples_limit)
        
        # Create validation data file
        validation_data = {
            "samples": [
                {
                    "sample_id": sample.sample_id,
                    "image_path": sample.image_path,
                    "predicted_class": sample.predicted_class,
                    "confidence_scores": sample.confidence_scores,
                    "uncertainty_score": sample.uncertainty_score,
                    "acquisition_method": sample.acquisition_method,
                    "timestamp": sample.timestamp.isoformat()
                }
                for sample in samples
            ],
            "export_date": datetime.now().isoformat(),
            "total_samples": len(samples)
        }
        
        with open(output_path / "validation_samples.json", 'w') as f:
            json.dump(validation_data, f, indent=2)
        
        # Create validation form template
        html_template = self._create_validation_html_template()
        with open(output_path / "validation_interface.html", 'w') as f:
            f.write(html_template)
        
        logger.info(f"Validation interface exported to: {output_path}")
        logger.info(f"Samples exported: {len(samples)}")
    
    def _create_validation_html_template(self) -> str:
        """Create HTML template for expert validation"""
        return """
<!DOCTYPE html>
<html>
<head>
    <title>Wildlife Classification Expert Validation</title>
    <style>
        body { font-family: Arial, sans-serif; margin: 20px; }
        .sample { border: 1px solid #ccc; margin: 10px 0; padding: 15px; }
        .image { max-width: 300px; height: auto; }
        .predictions { margin: 10px 0; }
        .form-group { margin: 10px 0; }
        button { background: #007cba; color: white; padding: 8px 16px; border: none; cursor: pointer; }
        button:hover { background: #005a82; }
    </style>
</head>
<body>
    <h1>Wildlife Classification Expert Validation</h1>
    <p>Please review the following uncertain predictions and provide correct labels.</p>
    
    <form id="validationForm">
        <div id="samples"></div>
        <button type="submit">Submit All Validations</button>
    </form>
    
    <script>
        // Load validation samples and create interface
        fetch('validation_samples.json')
            .then(response => response.json())
            .then(data => {
                const samplesDiv = document.getElementById('samples');
                data.samples.forEach(sample => {
                    const sampleDiv = document.createElement('div');
                    sampleDiv.className = 'sample';
                    sampleDiv.innerHTML = `
                        <h3>Sample ID: ${sample.sample_id}</h3>
                        <img src="${sample.image_path}" class="image" alt="Wildlife image">
                        <div class="predictions">
                            <strong>Predicted Class:</strong> ${sample.predicted_class}<br>
                            <strong>Uncertainty Score:</strong> ${sample.uncertainty_score.toFixed(3)}<br>
                            <strong>Method:</strong> ${sample.acquisition_method}
                        </div>
                        <div class="form-group">
                            <label>Correct Label:</label>
                            <input type="number" name="true_label_${sample.sample_id}" min="0" max="49" required>
                        </div>
                        <div class="form-group">
                            <label>Confidence (0-1):</label>
                            <input type="number" name="confidence_${sample.sample_id}" min="0" max="1" step="0.1" value="1.0" required>
                        </div>
                        <div class="form-group">
                            <label>Notes:</label>
                            <textarea name="notes_${sample.sample_id}" rows="2" cols="50"></textarea>
                        </div>
                    `;
                    samplesDiv.appendChild(sampleDiv);
                });
            });
        
        // Handle form submission
        document.getElementById('validationForm').addEventListener('submit', function(e) {
            e.preventDefault();
            alert('Validation submitted! In a real implementation, this would send data to the server.');
        });
    </script>
</body>
</html>
        """


def main():
    """Example usage of active learning system"""
    # Initialize active learning manager
    active_learning = ActiveLearningManager(
        model_path="/models/wildlife_classifier_quantized.tflite",
        database_path="/data/active_learning.db",
        output_dir="/data/active_learning"
    )
    
    # Process batch of images
    image_paths = ["/data/images/batch1/img_{:04d}.jpg".format(i) for i in range(100)]
    uncertain_samples = active_learning.process_batch(image_paths, "batch_001")
    
    # Export validation interface
    active_learning.export_validation_interface("/data/validation_interface")
    
    # Get statistics
    stats = active_learning.get_statistics()
    print("Active Learning Statistics:")
    print(json.dumps(stats, indent=2))


if __name__ == "__main__":
    main()