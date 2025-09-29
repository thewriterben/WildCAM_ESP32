import tensorflow as tf
from tensorflow.keras import layers, models
import numpy as np
from sklearn.model_selection import train_test_split
import cv2
import os

class WildlifeModelTrainer:
    def __init__(self):
        self.model = None
        self.class_names = [
            'deer', 'bear', 'fox', 'rabbit', 'squirrel',
            'bird', 'raccoon', 'coyote', 'empty', 'human'
        ]
        
    def build_model(self):
        """Build CNN model for wildlife detection"""
        self.model = models.Sequential([
            layers.Conv2D(32, (3, 3), activation='relu', input_shape=(224, 224, 3)),
            layers.MaxPooling2D((2, 2)),
            layers.Conv2D(64, (3, 3), activation='relu'),
            layers.MaxPooling2D((2, 2)),
            layers.Conv2D(128, (3, 3), activation='relu'),
            layers.MaxPooling2D((2, 2)),
            layers.Conv2D(128, (3, 3), activation='relu'),
            layers.MaxPooling2D((2, 2)),
            layers.Flatten(),
            layers.Dropout(0.5),
            layers.Dense(512, activation='relu'),
            layers.Dense(len(self.class_names), activation='softmax')
        ])
        
        self.model.compile(
            optimizer='adam',
            loss='sparse_categorical_crossentropy',
            metrics=['accuracy']
        )
        
    def train(self, data_dir, epochs=50):
        """Train the model on wildlife dataset"""
        X, y = self.load_dataset(data_dir)
        X_train, X_val, y_train, y_val = train_test_split(
            X, y, test_size=0.2, random_state=42
        )
        
        history = self.model.fit(
            X_train, y_train,
            epochs=epochs,
            validation_data=(X_val, y_val),
            callbacks=[
                tf.keras.callbacks.EarlyStopping(patience=5),
                tf.keras.callbacks.ModelCheckpoint(
                    'best_model.h5',
                    save_best_only=True
                )
            ]
        )
        return history
        
    def detect_wildlife(self, image_path):
        """Detect wildlife in a single image"""
        img = cv2.imread(image_path)
        img = cv2.resize(img, (224, 224))
        img = img / 255.0
        img = np.expand_dims(img, axis=0)
        
        predictions = self.model.predict(img)
        class_idx = np.argmax(predictions[0])
        confidence = predictions[0][class_idx]
        
        return {
            'species': self.class_names[class_idx],
            'confidence': float(confidence)
        }

# Edge AI for ESP32
class TinyMLModel:
    """Lightweight model for on-device inference"""
    def convert_to_tflite(self, keras_model_path):
        model = tf.keras.models.load_model(keras_model_path)
        converter = tf.lite.TFLiteConverter.from_keras_model(model)
        converter.optimizations = [tf.lite.Optimize.DEFAULT]
        converter.target_spec.supported_types = [tf.float16]
        tflite_model = converter.convert()
        
        with open('wildlife_model.tflite', 'wb') as f:
            f.write(tflite_model)