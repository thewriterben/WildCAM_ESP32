# WildCAM Dashboard - Examples & Templates

**Collection of themes, configurations, and workflow examples for power users**

## 📁 Directory Structure

```
examples/
├── themes/              # Custom theme examples
│   ├── forest-theme.js
│   ├── ocean-theme.js
│   └── desert-theme.js
├── configs/            # Environment configuration examples
│   ├── research-station.env
│   ├── field-deployment.env
│   └── conservation-org.env
├── workflows/          # Common workflow guides
│   └── daily-monitoring.md
└── README.md          # This file
```

## 🎨 Themes

### Forest Theme
**Nature-inspired dark theme with deep greens and browns**

```javascript
import forestTheme from './examples/themes/forest-theme';

<ThemeProvider theme={forestTheme}>
  <App />
</ThemeProvider>
```

**Best for:**
- Woodland monitoring
- Forest research stations
- Natural color palette preference

**Colors:**
- Primary: Deep forest green (#2d5016)
- Secondary: Saddle brown (#8b4513)
- Background: Rich black (#1a1a1a)

### Ocean Theme
**Cool, professional theme with ocean blues**

```javascript
import oceanTheme from './examples/themes/ocean-theme';

<ThemeProvider theme={oceanTheme}>
  <App />
</ThemeProvider>
```

**Best for:**
- Coastal wildlife monitoring
- Marine research
- Professional presentations

**Colors:**
- Primary: Deep ocean blue (#006994)
- Secondary: Teal (#0097a7)
- Background: Deep ocean (#0a1929)

### Desert Theme
**Warm, earthy light theme with sandy tones**

```javascript
import desertTheme from './examples/themes/desert-theme';

<ThemeProvider theme={desertTheme}>
  <App />
</ThemeProvider>
```

**Best for:**
- Arid region monitoring
- Desert wildlife research
- High-contrast displays

**Colors:**
- Primary: Amber (#d97706)
- Secondary: Brown (#78350f)
- Background: Sand (#fef3c7)

## ⚙️ Configuration Examples

### Research Station
**Optimized for research facilities with multiple cameras**

```bash
cp examples/configs/research-station.env .env
```

**Features:**
- High data volume handling
- Extended collaboration features
- Comprehensive analytics
- Multiple export formats
- Advanced AI features enabled

**Recommended for:**
- University research labs
- Government facilities
- Large-scale monitoring projects
- Multi-team collaboration

### Field Deployment
**Optimized for remote locations with limited connectivity**

```bash
cp examples/configs/field-deployment.env .env
```

**Features:**
- Low bandwidth optimization
- Offline-first approach
- Battery-efficient settings
- Simplified UI
- Extended cache TTL

**Recommended for:**
- Remote wilderness areas
- Mobile field stations
- Limited internet access
- Solo researchers

### Conservation Organization
**Optimized for conservation groups with team collaboration**

```bash
cp examples/configs/conservation-org.env .env
```

**Features:**
- Team collaboration tools
- Comprehensive reporting
- Multiple user roles
- Map visualization
- Export for presentations

**Recommended for:**
- NGOs and conservation groups
- Wildlife protection agencies
- Community-based monitoring
- Multi-stakeholder projects

## 📋 Workflows

### Daily Monitoring Workflow
**Structured routine for efficient daily wildlife monitoring**

[View Full Workflow →](./workflows/daily-monitoring.md)

**Includes:**
- Morning check routine (5-10 min)
- Midday health check (3-5 min)
- Evening review (10-15 min)
- Weekly tasks
- Monthly tasks
- Quick reference commands

**Key Features:**
- Keyboard shortcut references
- Console commands
- Data quality checks
- Report generation
- Troubleshooting tips

## 🚀 Quick Start

### Using a Theme

1. Copy theme file to your project:
```bash
cp examples/themes/forest-theme.js src/theme/
```

2. Import and apply in `src/App.js`:
```javascript
import { ThemeProvider } from '@mui/material/styles';
import forestTheme from './theme/forest-theme';

function App() {
  return (
    <ThemeProvider theme={forestTheme}>
      {/* Your app components */}
    </ThemeProvider>
  );
}
```

### Using a Configuration

1. Copy configuration file:
```bash
cp examples/configs/research-station.env .env
```

2. Edit with your specific values:
```bash
nano .env
# Update API URLs, credentials, etc.
```

3. Restart development server:
```bash
npm start
```

### Following a Workflow

1. Review workflow document:
```bash
cat examples/workflows/daily-monitoring.md
```

2. Customize for your needs:
- Adjust time estimates
- Add/remove tasks
- Modify console commands
- Create your own checklists

3. Print or bookmark for quick reference

## 🎓 Best Practices

### Theme Customization

**Do:**
- Start with an example theme
- Modify colors gradually
- Test in both light and dark environments
- Maintain sufficient contrast ratios
- Consider accessibility

**Don't:**
- Change too many properties at once
- Use colors without testing contrast
- Forget to test on mobile devices
- Ignore Material-UI guidelines

### Configuration Management

**Do:**
- Use `.env.example` as a template
- Document custom settings
- Test in staging before production
- Keep sensitive data in `.env.local`
- Version control `.env.example` only

**Don't:**
- Commit `.env` to git
- Share credentials in config files
- Use production URLs in development
- Ignore environment-specific settings

### Workflow Development

**Do:**
- Start simple, add complexity gradually
- Time your routines for accuracy
- Document keyboard shortcuts
- Include troubleshooting steps
- Share successful workflows with team

**Don't:**
- Create overly complex routines
- Skip documentation
- Ignore team feedback
- Forget to update workflows

## 🛠️ Customization Tips

### Creating Your Own Theme

1. Copy an existing theme:
```bash
cp examples/themes/forest-theme.js examples/themes/my-theme.js
```

2. Modify colors:
```javascript
palette: {
  primary: {
    main: '#your-color',
  },
  // ... customize other colors
}
```

3. Test thoroughly:
- Light and dark modes
- All component states
- Accessibility
- Mobile devices

### Creating Custom Configuration

1. Start with closest example:
```bash
cp examples/configs/research-station.env .env.custom
```

2. Adjust settings:
```bash
# Add your custom settings
REACT_APP_CUSTOM_FEATURE=true
REACT_APP_CUSTOM_VALUE=123
```

3. Document changes:
```bash
# Add comments explaining custom settings
# This enables our custom feature
REACT_APP_CUSTOM_FEATURE=true
```

### Creating Custom Workflow

1. Use template structure:
```markdown
# Workflow Name
## Morning Tasks
- [ ] Task 1
- [ ] Task 2

## Midday Tasks
- [ ] Task 3

## Evening Tasks
- [ ] Task 4
```

2. Add code examples:
```javascript
// Helpful console commands
wildcam.myCustomFunction();
```

3. Include troubleshooting:
```markdown
### Common Issues
- Problem: Description
  Solution: Steps to resolve
```

## 📚 Additional Resources

### Documentation
- [Main README](../README.md)
- [Design Guide](../DESIGN_GUIDE.md)
- [Configuration Guide](../CONFIGURATION_GUIDE.md)
- [Deployment Guide](../DEPLOYMENT_GUIDE.md)

### Material-UI Resources
- [Theme Documentation](https://mui.com/material-ui/customization/theming/)
- [Color Tool](https://material.io/resources/color/)
- [Component API](https://mui.com/material-ui/api/)

### Inspiration
- [Material Design](https://material.io/)
- [Color Hunt](https://colorhunt.co/)
- [Coolors](https://coolors.co/)

## 🤝 Contributing

Have a great theme, configuration, or workflow to share?

1. Create your example
2. Test thoroughly
3. Document usage
4. Submit a pull request
5. Share with the community

## 📧 Support

Questions about examples?
- [GitHub Issues](https://github.com/thewriterben/WildCAM_ESP32/issues)
- [GitHub Discussions](https://github.com/thewriterben/WildCAM_ESP32/discussions)

---

**Examples Version**: 1.0  
**Last Updated**: 2025-10-14  
**Maintained by**: WildCAM ESP32 Team
