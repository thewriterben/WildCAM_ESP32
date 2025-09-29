# Budget Conservation Enclosures

This directory contains cost-effective 3D printable enclosures for wildlife conservation projects with limited budgets.

## Budget-Friendly Design Philosophy

### Design Goals
- **Minimize material usage** - Optimized wall thickness and infill
- **Simple printing** - No complex overhangs or supports required
- **Standard hardware** - Common screws and fasteners
- **Multi-board compatibility** - Works with various ESP32 boards
- **Easy assembly** - Snap-fit and interference connections

## Enclosure Components

### Basic Protection
- **basic_wildlife_cam_case.stl** - Simple weatherproof housing
  - Single-piece design to minimize assembly
  - 70mm x 55mm x 35mm internal space
  - Universal ESP32 board mounting
  - Basic weather protection (IP54)

### Power System
- **solar_panel_mount.stl** - Simple solar panel bracket
  - Fits 60-100mm solar panels
  - Fixed 30-degree tilt for optimal charging
  - Minimal material usage

- **battery_compartment.stl** - Secure battery housing
  - 18650 battery holder
  - Weatherproof seal
  - Easy access for maintenance

### Camera Protection
- **camera_lens_hood.stl** - Lens protection hood
  - Reduces glare and rain impact
  - UV protection for camera sensor
  - Snap-fit attachment to main case

## Material Selection for Budget Builds

### Primary Material: PLA
**Advantages:**
- Lowest cost filament option
- Easy to print on basic printers
- No heated bed required
- Biodegradable (eco-friendly)

**Limitations:**
- Limited UV resistance (6-12 months outdoors)
- Temperature sensitivity (60°C max)
- Requires UV coating for longevity

### UV Protection for PLA
```bash
# Post-processing steps for PLA longevity
1. Sand surface with 400-grit sandpaper
2. Apply UV-resistant primer
3. Apply 2 coats of automotive UV clear coat
4. Allow 48-hour cure time before deployment
```

### Alternative Budget Materials
1. **PETG** - Better weather resistance, still affordable
2. **Recycled filaments** - Environmental and cost benefits
3. **PLA+** - Enhanced PLA with better outdoor performance

## Print Settings for Cost Optimization

### Optimized Settings
```
Layer Height: 0.3mm (faster printing)
Infill: 15% (minimal, adequate strength)
Wall Thickness: 0.8mm (2 perimeters)
Top/Bottom Layers: 3 layers (minimal)
Support: Minimize with design optimization
Print Speed: 60mm/s (balance of speed/quality)
```

### Realistic Print Times and Costs
| Component | Print Time | Material Cost (PLA) | Power Cost | Printer Wear | Failed Prints (15%) |
|-----------|------------|---------------------|------------|--------------|---------------------|
| Main Case | 4-6 hours | $2.50 | $0.50 | $0.40 | $0.53 |
| Solar Mount | 2-3 hours | $1.25 | $0.25 | $0.20 | $0.26 |
| Battery Compartment | 3-4 hours | $1.75 | $0.35 | $0.30 | $0.37 |
| Lens Hood | 1-2 hours | $0.75 | $0.15 | $0.15 | $0.16 |
| **Subtotal** | **10-15 hours** | **$6.25** | **$1.25** | **$1.05** | **$1.32** |

### Additional Costs
| Item | Cost | Notes |
|------|------|-------|
| Post-processing materials | $1.50 | Sandpaper, solvents, supports removal |
| Quality control time | $2.00 | Testing, fitting, rework at $20/hour |
| **Total per unit** | **$13.37** | **Realistic all-in cost** |

> **Note:** Costs assume hobbyist 3D printer depreciation, 15% failure rate for complex prints, and basic post-processing. Professional printing services may cost 2-3x more but include quality guarantees.

## Assembly Without Specialized Tools

### Required Tools (Common Household Items)
- Phillips head screwdriver
- Drill with 3mm bit (or heated nail for holes)
- File or sandpaper for adjustments
- Silicone sealant (bathroom grade acceptable)

### Hardware Requirements
```
- M3 x 12mm screws (4 pieces) - $0.50
- M3 nuts (4 pieces) - $0.25
- Rubber gasket material - $1.00
- Cable glands (2 pieces) - $2.00
- Total hardware cost: $3.75
```

## Camouflage and Concealment

### Natural Camouflage Options
1. **Paint Treatment**
   - Brown/green base coat
   - Sponge-applied texture patterns
   - Matte finish to reduce reflection

