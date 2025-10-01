# Satellite Communication Hardware Bill of Materials

## Overview

This document provides detailed hardware specifications and purchasing information for adding satellite communication capabilities to your ESP32 Wildlife Camera.

## Complete Satellite Module Kits

### Option 1: Swarm M138 Kit (Budget-Friendly)
**Recommended for: Cost-conscious deployments, frequent messaging**

| Item | Specification | Quantity | Unit Cost | Total | Source |
|------|--------------|----------|-----------|-------|--------|
| Swarm M138 Modem | VHF satellite modem | 1 | $119 | $119 | swarm.space |
| Swarm Antenna | VHF patch antenna | 1 | $29 | $29 | swarm.space |
| Swarm Data Plan | 750 messages/month | 1 | $5/mo | $5/mo | swarm.space |
| RF Cable | SMA to U.FL, 15cm | 1 | $8 | $8 | Amazon |
| **Total (One-time)** | | | | **$156** | |
| **Total (Monthly)** | | | | **$5** | |

**Monthly Cost Estimate**: $5 base + $0.05 per message above 750

### Option 2: RockBLOCK 9603 Kit (Professional)
**Recommended for: Reliable deployments, proven hardware**

| Item | Specification | Quantity | Unit Cost | Total | Source |
|------|--------------|----------|-----------|-------|--------|
| RockBLOCK 9603 | Iridium modem w/ antenna | 1 | $249.95 | $249.95 | rock7.com |
| Line Rental | Monthly subscription | 1 | $12/mo | $12/mo | rock7.com |
| Message Credits | Prepaid credits | 1 | $0.04-0.14 | varies | rock7.com |
| RF Cable (included) | SMA, 3m | 1 | included | $0 | - |
| **Total (One-time)** | | | | **$249.95** | |
| **Total (Monthly)** | | | | **$12+** | |

**Monthly Cost Estimate**: $12 base + $0.04-0.14 per message (bulk discounts available)

### Option 3: Iridium 9603N Kit (DIY)
**Recommended for: Custom integrations, advanced users**

| Item | Specification | Quantity | Unit Cost | Total | Source |
|------|--------------|----------|-----------|-------|--------|
| Iridium 9603N Module | Bare modem module | 1 | $199 | $199 | DigiKey/Mouser |
| Iridium Antenna | Passive patch antenna | 1 | $89 | $89 | Taoglas/DigiKey |
| SIM Card | Iridium data SIM | 1 | $50 | $50 | NAL Research |
| RF Cable | SMA to U.FL, 30cm | 1 | $12 | $12 | Amazon |
| Level Shifter | 3.3V logic (optional) | 1 | $5 | $5 | Amazon |
| **Total (One-time)** | | | | **$355** | |
| **Total (Monthly)** | | | | varies | |

**Monthly Cost Estimate**: Service provider dependent, typically $20-50/month base + per-message fees

## Required Supporting Hardware

### Power Supply Components

| Item | Specification | Quantity | Unit Cost | Total | Purpose |
|------|--------------|----------|-----------|-------|---------|
| Buck Converter | 5V/12V to 3.3V, 3A+ | 1 | $8 | $8 | Stable power |
| Capacitor (bulk) | 1000µF, 16V electrolytic | 2 | $1 | $2 | Power smoothing |
| Capacitor (bypass) | 100nF ceramic | 4 | $0.25 | $1 | Noise filtering |
| **Subtotal** | | | | **$11** | |

### Connection Components

| Item | Specification | Quantity | Unit Cost | Total | Purpose |
|------|--------------|----------|-----------|-------|---------|
| Jumper Wires | Female-female, 20cm | 10 | $0.15 | $1.50 | Prototyping |
| Dupont Connectors | 2.54mm pitch | 1 set | $5 | $5 | Permanent wiring |
| Heat Shrink Tubing | Assorted sizes | 1 set | $8 | $8 | Wire protection |
| Solder | Lead-free rosin core | 1 roll | $12 | $12 | Soldering |
| PCB (optional) | Custom breakout board | 1 | $15 | $15 | Clean integration |
| **Subtotal** | | | | **$41.50** | |

### Enclosure & Mounting

| Item | Specification | Quantity | Unit Cost | Total | Purpose |
|------|--------------|----------|-----------|-------|---------|
| Waterproof Box | IP67, 200x150x75mm | 1 | $25 | $25 | Main enclosure |
| Cable Glands | PG9, IP68 | 4 | $2 | $8 | Cable entry |
| Antenna Mount | Magnetic or pole mount | 1 | $15 | $15 | Antenna positioning |
| RF-Transparent Panel | Acrylic or HDPE | 1 | $8 | $8 | Antenna window |
| Mounting Hardware | Screws, standoffs | 1 set | $6 | $6 | Internal mounting |
| **Subtotal** | | | | **$62** | |

