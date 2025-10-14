/**
 * Forest Theme - Nature-inspired color scheme
 * 
 * A warm, earthy theme perfect for wildlife monitoring
 * Features deep greens, browns, and natural tones
 */

import { createTheme } from '@mui/material/styles';

const forestTheme = createTheme({
  palette: {
    mode: 'dark',
    
    primary: {
      main: '#2d5016',      // Deep forest green
      light: '#4a7729',     // Moss green
      dark: '#1a3309',      // Dark pine
      contrastText: '#ffffff',
    },
    
    secondary: {
      main: '#8b4513',      // Saddle brown
      light: '#a0522d',     // Sienna
      dark: '#5c2e0a',      // Dark brown
      contrastText: '#ffffff',
    },
    
    success: {
      main: '#558b2f',      // Leaf green
      light: '#7cb342',
      dark: '#33691e',
    },
    
    warning: {
      main: '#f57c00',      // Autumn orange
      light: '#ff9800',
      dark: '#e65100',
    },
    
    error: {
      main: '#c62828',      // Berry red
      light: '#d32f2f',
      dark: '#b71c1c',
    },
    
    info: {
      main: '#0277bd',      // Sky blue
      light: '#03a9f4',
      dark: '#01579b',
    },
    
    background: {
      default: '#1a1a1a',   // Rich black
      paper: '#2d2d2d',     // Charcoal
    },
    
    text: {
      primary: '#e8e8e8',
      secondary: '#b8b8b8',
    },
  },
  
  typography: {
    fontFamily: '"Roboto", "Open Sans", sans-serif',
    
    h1: {
      fontWeight: 700,
      letterSpacing: '-0.02em',
    },
    
    h2: {
      fontWeight: 600,
      letterSpacing: '-0.01em',
    },
    
    button: {
      fontWeight: 600,
      textTransform: 'none',
    },
  },
  
  components: {
    MuiCard: {
      styleOverrides: {
        root: {
          backgroundImage: 'linear-gradient(rgba(255, 255, 255, 0.05), rgba(255, 255, 255, 0.05))',
          borderRadius: 12,
        },
      },
    },
    
    MuiChip: {
      styleOverrides: {
        root: {
          fontWeight: 600,
        },
      },
    },
    
    MuiButton: {
      styleOverrides: {
        root: {
          borderRadius: 8,
        },
        contained: {
          boxShadow: '0 4px 6px rgba(0, 0, 0, 0.1)',
          '&:hover': {
            boxShadow: '0 6px 10px rgba(0, 0, 0, 0.15)',
          },
        },
      },
    },
  },
});

export default forestTheme;

/**
 * Usage:
 * 
 * import { ThemeProvider } from '@mui/material/styles';
 * import forestTheme from './examples/themes/forest-theme';
 * 
 * function App() {
 *   return (
 *     <ThemeProvider theme={forestTheme}>
 *       <YourApp />
 *     </ThemeProvider>
 *   );
 * }
 */
