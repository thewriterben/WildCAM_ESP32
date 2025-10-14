# 🎨 WildCAM Dashboard - UI/UX Design Guide

**Comprehensive design system and visual guidelines for the advanced user dashboard**

## 📋 Table of Contents

- [Design Philosophy](#-design-philosophy)
- [Color System](#-color-system)
- [Typography](#-typography)
- [Component Library](#-component-library)
- [Layout & Grid](#-layout--grid)
- [Responsive Design](#-responsive-design)
- [Icons & Imagery](#-icons--imagery)
- [Interactions](#-interactions)
- [Accessibility](#-accessibility)
- [Dark Mode](#-dark-mode)

## 🎯 Design Philosophy

### Core Principles

**1. Data-First Interface**
- Prioritize actionable information
- Minimize visual noise and clutter
- Use progressive disclosure for complex data
- Clear visual hierarchy for scanning

**2. Professional & Technical**
- Clean, modern aesthetic suitable for research and monitoring
- Technical but not intimidating
- Professional color palette and typography
- Precise data representation

**3. Real-Time Awareness**
- Clear indicators for live data
- Smooth transitions and updates
- Non-intrusive notifications
- Maintain context during updates

**4. Power User Friendly**
- Dense information layouts
- Keyboard shortcuts
- Quick actions and filters
- Efficient workflows

### Design Goals

- ✅ **Clarity**: Information is easy to understand at a glance
- ✅ **Efficiency**: Common tasks require minimal clicks
- ✅ **Consistency**: UI patterns are predictable and reusable
- ✅ **Scalability**: Design works with 1 camera or 100 cameras
- ✅ **Accessibility**: Usable by all, including those with disabilities
- ✅ **Responsiveness**: Adapts seamlessly to any screen size

## 🎨 Color System

### Primary Colors

```css
/* Primary - Green (Nature, Wildlife) */
--primary-50:  #e8f5e9;
--primary-100: #c8e6c9;
--primary-200: #a5d6a7;
--primary-300: #81c784;
--primary-400: #66bb6a;
--primary-500: #4caf50;  /* Main primary */
--primary-600: #43a047;
--primary-700: #388e3c;
--primary-800: #2e7d32;  /* Dark primary */
--primary-900: #1b5e20;
```

### Secondary Colors

```css
/* Secondary - Blue (Technology, Data) */
--secondary-50:  #e3f2fd;
--secondary-100: #bbdefb;
--secondary-200: #90caf9;
--secondary-300: #64b5f6;
--secondary-400: #42a5f5;
--secondary-500: #2196f3;  /* Main secondary */
--secondary-600: #1e88e5;
--secondary-700: #1976d2;
--secondary-800: #1565c0;
--secondary-900: #0d47a1;
```

### Status Colors

```css
/* Success - Green */
--success: #4caf50;
--success-light: #81c784;
--success-dark: #388e3c;

/* Warning - Orange */
--warning: #ff9800;
--warning-light: #ffb74d;
--warning-dark: #f57c00;

/* Error - Red */
--error: #f44336;
--error-light: #e57373;
--error-dark: #d32f2f;

/* Info - Blue */
--info: #2196f3;
--info-light: #64b5f6;
--info-dark: #1976d2;
```

### Neutral Colors (Light Mode)

```css
--gray-50:  #fafafa;
--gray-100: #f5f5f5;
--gray-200: #eeeeee;
--gray-300: #e0e0e0;
--gray-400: #bdbdbd;
--gray-500: #9e9e9e;
--gray-600: #757575;
--gray-700: #616161;
--gray-800: #424242;
--gray-900: #212121;

--text-primary: rgba(0, 0, 0, 0.87);
--text-secondary: rgba(0, 0, 0, 0.6);
--text-disabled: rgba(0, 0, 0, 0.38);
```

### Dark Mode Colors

```css
/* Dark mode backgrounds */
--dark-bg-default: #0a1929;
--dark-bg-paper: #132f4c;
--dark-bg-elevation-1: #1a3a52;
--dark-bg-elevation-2: #20445c;

/* Dark mode text */
--dark-text-primary: rgba(255, 255, 255, 0.87);
--dark-text-secondary: rgba(255, 255, 255, 0.6);
--dark-text-disabled: rgba(255, 255, 255, 0.38);

/* Dark mode borders */
--dark-divider: rgba(255, 255, 255, 0.12);
--dark-border: rgba(255, 255, 255, 0.23);
```

### Semantic Colors

```css
/* Battery Levels */
--battery-critical: #d32f2f;  /* < 20% */
--battery-low: #ff9800;       /* 20-40% */
--battery-medium: #fbc02d;    /* 40-60% */
--battery-good: #81c784;      /* 60-80% */
--battery-full: #4caf50;      /* > 80% */

/* Connection Status */
--status-online: #4caf50;
--status-offline: #757575;
--status-warning: #ff9800;
--status-error: #f44336;

/* Detection Confidence */
--confidence-high: #4caf50;   /* > 0.8 */
--confidence-medium: #ff9800; /* 0.5-0.8 */
--confidence-low: #f44336;    /* < 0.5 */
```

### Color Usage Guidelines

**Do's:**
- Use primary green for main actions and nature-related content
- Use secondary blue for technology and data visualization
- Reserve red for errors, critical alerts, and dangerous actions
- Use neutral grays for most text and backgrounds
- Maintain 4.5:1 contrast ratio for accessibility

**Don'ts:**
- Don't use red for non-critical information
- Don't mix too many colors in one view
- Don't use color as the only indicator (use icons/text too)
- Don't use pure black (#000) or white (#fff) in dark mode

## ✍️ Typography

### Font Family

```css
/* Primary font stack */
font-family: 'Roboto', 'Helvetica Neue', 'Arial', sans-serif;

/* Monospace for code/data */
font-family-mono: 'Roboto Mono', 'Courier New', monospace;
```

### Font Scales

```css
/* Display - Large headings */
--font-size-display-1: 96px;  /* h1 - Hero text */
--font-size-display-2: 60px;  /* h2 - Page titles */

/* Headings */
--font-size-h1: 48px;  /* 3rem */
--font-size-h2: 40px;  /* 2.5rem */
--font-size-h3: 34px;  /* 2.125rem */
--font-size-h4: 28px;  /* 1.75rem */
--font-size-h5: 24px;  /* 1.5rem */
--font-size-h6: 20px;  /* 1.25rem */

/* Body */
--font-size-body-1: 16px;   /* 1rem - Default body */
--font-size-body-2: 14px;   /* 0.875rem - Secondary text */
--font-size-caption: 12px;  /* 0.75rem - Captions, labels */
--font-size-small: 11px;    /* 0.6875rem - Fine print */
```

### Font Weights

```css
--font-weight-light: 300;
--font-weight-regular: 400;
--font-weight-medium: 500;
--font-weight-semibold: 600;
--font-weight-bold: 700;
```

### Line Heights

```css
--line-height-tight: 1.2;   /* Headings */
--line-height-normal: 1.5;  /* Body text */
--line-height-relaxed: 1.8; /* Long-form content */
```

### Typography Examples

```css
/* Page Title */
.page-title {
  font-size: 28px;
  font-weight: 600;
  line-height: 1.2;
  color: var(--text-primary);
  margin: 0 0 8px 0;
}

/* Section Heading */
.section-heading {
  font-size: 18px;
  font-weight: 600;
  line-height: 1.2;
  color: var(--text-primary);
  margin: 0 0 16px 0;
}

/* Body Text */
.body-text {
  font-size: 16px;
  font-weight: 400;
  line-height: 1.5;
  color: var(--text-primary);
}

/* Caption/Label */
.caption {
  font-size: 12px;
  font-weight: 400;
  line-height: 1.5;
  color: var(--text-secondary);
  text-transform: uppercase;
  letter-spacing: 0.5px;
}

/* Monospace Data */
.data-mono {
  font-family: var(--font-family-mono);
  font-size: 14px;
  font-weight: 400;
  line-height: 1.5;
}
```

## 🧩 Component Library

### Buttons

#### Primary Button
```jsx
<Button 
  variant="contained" 
  color="primary"
  size="medium"
>
  Primary Action
</Button>
```

**Usage**: Main call-to-action, primary operations
**Style**: Filled, elevated, primary color

#### Secondary Button
```jsx
<Button 
  variant="outlined" 
  color="primary"
  size="medium"
>
  Secondary Action
</Button>
```

**Usage**: Secondary actions, less emphasis
**Style**: Outlined, no fill, primary color border

#### Text Button
```jsx
<Button 
  variant="text" 
  color="primary"
  size="small"
>
  Cancel
</Button>
```

**Usage**: Tertiary actions, minimal emphasis
**Style**: No border, no fill, text only

### Cards

```jsx
<Card elevation={2}>
  <CardContent>
    <Typography variant="h6" gutterBottom>
      Card Title
    </Typography>
    <Typography variant="body2" color="text.secondary">
      Card content goes here
    </Typography>
  </CardContent>
  <CardActions>
    <Button size="small">Action</Button>
  </CardActions>
</Card>
```

**Usage**: Container for related information
**Variants**: Elevated (shadow), outlined (border)
**Best for**: Dashboard panels, grouped content

### Status Indicators

#### Status Chip
```jsx
<Chip 
  label="Online" 
  color="success" 
  size="small"
  icon={<CircleIcon />}
/>
```

**Colors**: success (green), warning (orange), error (red), default (gray)
**Usage**: Show status at a glance

#### Progress Indicators
```jsx
{/* Linear progress */}
<LinearProgress 
  variant="determinate" 
  value={75}
  color="primary"
/>

{/* Circular progress */}
<CircularProgress 
  variant="determinate" 
  value={75}
  size={40}
/>
```

**Usage**: Loading states, progress tracking

### Data Tables

```jsx
<TableContainer component={Paper}>
  <Table>
    <TableHead>
      <TableRow>
        <TableCell>Column 1</TableCell>
        <TableCell>Column 2</TableCell>
      </TableRow>
    </TableHead>
    <TableBody>
      {data.map((row) => (
        <TableRow key={row.id} hover>
          <TableCell>{row.col1}</TableCell>
          <TableCell>{row.col2}</TableCell>
        </TableRow>
      ))}
    </TableBody>
  </Table>
</TableContainer>
```

**Features**: Sortable headers, hover states, pagination
**Usage**: Display structured data

### Charts

#### Pie Chart
```jsx
<PieChart width={400} height={300}>
  <Pie 
    data={data} 
    dataKey="value"
    nameKey="name"
    cx="50%" 
    cy="50%"
  />
  <Tooltip />
  <Legend />
</PieChart>
```

**Usage**: Show proportions and distributions

#### Bar Chart
```jsx
<BarChart width={600} height={300} data={data}>
  <CartesianGrid strokeDasharray="3 3" />
  <XAxis dataKey="name" />
  <YAxis />
  <Tooltip />
  <Bar dataKey="value" fill="#4caf50" />
</BarChart>
```

**Usage**: Compare quantities across categories

#### Line Chart
```jsx
<LineChart width={600} height={300} data={data}>
  <CartesianGrid strokeDasharray="3 3" />
  <XAxis dataKey="time" />
  <YAxis />
  <Tooltip />
  <Line type="monotone" dataKey="value" stroke="#2196f3" />
</LineChart>
```

**Usage**: Show trends over time

### Forms

```jsx
<Box component="form" sx={{ p: 3 }}>
  <TextField
    fullWidth
    label="Camera Name"
    variant="outlined"
    margin="normal"
    required
  />
  
  <FormControl fullWidth margin="normal">
    <InputLabel>Photo Quality</InputLabel>
    <Select value={quality} onChange={handleChange}>
      <MenuItem value="low">Low</MenuItem>
      <MenuItem value="medium">Medium</MenuItem>
      <MenuItem value="high">High</MenuItem>
    </Select>
  </FormControl>
  
  <FormControlLabel
    control={<Switch checked={nightVision} />}
    label="Enable Night Vision"
  />
  
  <Button 
    type="submit" 
    variant="contained" 
    color="primary"
    sx={{ mt: 2 }}
  >
    Save Configuration
  </Button>
</Box>
```

## 📐 Layout & Grid

### Grid System

Material-UI uses a 12-column grid system:

```jsx
<Grid container spacing={3}>
  {/* 50/50 split */}
  <Grid item xs={12} md={6}>
    <Card>Left content</Card>
  </Grid>
  <Grid item xs={12} md={6}>
    <Card>Right content</Card>
  </Grid>
  
  {/* 2/3 - 1/3 split */}
  <Grid item xs={12} md={8}>
    <Card>Main content</Card>
  </Grid>
  <Grid item xs={12} md={4}>
    <Card>Sidebar</Card>
  </Grid>
</Grid>
```

### Spacing Scale

```css
--spacing-0: 0px;
--spacing-1: 4px;
--spacing-2: 8px;
--spacing-3: 12px;
--spacing-4: 16px;
--spacing-5: 20px;
--spacing-6: 24px;
--spacing-8: 32px;
--spacing-10: 40px;
--spacing-12: 48px;
--spacing-16: 64px;
```

**Usage**:
- `spacing-2` (8px): Tight spacing within components
- `spacing-3` (12px): Related elements
- `spacing-4` (16px): Default spacing between elements
- `spacing-6` (24px): Section spacing
- `spacing-8` (32px): Major section breaks

### Page Layouts

#### Dashboard Layout
```
┌──────────────────────────────────────┐
│ Navigation Drawer │ Main Content     │
│ (240px fixed)     │                  │
│                   │                  │
│ • Dashboard       │  Page Title      │
│ • Detections      │  ──────────────  │
│ • Analytics       │                  │
│ • Devices         │  Content Grid    │
│ • Map             │  [Card] [Card]   │
│ • Settings        │  [Card] [Card]   │
│                   │                  │
└──────────────────────────────────────┘
```

#### Full-Width Layout
```
┌──────────────────────────────────────┐
│ App Bar (64px)                       │
├──────────────────────────────────────┤
│                                      │
│  Full-Width Content                  │
│  (Map View, Gallery, etc.)           │
│                                      │
└──────────────────────────────────────┘
```

## 📱 Responsive Design

### Breakpoints

```javascript
const breakpoints = {
  xs: 0,      // Mobile (portrait)
  sm: 600,    // Mobile (landscape) / Small tablet
  md: 900,    // Tablet
  lg: 1200,   // Desktop
  xl: 1536,   // Large desktop
};
```

### Responsive Patterns

#### Stack on Mobile
```jsx
<Grid container spacing={3}>
  <Grid item xs={12} md={6}>
    {/* Full width on mobile, half on desktop */}
  </Grid>
  <Grid item xs={12} md={6}>
    {/* Full width on mobile, half on desktop */}
  </Grid>
</Grid>
```

#### Hide on Small Screens
```jsx
<Box sx={{ display: { xs: 'none', md: 'block' } }}>
  {/* Hidden on mobile, visible on desktop */}
</Box>
```

#### Drawer Behavior
```jsx
<Drawer
  variant="permanent"     // Desktop
  sx={{ 
    display: { xs: 'none', md: 'block' }  // Hidden on mobile
  }}
/>

<Drawer
  variant="temporary"     // Mobile
  open={mobileOpen}
  onClose={handleDrawerToggle}
  sx={{ 
    display: { xs: 'block', md: 'none' }  // Visible only on mobile
  }}
/>
```

### Mobile Optimizations

- **Touch Targets**: Minimum 44x44px for interactive elements
- **Simplified Navigation**: Bottom navigation on mobile
- **Reduced Density**: More spacing on small screens
- **Swipe Gestures**: Support swipe for navigation
- **Optimized Images**: Smaller images for mobile

## 🖼️ Icons & Imagery

### Icon Library

Using Material-UI Icons:

```jsx
import {
  Dashboard,
  PhotoCamera,
  Analytics,
  Settings,
  DeviceHub,
  Map,
  Timeline,
  Assessment,
  TrendingUp,
  Battery80,
  SignalWifi4Bar,
  Warning,
  CheckCircle,
  Error,
} from '@mui/icons-material';
```

### Icon Sizes

```jsx
<Icon fontSize="small" />    {/* 20px */}
<Icon fontSize="medium" />   {/* 24px - Default */}
<Icon fontSize="large" />    {/* 36px */}
```

### Icon Usage

- **Navigation**: Use recognizable icons for menu items
- **Actions**: Icons for buttons (with labels when possible)
- **Status**: Icons to reinforce status (✓, ⚠, ✗)
- **Data Points**: Icons for metrics (battery, signal, etc.)

### Image Guidelines

**Detection Images**:
- Aspect ratio: 16:9 or 4:3
- Thumbnail: 200x150px
- Full size: Max 1920x1080px
- Format: WebP (fallback to JPEG)
- Compression: Balance quality and size

**Placeholders**:
- Use skeleton loaders for loading states
- Gray placeholder for missing images
- Icon placeholder for empty states

## ⚡ Interactions

### Hover States

```css
/* Card hover */
.card:hover {
  box-shadow: 0 8px 16px rgba(0,0,0,0.15);
  transform: translateY(-2px);
  transition: all 0.2s ease;
}

/* Button hover */
.button:hover {
  background-color: rgba(46, 125, 50, 0.04);
  transition: background-color 0.2s ease;
}
```

### Loading States

```jsx
{/* Skeleton loading */}
<Skeleton variant="rectangular" width="100%" height={200} />
<Skeleton variant="text" />
<Skeleton variant="circular" width={40} height={40} />

{/* Spinner loading */}
<CircularProgress size={24} />
```

### Transitions

```jsx
<Fade in={show} timeout={300}>
  <Box>Fading content</Box>
</Fade>

<Slide direction="up" in={show}>
  <Box>Sliding content</Box>
</Slide>

<Grow in={show}>
  <Box>Growing content</Box>
</Grow>
```

### Toast Notifications

```jsx
<Snackbar
  open={open}
  autoHideDuration={6000}
  onClose={handleClose}
  message="Detection uploaded successfully"
  action={
    <Button color="inherit" onClick={handleClose}>
      UNDO
    </Button>
  }
/>
```

## ♿ Accessibility

### WCAG 2.1 Compliance

**Level AA Requirements**:
- ✅ Color contrast ratio: 4.5:1 for normal text
- ✅ Color contrast ratio: 3:1 for large text (18pt+)
- ✅ Keyboard navigation for all interactive elements
- ✅ Focus indicators visible and clear
- ✅ Alt text for all images
- ✅ ARIA labels for icon-only buttons
- ✅ Semantic HTML structure

### Keyboard Navigation

```jsx
<Button
  onClick={handleClick}
  onKeyPress={(e) => {
    if (e.key === 'Enter' || e.key === ' ') {
      handleClick();
    }
  }}
  aria-label="Delete detection"
>
  <DeleteIcon />
</Button>
```

### Screen Reader Support

```jsx
<Box 
  role="status" 
  aria-live="polite"
  aria-atomic="true"
>
  {statusMessage}
</Box>

<IconButton
  aria-label="refresh dashboard"
  onClick={handleRefresh}
>
  <RefreshIcon />
</IconButton>
```

### Focus Management

```jsx
// Focus on modal open
useEffect(() => {
  if (open) {
    dialogRef.current?.focus();
  }
}, [open]);

// Return focus on close
const handleClose = () => {
  setOpen(false);
  triggerRef.current?.focus();
};
```

## 🌙 Dark Mode

### Implementation

```jsx
const theme = createTheme({
  palette: {
    mode: isDarkMode ? 'dark' : 'light',
    primary: {
      main: '#4caf50',
    },
    background: {
      default: isDarkMode ? '#0a1929' : '#ffffff',
      paper: isDarkMode ? '#132f4c' : '#f5f5f5',
    },
  },
});
```

### Dark Mode Guidelines

**Do's**:
- Use dark backgrounds (#0a1929, #132f4c)
- Increase elevation with lighter backgrounds
- Reduce text opacity (87% for primary, 60% for secondary)
- Use subtle shadows for depth
- Test all colors for sufficient contrast

**Don'ts**:
- Don't use pure black (#000000)
- Don't invert colors directly (breaks semantic meaning)
- Don't reduce contrast below WCAG standards
- Don't make charts hard to read

### Theme Toggle

```jsx
<IconButton 
  onClick={toggleTheme}
  aria-label="toggle theme"
>
  {isDarkMode ? <LightModeIcon /> : <DarkModeIcon />}
</IconButton>
```

## 📊 Data Visualization Best Practices

### Chart Colors

Use a consistent, accessible palette:

```javascript
const chartColors = [
  '#4caf50',  // Green
  '#2196f3',  // Blue
  '#ff9800',  // Orange
  '#9c27b0',  // Purple
  '#f44336',  // Red
  '#00bcd4',  // Cyan
  '#ffeb3b',  // Yellow
  '#795548',  // Brown
];
```

### Chart Guidelines

- **Labels**: Always label axes and provide units
- **Legend**: Include legend when multiple series
- **Tooltips**: Show detailed info on hover
- **Responsive**: Charts scale with container
- **Animation**: Subtle, not distracting
- **Loading**: Show skeleton while loading data
- **Empty State**: Clear message when no data

## 🎭 Motion Design

### Animation Timing

```css
--duration-shortest: 150ms;  /* Micro-interactions */
--duration-shorter: 200ms;   /* Small components */
--duration-short: 250ms;     /* Default */
--duration-standard: 300ms;  /* Medium components */
--duration-complex: 375ms;   /* Complex transitions */
--duration-entering: 225ms;  /* Entering screen */
--duration-leaving: 195ms;   /* Leaving screen */
```

### Easing Functions

```css
--easing-ease-in: cubic-bezier(0.4, 0, 1, 1);
--easing-ease-out: cubic-bezier(0, 0, 0.2, 1);
--easing-ease-in-out: cubic-bezier(0.4, 0, 0.2, 1);
--easing-sharp: cubic-bezier(0.4, 0, 0.6, 1);
```

### Animation Principles

- **Purpose**: Every animation serves a purpose
- **Subtle**: Animations should feel natural, not flashy
- **Performance**: Use transform and opacity for smooth 60fps
- **Respect Preferences**: Honor `prefers-reduced-motion`

```css
@media (prefers-reduced-motion: reduce) {
  * {
    animation-duration: 0.01ms !important;
    animation-iteration-count: 1 !important;
    transition-duration: 0.01ms !important;
  }
}
```

---

## 📚 Additional Resources

### Design Tools

- [Material-UI Documentation](https://mui.com/)
- [Figma Design Kit](https://www.figma.com/community/file/912837788133317724)
- [Material Design Guidelines](https://material.io/design)
- [Recharts Documentation](https://recharts.org/)

### Related Guides

- [Component Documentation](./README.md#-features)
- [Accessibility Guide](./ACCESSIBILITY.md)
- [Customization Examples](./README.md#-customization)

---

**Design System Version**: 3.0.0  
**Last Updated**: 2025-10-14  
**Maintained by**: WildCAM ESP32 Team
