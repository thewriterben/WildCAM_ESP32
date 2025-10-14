# Advanced Research Methodologies

## Overview

This document outlines the scientific methodologies, statistical approaches, and best practices for conducting rigorous wildlife research using the WildCAM_ESP32 platform.

## Reproducible Research Framework

### Principles

1. **Transparency**: All methods, code, and data openly documented
2. **Replicability**: Others can reproduce analysis with same data
3. **Reproducibility**: Others can reproduce findings with new data collection
4. **Robustness**: Results hold under different analytical approaches

### Implementation

#### Version Control
```bash
# Initialize research project with version control
git init tiger-population-study
cd tiger-population-study

# Track data collection protocols
git add protocols/camera_placement.md
git add protocols/species_identification.md
git commit -m "Initial research protocols"

# Track analysis code
git add analysis/population_estimation.R
git commit -m "Add population estimation model"
```

#### Automated Workflows

```yaml
# .github/workflows/research-pipeline.yml
name: Research Data Pipeline

on:
  push:
    paths:
      - 'data/raw/**'

jobs:
  process-data:
    runs-on: ubuntu-latest
    steps:
      - uses: actions/checkout@v2
      
      - name: Setup R
        uses: r-lib/actions/setup-r@v2
      
      - name: Process detections
        run: Rscript scripts/process_detections.R
      
      - name: Generate reports
        run: Rscript scripts/generate_report.R
      
      - name: Run quality checks
        run: Rscript scripts/quality_checks.R
```

#### Computational Notebooks

```python
# notebooks/population_analysis.ipynb
"""
Population Dynamics Analysis
Author: Dr. Jane Smith
Date: 2025-10-14
Version: 1.0

This notebook analyzes tiger population trends from camera trap data
collected over 5 years across 3 protected areas.
"""

import pandas as pd
import numpy as np
import matplotlib.pyplot as plt
from wildcam_research import PopulationEstimator

# Load data with provenance tracking
data = pd.read_csv('data/processed/tiger_detections_2020_2025.csv')
print(f"Data loaded: {len(data)} observations")
print(f"Data hash: {hash(data.to_csv())}")

# Fit population model
estimator = PopulationEstimator(method='spatial_capture_recapture')
results = estimator.fit(data)

# Visualize trends with confidence intervals
estimator.plot_trends(save_path='figures/population_trend.png')

# Export results with metadata
results.to_csv('results/population_estimates.csv', 
               metadata={'method': 'SCR', 'version': '1.0'})
```

## Statistical Analysis Methods

### Population Estimation

#### Spatial Capture-Recapture (SCR)

```r
# R implementation
library(secr)
library(wildcamR)

# Load detection data
detections <- read_wildcam_detections("data/tiger_detections.csv")

# Prepare capture history
captures <- make_capthist(
  detections,
  detector_type = "proximity"
)

# Fit SCR model
model <- secr.fit(
  captures,
  model = list(D ~ 1, g0 ~ 1, sigma ~ 1),
  buffer = 10000,  # 10km buffer
  mask = habitat_mask
)

# Estimate density
density <- predict(model)
population <- density$estimate * area_km2

# Bootstrap confidence intervals
ci <- boot.secr(model, nboot = 1000)
```

#### Mark-Resight Analysis

```python
from wildcam_research import MarkResight

# Individual identification from images
individuals = identify_individuals(
    images=camera_trap_images,
    species='Panthera tigris',
    features=['stripe_pattern', 'whisker_spots']
)

# Mark-resight analysis
mr = MarkResight(
    marked_individuals=known_collared_tigers,
    sightings=individuals,
    survey_effort=camera_nights
)

# Estimate population size
N_estimate = mr.estimate_abundance(method='bowden_kufeld')
print(f"Population estimate: {N_estimate.mean} (95% CI: {N_estimate.ci})")
```

### Time Series Analysis

#### Trend Detection

