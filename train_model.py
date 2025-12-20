
# AI Implementation: Model Training Script

# Author: Manus AI

# Date: October 29, 2025


import tensorflow as tf
from tensorflow.keras.applications import MobileNetV2
from tensorflow.keras.layers import Dense, GlobalAveragePooling2D, Dropout
from tensorflow.keras.models import Model
from tensorflow.keras.preprocessing.image import ImageDataGenerator

# --- Configuration ---
DATASET_DIR = './wildlife_dataset/'
IMAGE_SIZE = (128, 128)  # Keep this small for ESP32
BATCH_SIZE = 32
NUM_CLASSES = 10  # Must match the number of species in your dataset
EPOCHS_FEATURE_EXTRACTION = 10
EPOCHS_FINE_TUNING = 20
LEARNING_RATE_FINE_TUNING = 1e-5

# --- 1. Data Augmentation and Loading ---
def create_data_generators():
    # Augmentation for the training set
    train_datagen = ImageDataGenerator(
        rescale=1./255,
        rotation_range=20,
        width_shift_range=0.1,
        height_shift_range=0.1,
        shear_range=0.1,
        zoom_range=0.1,
        horizontal_flip=True,
        fill_mode='nearest'
    )

    # No augmentation for validation and test sets, only rescaling
    validation_datagen = ImageDataGenerator(rescale=1./255)

    train_generator = train_datagen.flow_from_directory(
        os.path.join(DATASET_DIR, 'train'),
        target_size=IMAGE_SIZE,
        batch_size=BATCH_SIZE,
        class_mode='categorical'
    )

    validation_generator = validation_datagen.flow_from_directory(
        os.path.join(DATASET_DIR, 'validation'),
        target_size=IMAGE_SIZE,
        batch_size=BATCH_SIZE,
        class_mode='categorical'
    )

    return train_generator, validation_generator

# --- 2. Model Architecture ---
def build_model():
    # Load pre-trained MobileNetV2 base
    base_model = MobileNetV2(
        input_shape=(*IMAGE_SIZE, 3),
        include_top=False,  # Exclude the final classification layer
        weights='imagenet'
    )

    # Freeze the base model layers
    base_model.trainable = False

    # Add custom classification head
    x = base_model.output
    x = GlobalAveragePooling2D()(x)
    x = Dropout(0.5)(x)  # Regularization
    predictions = Dense(NUM_CLASSES, activation='softmax')(x)

    model = Model(inputs=base_model.input, outputs=predictions)
    return model, base_model

# --- 3. Training Process ---
def main():
    train_generator, validation_generator = create_data_generators()
    model, base_model = build_model()

    # --- Phase 1: Feature Extraction ---
    print("\n--- Starting Feature Extraction Phase ---")
    model.compile(
        optimizer='adam',
        loss='categorical_crossentropy',
        metrics=['accuracy']
    )

    history_feature_extraction = model.fit(
        train_generator,
        epochs=EPOCHS_FEATURE_EXTRACTION,
        validation_data=validation_generator
    )

    print("\n--- Feature Extraction Complete ---")

    # --- Phase 2: Fine-Tuning ---
    print("\n--- Starting Fine-Tuning Phase ---")
    # Unfreeze the top layers of the base model
    base_model.trainable = True
    fine_tune_at = 100  # Fine-tune from this layer onwards
    for layer in base_model.layers[:fine_tune_at]:
        layer.trainable = False

    model.compile(
        optimizer=tf.keras.optimizers.Adam(learning_rate=LEARNING_RATE_FINE_TUNING),
        loss='categorical_crossentropy',
        metrics=['accuracy']
    )

    history_fine_tuning = model.fit(
        train_generator,
        epochs=EPOCHS_FINE_TUNING,
        validation_data=validation_generator
    )

    print("\n--- Fine-Tuning Complete ---")

    # --- 4. Save the Final Model ---
    print("\nSaving final model...")
    model.save('wildlife_classifier.h5')
    print("Model saved as wildlife_classifier.h5")

if __name__ == '__main__':
    main()
```

