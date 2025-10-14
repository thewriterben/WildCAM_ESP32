/**
 * Desert Theme - Warm sandy color scheme
 * 
 * A warm, earthy theme inspired by desert landscapes
 * Ideal for arid region wildlife monitoring
 */

import { createTheme } from '@mui/material/styles';

const desertTheme = createTheme({
  palette: {
    mode: 'light',
    
    primary: {
      main: '#d97706',      // Amber
      light: '#f59e0b',     // Light amber
      dark: '#b45309',      // Dark amber
      contrastText: '#ffffff',
    },
    
    secondary: {
      main: '#78350f',      // Brown
      light: '#92400e',     // Light brown
      dark: '#451a03',      // Dark brown
      contrastText: '#ffffff',
    },
    
    success: {
      main: '#84cc16',      // Desert sage
      light: '#a3e635',
      dark: '#65a30d',
    },
    
    warning: {
      main: '#f97316',      // Sunset orange
      light: '#fb923c',
      dark: '#ea580c',
    },
    
    error: {
      main: '#dc2626',      // Desert red
      light: '#ef4444',
      dark: '#b91c1c',
    },
    
    info: {
      main: '#0ea5e9',      // Sky blue
      light: '#38bdf8',
      dark: '#0284c7',
    },
    
    background: {
      default: '#fef3c7',   // Sand
      paper: '#fde68a',     // Light sand
    },
    
    text: {
      primary: '#292524',
      secondary: '#57534e',
    },
  },
  
  typography: {
    fontFamily: '"Merriweather", "Georgia", serif',
    
    h1: {
      fontWeight: 700,
      color: '#78350f',
    },
    
    h2: {
      fontWeight: 600,
      color: '#78350f',
    },
    
    button: {
      fontWeight: 700,
      textTransform: 'uppercase',
      fontFamily: '"Roboto", sans-serif',
    },
  },
  
  shape: {
    borderRadius: 8,
  },
  
  components: {
    MuiCard: {
      styleOverrides: {
        root: {
          background: 'linear-gradient(135deg, #fef3c7 0%, #fde68a 100%)',
          border: '2px solid #d97706',
          boxShadow: '0 4px 6px rgba(217, 119, 6, 0.1)',
        },
      },
    },
    
    MuiButton: {
      styleOverrides: {
        root: {
          borderRadius: 6,
          fontWeight: 700,
        },
        contained: {
          background: 'linear-gradient(45deg, #d97706 30%, #f59e0b 90%)',
          boxShadow: '0 3px 5px 2px rgba(217, 119, 6, .3)',
          '&:hover': {
            background: 'linear-gradient(45deg, #b45309 30%, #d97706 90%)',
          },
        },
      },
    },
    
    MuiChip: {
      styleOverrides: {
        root: {
          fontWeight: 600,
          borderRadius: 16,
        },
      },
    },
    
    MuiAppBar: {
      styleOverrides: {
        root: {
          background: 'linear-gradient(90deg, #78350f 0%, #92400e 100%)',
          boxShadow: '0 2px 4px rgba(120, 53, 15, 0.2)',
        },
      },
    },
  },
});

export default desertTheme;

/**
 * Usage:
 * 
 * import { ThemeProvider } from '@mui/material/styles';
 * import desertTheme from './examples/themes/desert-theme';
 * 
 * function App() {
 *   return (
 *     <ThemeProvider theme={desertTheme}>
 *       <YourApp />
 *     </ThemeProvider>
 *   );
 * }
 */