```python
import numpy as np
from scipy import stats
from statsmodels.tsa.seasonal import seasonal_decompose

# Load time series data
ts_data = load_monthly_detections(species='Ursus arctos', years=range(2020, 2025))

# Decompose into trend, seasonal, and residual components
decomposition = seasonal_decompose(
    ts_data['count'],
    model='additive',
    period=12  # monthly seasonality
)

# Mann-Kendall trend test
trend_result = stats.mstats.kendalltau(
    range(len(ts_data)),
    ts_data['count']
)

if trend_result.pvalue < 0.05:
    print(f"Significant trend detected (p={trend_result.pvalue:.4f})")
    print(f"Kendall's tau: {trend_result.correlation:.4f}")
```

#### Change Point Detection

```r
library(changepoint)
library(bcp)

# Detect changes in population trends
detection_counts <- monthly_detections$count

# PELT algorithm for multiple change points
cpts <- cpt.meanvar(
  detection_counts,
  method = "PELT",
  penalty = "BIC"
)

# Bayesian change point analysis
bcp_result <- bcp(detection_counts)
plot(bcp_result)
```

### Behavioral Analysis

#### Activity Pattern Analysis

```r
library(activity)
library(overlap)

# Load detection times
tiger_times <- get_detection_times(species = "Panthera tigris")
human_times <- get_detection_times(species = "Homo sapiens")

# Estimate activity patterns
tiger_act <- fitact(tiger_times, sample = "data")
human_act <- fitact(human_times, sample = "data")

# Calculate temporal overlap
overlap_coef <- overlapEst(
  tiger_times,
  human_times,
  type = "Dhat4"
)

# Bootstrap confidence intervals
overlap_boot <- bootstrap(
  tiger_times,
  human_times,
  nb = 1000
)
```

#### Social Network Analysis

```python
import networkx as nx
from wildcam_research import SocialNetworkBuilder

# Build social network from co-occurrences
builder = SocialNetworkBuilder(
    detections=elephant_detections,
    temporal_threshold_hours=24,
    spatial_threshold_meters=100
)

network = builder.build_network()

# Calculate network metrics
centrality = nx.degree_centrality(network)
clustering = nx.clustering(network)
communities = nx.community.greedy_modularity_communities(network)

# Visualize social structure
builder.plot_network(
    node_size_by='centrality',
    color_by='community',
    save_path='figures/elephant_social_network.png'
)
```

### Spatial Analysis

#### Habitat Selection

```r
library(amt)
library(raster)

# Load GPS/camera trap locations and environmental layers
locations <- read_wildcam_locations("data/tiger_locations.csv")
habitat <- stack(
  "data/forest_cover.tif",
  "data/elevation.tif",
  "data/water_distance.tif"
)

# Fit resource selection function
rsf <- fit_rsf(
  case_ ~ forest_cover + elevation + water_distance,
  data = locations,
  control_type = "random"
)

# Predict habitat suitability
suitability_map <- predict(habitat, rsf)
writeRaster(suitability_map, "results/habitat_suitability.tif")
```

#### Home Range Analysis

```python
from wildcam_research import HomeRangeEstimator

# Estimate home ranges for individual tigers
estimator = HomeRangeEstimator(method='kernel_density')

for individual_id in individual_tigers:
    locations = get_locations(individual_id)
    
    home_range = estimator.estimate(
        locations,
        bandwidth_method='reference',
        contour_levels=[50, 95]  # 50% core, 95% home range
    )
    
    # Calculate area
    core_area = home_range.area(level=50)
    total_area = home_range.area(level=95)
    
    print(f"Individual {individual_id}:")
    print(f"  Core area: {core_area:.2f} km²")
    print(f"  Home range: {total_area:.2f} km²")
```

## Machine Learning Applications

### Species Classification

#### Transfer Learning

```python
import tensorflow as tf
from wildcam_research import SpeciesClassifier

# Load pre-trained model
base_model = tf.keras.applications.ResNet50(
    weights='imagenet',
    include_top=False,
    input_shape=(224, 224, 3)
)

# Fine-tune for local species
model = SpeciesClassifier(
    base_model=base_model,
    species_list=['Panthera tigris', 'Ursus arctos', 'Sus scrofa'],
    freeze_layers=100
)

# Train on local dataset
history = model.train(
    train_data='data/training/',
    validation_data='data/validation/',
    epochs=50,
    batch_size=32,
    augmentation=True
)

# Evaluate performance
metrics = model.evaluate(test_data='data/test/')
print(f"Test accuracy: {metrics['accuracy']:.4f}")
print(f"F1-score: {metrics['f1_macro']:.4f}")
```