## Optional Enhancement Components

### Power Management

| Item | Specification | Quantity | Unit Cost | Total | Benefit |
|------|--------------|----------|-----------|-------|---------|
| Solar Panel | 10W mono-crystalline | 1 | $35 | $35 | Autonomous power |
| Solar Charge Controller | MPPT, 12V/24V | 1 | $25 | $25 | Efficient charging |
| Battery | 18650 Li-ion, 3000mAh | 4 | $8 | $32 | Energy storage |
| Battery Holder | 4-cell series holder | 1 | $6 | $6 | Battery management |
| **Subtotal** | | | | **$98** | Extended runtime |

### Environmental Protection

| Item | Specification | Quantity | Unit Cost | Total | Benefit |
|------|--------------|----------|-----------|-------|---------|
| Desiccant Packs | Silica gel, rechargeable | 4 | $2 | $8 | Moisture control |
| Thermal Insulation | Foam padding | 1 sheet | $12 | $12 | Temperature stability |
| Vent Plugs | Gore-Tex membrane | 2 | $8 | $16 | Pressure equalization |
| **Subtotal** | | | | **$36** | Weather resistance |

### Diagnostic & Testing

| Item | Specification | Quantity | Unit Cost | Total | Benefit |
|------|--------------|----------|-----------|-------|---------|
| USB-TTL Adapter | FTDI FT232RL | 1 | $10 | $10 | Serial debugging |
| Logic Analyzer | 8-channel | 1 | $25 | $25 | Protocol debugging |
| Multimeter | Digital, auto-ranging | 1 | $30 | $30 | Voltage/current testing |
| SMA Test Adapter | Male-female passthrough | 1 | $12 | $12 | Antenna testing |
| **Subtotal** | | | | **$77** | Development/testing |

## Complete System Costs

### Budget Configuration (Swarm-based)
| Category | Cost |
|----------|------|
| Satellite Module Kit | $156 |
| Supporting Hardware | $11 |
| Connection Components | $15 (minimal) |
| Basic Enclosure | $25 |
| **Total One-time** | **$207** |
| **Monthly Service** | **$5+** |

### Standard Configuration (RockBLOCK)
| Category | Cost |
|----------|------|
| Satellite Module Kit | $249.95 |
| Supporting Hardware | $11 |
| Connection Components | $41.50 |
| Enclosure & Mounting | $62 |
| **Total One-time** | **$364.45** |
| **Monthly Service** | **$12+** |

### Professional Configuration (Full Featured)
| Category | Cost |
|----------|------|
| Satellite Module Kit | $249.95 |
| Supporting Hardware | $11 |
| Connection Components | $41.50 |
| Enclosure & Mounting | $62 |
| Power Management | $98 |
| Environmental Protection | $36 |
| Diagnostic Tools | $77 |
| **Total One-time** | **$575.95** |
| **Monthly Service** | **$12+** |

## Vendor Information

### Satellite Module Suppliers

**Swarm Technologies**
- Website: https://swarm.space
- Products: M138 modem, antennas, accessories
- Support: support@swarm.space
- Shipping: Worldwide
- Lead Time: 2-4 weeks

**Rock Seven (RockBLOCK)**
- Website: https://www.rock7.com
- Products: RockBLOCK modules, accessories
- Support: support@rock7.com
- Shipping: Worldwide
- Lead Time: 1-2 weeks

**NAL Research**
- Website: https://www.nalresearch.com
- Products: Iridium modules, SIM cards, service plans
- Support: sales@nalresearch.com
- Shipping: North America
- Lead Time: 1-3 weeks

### Component Suppliers

**DigiKey Electronics**
- Website: https://www.digikey.com
- Products: Iridium modules, electronic components
- Shipping: Worldwide, fast
- Lead Time: 1-3 days

**Mouser Electronics**
- Website: https://www.mouser.com
- Products: Electronic components, connectors
- Shipping: Worldwide
- Lead Time: 1-3 days

**Amazon**
- Website: https://www.amazon.com
- Products: Cables, connectors, enclosures
- Shipping: Prime available
- Lead Time: 1-2 days

**SparkFun Electronics**
- Website: https://www.sparkfun.com
- Products: Development boards, sensors
- Shipping: Worldwide
- Lead Time: 1-5 days

**Adafruit Industries**
- Website: https://www.adafruit.com
- Products: Development boards, accessories
- Shipping: Worldwide
- Lead Time: 1-5 days

## Service Plan Comparison

### Swarm Pricing (as of 2024)
- **Base Plan**: $5/month
- **Included Messages**: 750/month
- **Additional Messages**: $0.05 each
- **Contract**: Month-to-month
- **Best For**: Frequent messaging, predictable costs

### RockBLOCK Pricing
- **Line Rental**: $12/month
- **Message Credits**: $0.04-0.14 per message (bulk discounts)
- **Contract**: Monthly or annual
- **Best For**: Flexible usage, proven reliability

