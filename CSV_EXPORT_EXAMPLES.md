# CSV Export Usage Examples

This document provides practical examples for using the CSV export endpoint.

## Quick Start

### Example 1: Download from Browser
Simply open this URL in your browser (replace `ESP32_IP` with your device's IP address):
```
http://192.168.1.100/api/export/detections.csv
```

Your browser will download a file named `detections_20251016.csv` (with today's date).

### Example 2: Using curl
```bash
# Download to current directory
curl -O http://192.168.1.100/api/export/detections.csv

# Download with custom filename
curl -o my_wildlife_data.csv http://192.168.1.100/api/export/detections.csv

# Download with date range filter
curl -O "http://192.168.1.100/api/export/detections.csv?start=2025-01-01&end=2025-12-31"
```

### Example 3: Using wget
```bash
wget http://192.168.1.100/api/export/detections.csv
```

## Advanced Usage

### Example 4: Python - Download and Parse
```python
import requests
import csv
from io import StringIO

# Download CSV
url = 'http://192.168.1.100/api/export/detections.csv'
response = requests.get(url)

# Parse CSV
csv_data = StringIO(response.text)
reader = csv.DictReader(csv_data)

# Process each detection
for row in reader:
    print(f"Species: {row['Species']}, Confidence: {row['Confidence']}")
    print(f"  Location: ({row['GPS_Lat']}, {row['GPS_Lon']})")
    print(f"  Battery: {row['Battery_Level']}%")
    print()
```

### Example 5: Python - Filter by Date Range
```python
import requests
import pandas as pd
from io import StringIO

# Request with date range
url = 'http://192.168.1.100/api/export/detections.csv'
params = {
    'start': '2025-09-01',
    'end': '2025-09-30'
}
response = requests.get(url, params=params)

# Load into pandas DataFrame
df = pd.read_csv(StringIO(response.text))

# Analyze data
print(f"Total detections: {len(df)}")
print(f"Species breakdown:")
print(df['Species'].value_counts())
print(f"\nAverage confidence: {df['Confidence'].astype(float).mean():.2f}")
```

### Example 6: JavaScript - Download from Web App
```javascript
async function downloadDetections(startDate, endDate) {
    const baseUrl = 'http://192.168.1.100/api/export/detections.csv';
    const params = new URLSearchParams();
    
    if (startDate) params.append('start', startDate);
    if (endDate) params.append('end', endDate);
    
    const url = `${baseUrl}?${params.toString()}`;
    
    try {
        const response = await fetch(url);
        const blob = await response.blob();
        
        // Create download link
        const downloadUrl = window.URL.createObjectURL(blob);
        const a = document.createElement('a');
        a.href = downloadUrl;
        a.download = `detections_${new Date().toISOString().split('T')[0]}.csv`;
        document.body.appendChild(a);
        a.click();
        
        // Cleanup
        window.URL.revokeObjectURL(downloadUrl);
        document.body.removeChild(a);
        
        console.log('CSV download started');
    } catch (error) {
        console.error('Download failed:', error);
    }
}

// Usage
downloadDetections('2025-01-01', '2025-12-31');
```

### Example 7: Node.js - Download and Save
```javascript
const axios = require('axios');
const fs = require('fs');

async function downloadCSV(filename) {
    const url = 'http://192.168.1.100/api/export/detections.csv';
    
    try {
        const response = await axios.get(url, {
            responseType: 'stream'
        });
        
        const writer = fs.createWriteStream(filename);
        response.data.pipe(writer);
        
        return new Promise((resolve, reject) => {
            writer.on('finish', resolve);
            writer.on('error', reject);
        });
    } catch (error) {
        console.error('Download failed:', error);
        throw error;
    }
}

// Usage
downloadCSV('wildlife_detections.csv')
    .then(() => console.log('Download complete'))
    .catch(err => console.error('Error:', err));
```

### Example 8: R - Download and Analyze
```r
library(httr)
library(readr)

# Download CSV
url <- "http://192.168.1.100/api/export/detections.csv"
response <- GET(url)

# Parse CSV
detections <- read_csv(content(response, "text"))

# Analyze
summary(detections)
table(detections$Species)

# Plot confidence distribution
hist(detections$Confidence, 
     main="Detection Confidence Distribution",
     xlab="Confidence Score",
     col="lightblue")
```

### Example 9: Excel Power Query
1. Open Excel
2. Go to Data → Get Data → From Web
3. Enter URL: `http://192.168.1.100/api/export/detections.csv`
4. Click OK and Load
5. Excel will automatically parse the CSV

For filtered data with dates:
```
http://192.168.1.100/api/export/detections.csv?start=2025-01-01&end=2025-12-31
```

### Example 10: Bash Script for Automated Backups
```bash
#!/bin/bash
# backup_detections.sh - Download and backup wildlife detection data

ESP32_IP="192.168.1.100"
BACKUP_DIR="$HOME/wildlife_backups"
DATE=$(date +%Y%m%d_%H%M%S)
FILENAME="detections_backup_$DATE.csv"

# Create backup directory if it doesn't exist
mkdir -p "$BACKUP_DIR"

# Download CSV
echo "Downloading detection data..."
curl -s -o "$BACKUP_DIR/$FILENAME" "http://$ESP32_IP/api/export/detections.csv"

if [ $? -eq 0 ]; then
    echo "Backup saved to: $BACKUP_DIR/$FILENAME"
    
    # Optional: Compress old backups
    find "$BACKUP_DIR" -name "*.csv" -mtime +7 -exec gzip {} \;
    echo "Old backups compressed"
else
    echo "Backup failed!"
    exit 1
fi
```

### Example 11: PowerShell - Download with Progress
```powershell
# download-detections.ps1
$url = "http://192.168.1.100/api/export/detections.csv"
$output = "detections_$(Get-Date -Format 'yyyyMMdd').csv"

Write-Host "Downloading wildlife detection data..."
Invoke-WebRequest -Uri $url -OutFile $output -UseBasicParsing

if (Test-Path $output) {
    $fileSize = (Get-Item $output).Length
    Write-Host "Download complete! File size: $($fileSize / 1KB) KB"
    
    # Optional: Import and display summary
    $csv = Import-Csv $output
    Write-Host "`nTotal records: $($csv.Count)"
    Write-Host "Species detected: $($csv.Species | Sort-Object -Unique)"
} else {
    Write-Host "Download failed!"
}
```

## Scheduled Exports

### Example 12: Cron Job (Linux/Mac)
Add to crontab (`crontab -e`):
```bash
# Daily backup at 2 AM
0 2 * * * curl -s -o /path/to/backups/detections_$(date +\%Y\%m\%d).csv http://192.168.1.100/api/export/detections.csv

# Weekly backup every Sunday at 3 AM
0 3 * * 0 curl -s -o /path/to/backups/weekly_$(date +\%Y\%U).csv http://192.168.1.100/api/export/detections.csv
```

### Example 13: Windows Task Scheduler
Create a PowerShell script and schedule it:
```powershell
# scheduled-export.ps1
$url = "http://192.168.1.100/api/export/detections.csv"
$backupPath = "C:\WildlifeBackups"
$date = Get-Date -Format "yyyyMMdd"
$filename = "$backupPath\detections_$date.csv"

New-Item -ItemType Directory -Force -Path $backupPath | Out-Null
Invoke-WebRequest -Uri $url -OutFile $filename -UseBasicParsing
```

Schedule in Task Scheduler:
- Action: Start a program
- Program: `powershell.exe`
- Arguments: `-ExecutionPolicy Bypass -File "C:\Scripts\scheduled-export.ps1"`

## Data Analysis Examples

### Example 14: Calculate Detection Statistics
```python
import pandas as pd
import requests
from io import StringIO

# Download and load data
url = 'http://192.168.1.100/api/export/detections.csv'
response = requests.get(url)
df = pd.read_csv(StringIO(response.text))

# Convert timestamp to datetime
df['DateTime'] = pd.to_datetime(df['Timestamp'], unit='s')

# Statistics
print("=== Detection Statistics ===")
print(f"Date range: {df['DateTime'].min()} to {df['DateTime'].max()}")
print(f"Total detections: {len(df)}")
print(f"\nSpecies counts:")
print(df['Species'].value_counts())
print(f"\nAverage confidence by species:")
print(df.groupby('Species')['Confidence'].mean().round(2))
print(f"\nDetections by hour:")
print(df['DateTime'].dt.hour.value_counts().sort_index())
```

### Example 15: Generate Detection Report
```python
import pandas as pd
import requests
from io import StringIO
from datetime import datetime

def generate_report():
    # Download data
    url = 'http://192.168.1.100/api/export/detections.csv'
    response = requests.get(url)
    df = pd.read_csv(StringIO(response.text))
    
    # Generate report
    report = f"""
    Wildlife Detection Report
    Generated: {datetime.now().strftime('%Y-%m-%d %H:%M:%S')}
    
    Summary:
    - Total Detections: {len(df)}
    - Unique Species: {df['Species'].nunique()}
    - Date Range: {df['Timestamp'].min()} to {df['Timestamp'].max()}
    - Average Confidence: {df['Confidence'].astype(float).mean():.2%}
    
    Top 5 Species:
    {df['Species'].value_counts().head().to_string()}
    
    High Confidence Detections (>90%):
    {len(df[df['Confidence'].astype(float) > 0.9])}
    
    Average Battery Level: {df['Battery_Level'].astype(int).mean():.1f}%
    """
    
    return report

# Generate and save report
report = generate_report()
with open('detection_report.txt', 'w') as f:
    f.write(report)
print(report)
```

## Troubleshooting

### Common Issues and Solutions

**Issue: "Connection refused"**
```bash
# Test if ESP32 is reachable
ping 192.168.1.100

# Check if web server is running
curl -I http://192.168.1.100/api/status
```

**Issue: "File is empty"**
```bash
# Check if data exists
curl -v http://192.168.1.100/api/export/detections.csv
```

**Issue: "Certificate error" (HTTPS)**
```bash
# Skip certificate verification (not recommended for production)
curl -k https://192.168.1.100/api/export/detections.csv
```

## Tips and Best Practices

1. **Regular Backups**: Schedule automatic exports to prevent data loss
2. **Date Filters**: Use date ranges to export specific time periods
3. **Batch Processing**: Process large exports in chunks if memory is limited
4. **Error Handling**: Always implement error handling in your scripts
5. **Validation**: Verify CSV format and data integrity after download
6. **Storage**: Compress old CSV files to save disk space
7. **Monitoring**: Log export operations for auditing

## Need Help?

Refer to the main documentation: [CSV_EXPORT_README.md](CSV_EXPORT_README.md)