#### Behavioral Classification

```python
from wildcam_research import BehaviorClassifier

# Train behavior recognition model
behaviors = ['hunting', 'resting', 'territorial_marking', 'social_interaction']

classifier = BehaviorClassifier(
    architecture='3D-CNN',
    temporal_window=30,  # 30 seconds
    behaviors=behaviors
)

# Train with video sequences
classifier.train(
    video_sequences='data/behavior_videos/',
    labels='data/behavior_labels.csv',
    validation_split=0.2
)

# Predict behaviors in new videos
predictions = classifier.predict_video('data/new_detection.mp4')
```

### Predictive Modeling

#### Species Distribution Models

```r
library(dismo)
library(raster)

# Prepare occurrence data
occurrences <- read_wildcam_occurrences(species = "Panthera tigris")

# Environmental predictors
predictors <- stack(
  "data/temperature.tif",
  "data/precipitation.tif",
  "data/elevation.tif",
  "data/forest_cover.tif",
  "data/human_density.tif"
)

# Fit MaxEnt model
maxent_model <- maxent(
  x = predictors,
  p = occurrences[, c("longitude", "latitude")],
  args = c('responsecurves=true', 'jackknife=true')
)

# Predict distribution
prediction <- predict(maxent_model, predictors)

# Evaluate model performance
evaluation <- evaluate(
  p = occurrences_test,
  a = background_points,
  model = maxent_model,
  x = predictors
)
```

#### Population Viability Analysis

```python
from wildcam_research import PopulationViabilityAnalysis

# Load population data
pop_data = load_population_timeseries(
    species='Panthera tigris',
    years=range(2000, 2025)
)

# Fit population model
pva = PopulationViabilityAnalysis(
    initial_population=pop_data['N0'],
    carrying_capacity=500,
    growth_rate=0.05,
    environmental_stochasticity=0.15,
    demographic_stochasticity=True
)

# Run simulations
extinction_risk = pva.simulate(
    years=50,
    n_simulations=10000,
    management_scenarios=[
        'baseline',
        'habitat_restoration',
        'anti_poaching',
        'combined'
    ]
)

# Plot results
pva.plot_scenarios(save_path='figures/pva_scenarios.png')
```

## Quality Assurance Protocols

### Data Quality Checks

```python
from wildcam_research import DataQualityChecker

# Initialize quality checker
qc = DataQualityChecker()

# Check detection data
results = qc.check_detections(
    data='data/raw_detections.csv',
    checks=[
        'missing_values',
        'duplicate_records',
        'invalid_coordinates',
        'temporal_consistency',
        'species_validation',
        'confidence_thresholds'
    ]
)

# Generate quality report
qc.generate_report(
    results,
    output='reports/data_quality_report.html'
)

# Flag suspicious records
flagged = results.get_flagged_records()
flagged.to_csv('data/flagged_records.csv')
```

### Inter-Observer Reliability

```r
library(irr)

# Load classification data from multiple observers
obs1 <- read.csv("data/observer1_classifications.csv")
obs2 <- read.csv("data/observer2_classifications.csv")

# Calculate Cohen's Kappa
kappa_result <- kappa2(
  data.frame(obs1$species, obs2$species),
  weight = "unweighted"
)

print(paste("Cohen's Kappa:", round(kappa_result$value, 3)))
print(paste("Agreement:", round(kappa_result$value, 3)))

# Calculate percent agreement by species
agreement_by_species <- aggregate(
  obs1$species == obs2$species,
  by = list(obs1$species),
  FUN = mean
)
```

## Ethical Considerations

### Minimal Disturbance Protocol

```yaml
# protocol/minimal_disturbance.yml
camera_placement:
  distance_from_den: ">50m"
  distance_from_nesting_site: ">100m"
  avoid_migration_corridors: true
  
camera_settings:
  flash: false
  silent_mode: true
  trigger_delay: 30  # seconds between triggers
  
monitoring_limits:
  max_cameras_per_territory: 3
  deployment_duration_months: 12
  recovery_period_months: 3
```

### Indigenous Knowledge Integration