### Iridium Prepaid
- **Service Provider**: Multiple options (NAL, Iridium, etc.)
- **Typical Cost**: $20-50/month base + per-message
- **Message Cost**: $0.50-1.00 each
- **Contract**: Varies by provider
- **Best For**: Custom requirements, enterprise

## Cost Optimization Tips

### Hardware Savings
1. **Buy in bulk**: Order multiple units for volume discounts
2. **Use generic components**: Substitute RF cables, connectors where possible
3. **3D print enclosures**: DIY enclosures if you have access to printer
4. **Reuse parts**: Salvage connectors, cables from old electronics
5. **Start minimal**: Add optional components only as needed

### Service Cost Savings
1. **Choose Swarm**: Lowest per-message cost ($0.05 vs $0.95)
2. **Optimize transmission frequency**: Send less frequently
3. **Use message compression**: Built-in feature saves bandwidth
4. **Batch messages**: Combine multiple updates into one transmission
5. **Set daily limits**: Prevent unexpected charges
6. **Prepay credits**: Some providers offer discounts for prepaid bulk credits

## Warranty & Support

### Module Warranties
- **Swarm M138**: 1 year manufacturer warranty
- **RockBLOCK 9603**: 2 year manufacturer warranty
- **Iridium 9603N**: 1 year manufacturer warranty

### Support Options
- **Email Support**: All vendors provide email support
- **Phone Support**: RockBLOCK and NAL Research offer phone support
- **Community Forums**: Active user communities for troubleshooting
- **Documentation**: Comprehensive guides from all vendors

## Environmental Ratings

### Module Specifications
| Module | Operating Temp | Storage Temp | Humidity | IP Rating |
|--------|---------------|--------------|----------|-----------|
| Swarm M138 | -40°C to +85°C | -40°C to +85°C | 0-100% RH | IP67 (with housing) |
| RockBLOCK 9603 | -40°C to +85°C | -40°C to +85°C | 0-100% RH | IP67 (with housing) |
| Iridium 9603N | -40°C to +70°C | -40°C to +85°C | 0-95% RH | Not rated (bare module) |

### Deployment Considerations
- Use IP67+ rated enclosures for outdoor deployment
- Add thermal insulation for extreme temperature environments
- Include desiccant packs for high-humidity locations
- Select UV-resistant materials for sun exposure
- Consider lightning protection for exposed antenna installations

## Purchase Checklist

### Before Ordering
- [ ] Decide on satellite module type (Swarm/RockBLOCK/Iridium)
- [ ] Calculate expected message volume and costs
- [ ] Check regulatory approval for your region
- [ ] Verify antenna compatibility
- [ ] Confirm power supply requirements
- [ ] Review vendor shipping times and costs

### Minimum Order
- [ ] Satellite module
- [ ] Antenna (if not included)
- [ ] RF cable (if not included)
- [ ] Jumper wires for prototyping
- [ ] Power supply (if needed)

### Recommended Order
- [ ] All minimum items above
- [ ] Waterproof enclosure
- [ ] Cable glands
- [ ] Antenna mount
- [ ] Capacitors for power smoothing
- [ ] Desiccant packs

### Professional Order
- [ ] All recommended items above
- [ ] Solar panel and charge controller
- [ ] Battery pack
- [ ] Environmental protection components
- [ ] Diagnostic tools
- [ ] Spare connectors and cables

## Regional Considerations

### North America
- ✓ All modules supported
- ✓ Fast shipping available
- ✓ Local service providers
- $ Competitive pricing

### Europe
- ✓ All modules supported
- ✓ EU-wide shipping
- ⚠ Import duties may apply
- $ Slightly higher costs

### Asia-Pacific
- ✓ All modules supported
- ⚠ Longer shipping times
- ⚠ Regulatory approval varies by country
- $ Import duties apply

### South America / Africa
- ✓ Satellite coverage available
- ⚠ Limited local distributors
- ⚠ Higher shipping costs
- ⚠ Extended delivery times

## Maintenance & Replacement

### Expected Lifespan
- **Satellite modules**: 5-10 years
- **Antennas**: 10+ years (with proper mounting)
- **RF cables**: 5-10 years (outdoor use)
- **Enclosures**: 5-10 years (UV-resistant materials)

### Replacement Parts Inventory
Recommended spare parts for field deployments:
- [ ] RF cable (1 spare)
- [ ] Jumper wire set (1 spare)
- [ ] Capacitors (2 of each value)
- [ ] Cable glands (2 spares)
- [ ] Desiccant packs (4 spares)
- [ ] Fuses (if used, 5 spares)

---

**Note**: Prices are approximate and subject to change. Check vendor websites for current pricing. Bulk order discounts may be available for research institutions or large deployments.