2. **Natural Materials**
   - Attach real bark pieces with silicone
   - Moss growth encouragement (yogurt/moss mixture)
   - Natural fiber wrapping

3. **Modular Camo System**
   - Velcro attachment points for seasonal changes
   - Removable texture panels
   - Interchangeable color schemes

### Urban/Suburban Camouflage
- **Building Colors**: Gray, beige, white options
- **Utility Box Mimicry**: Design to look like electrical equipment
- **Maintenance Access**: "Property of..." labels

## Community Building and Sharing

### Group Purchase Benefits
- **Filament bulk buying** - 50% cost reduction possible
- **Shared printer access** - Reduce individual equipment needs
- **Knowledge sharing** - Design improvements and troubleshooting
- **Bulk hardware purchase** - Significant savings on screws/fasteners

### Educational Workshops
1. **3D Printing Basics** - Introduction to technology
2. **Design Modification** - Customizing for local needs
3. **Assembly Techniques** - Best practices and troubleshooting
4. **Deployment Strategies** - Optimal placement and maintenance

## Performance vs. Cost Analysis

### Budget Enclosure Performance
| Feature | Budget Design | Premium Design | Cost Difference |
|---------|---------------|----------------|-----------------|
| Weather Protection | IP54 (6-12 months) | IP54+ (2+ years) | +$15 materials |
| Temperature Range | -5°C to 40°C | -20°C to 60°C | +$25 materials |
| UV Resistance | 6-12 months | 5+ years | +$30 materials |
| Assembly Time | 30 minutes | 2+ hours | +$0 |
| Maintenance | Quarterly | Annual | +$0 |

### When to Upgrade
- **Extended deployment** (>1 year)
- **Harsh environments** (desert, arctic, coastal)
- **Minimal maintenance access**
- **Critical research applications**

## Troubleshooting Common Issues

### Print Quality Problems
1. **Layer adhesion failures**
   - Increase bed temperature to 60°C
   - Clean build surface thoroughly
   - Reduce print speed to 40mm/s

2. **Warping and curling**
   - Use brim or raft for better adhesion
   - Reduce infill to decrease internal stress
   - Ensure draft-free printing environment

3. **Dimensional inaccuracy**
   - Calibrate printer steps/mm
   - Adjust flow rate (usually 95-105%)
   - Check belt tension

### Assembly Issues
1. **Parts don't fit properly**
   - File or sand contact surfaces
   - Check print scaling (should be 100%)
   - Verify correct STL file version

2. **Weak joints or cracks**
   - Use appropriate screws (not too tight)
   - Pre-drill holes in brittle materials
   - Consider reinforcement with epoxy

### Field Performance Issues
1. **Water ingress**
   - Inspect and replace gaskets
   - Re-apply sealant at joints
   - Check for print layer separation

> **Weatherproofing reference:** For detailed sealing techniques and troubleshooting, see `../common/README.md`.

2. **UV degradation**
   - Re-apply UV protective coating
   - Consider material upgrade
   - Add physical sun shade

## Community Resources

### Design Sources and Credits
- **Thingiverse**: Public domain ESP32 enclosures
- **Printables**: Community-contributed designs
- **GitHub**: Open-source wildlife monitoring projects
- **Local maker spaces**: Custom design collaboration

### Support Networks
1. **Online Forums**: r/wildlifetech, ESP32 communities
2. **Local Groups**: Maker spaces, conservation organizations
3. **Educational Partners**: Universities, technical schools
4. **Professional Networks**: Wildlife researchers, conservation NGOs

### Contributing Back
1. **Document your builds** - Share successes and failures
2. **Improve designs** - Submit modifications and enhancements
3. **Share cost data** - Help others plan their budgets
4. **Mentor newcomers** - Teach 3D printing and assembly skills

## Licensing and Legal

### Open Source Commitment
All designs in this directory are released under:
- **Creative Commons Attribution-ShareAlike 4.0**
- Free for personal, educational, and commercial use
- Attribution required for derivatives
- Share-alike requirement for modifications

### Responsible Use
- Respect private property and wildlife regulations
- Follow local laws regarding wildlife monitoring
- Consider environmental impact of deployments
- Share data responsibly for conservation benefit

---

*Last updated: August 2025*
*Budget enclosure program version: 1.0*
*Total project cost target: Under $15 per unit*