```python
from wildcam_research import IndigenousKnowledgeProtocol

# Follow CARE principles for Indigenous Data Governance
ikp = IndigenousKnowledgeProtocol(
    community='Local Community Name',
    territory='Territory Identifier'
)

# Request consent for research
consent = ikp.request_consent(
    project_description='Long-term tiger monitoring',
    data_usage='Scientific publication and conservation planning',
    benefit_sharing='Employment, training, co-authorship'
)

if consent.granted:
    # Proceed with data collection
    # Integrate traditional ecological knowledge
    tek = ikp.gather_traditional_knowledge(
        species='Panthera tigris',
        topics=['habitat_preferences', 'behavior', 'seasonal_patterns']
    )
    
    # Acknowledge in publications
    citation = ikp.generate_citation()
```

## Reporting Standards

### Standardized Metrics

All research reports should include:

1. **Study Design**
   - Sampling methodology
   - Sample size justification
   - Statistical power analysis

2. **Data Collection**
   - Camera trap specifications
   - Deployment protocol
   - Quality control measures

3. **Analysis Methods**
   - Statistical tests used
   - Software versions
   - Model selection criteria

4. **Results**
   - Effect sizes with confidence intervals
   - P-values and significance levels
   - Diagnostic plots

5. **Reproducibility**
   - Data availability statement
   - Code repository link
   - Protocol documentation

### Example Report Template

```markdown
# Tiger Population Dynamics Study - Annual Report 2025

## Study Metadata
- Principal Investigator: Dr. Jane Smith
- Institution: Stanford University
- Project ID: TIGER_2025
- Study Period: 2025-01-01 to 2025-12-31
- Funding: NSF Grant #123456

## Methods
### Camera Trap Deployment
- Number of cameras: 150
- Survey area: 1,200 km²
- Camera nights: 54,750
- Deployment protocol: Version 2.3

### Species Identification
- AI Model: WildCAM Species Classifier v3.1
- Manual verification: Yes (all detections)
- Inter-observer agreement: κ = 0.92

### Statistical Analysis
- Method: Spatial Capture-Recapture
- Software: R (v4.3.0), secr (v4.6.0)
- Code repository: github.com/researcher/tiger-analysis

## Results
### Population Estimate
- N = 42 individuals (95% CI: 38-47)
- Density = 3.5 tigers/100 km² (95% CI: 3.2-3.9)
- Sex ratio = 1:1.1 (M:F)

### Trend Analysis
- Annual growth rate: 0.12 (95% CI: 0.08-0.16)
- Mann-Kendall test: τ = 0.45, p < 0.001

## Data Availability
- Raw data: doi:10.5061/dryad.12345
- Analysis code: github.com/researcher/tiger-analysis
- Camera trap images: Available upon request (sensitive locations)

## Ethics Statement
Study approved by Stanford IRB #2024-001 and Forest Department permit #FD-2025-123.
```

## Best Practices Summary

### Data Collection
✅ Use standardized protocols
✅ Document all methodology
✅ Implement quality control
✅ Regular equipment calibration
✅ Backup data immediately

### Analysis
✅ Pre-register analysis plans
✅ Use appropriate statistical tests
✅ Check model assumptions
✅ Report effect sizes and CIs
✅ Account for multiple comparisons

### Reporting
✅ Follow ARRIVE guidelines
✅ Make data publicly available
✅ Share analysis code
✅ Report negative results
✅ Acknowledge limitations

### Ethics
✅ Obtain necessary permits
✅ Minimize animal disturbance
✅ Protect sensitive locations
✅ Engage local communities
✅ Follow CARE principles

## Resources

### Statistical Tools
- R packages: secr, activity, overlap, amt
- Python packages: scipy, statsmodels, scikit-learn
- Software: Program MARK, PRESENCE, Distance

### Data Standards
- Darwin Core Archive
- Camera Trap Data Package
- Ecological Metadata Language (EML)

### Training Materials
- `/docs/education/workshop_curriculum.md`
- Online courses: wildcam.org/training
- Webinars: Monthly research methodology series

### Support
- Statistics consultation: stats@wildcam.org
- Methodology review: methods@wildcam.org
- Community forum: forum.wildcam.org/research
