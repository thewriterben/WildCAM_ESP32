/**
 * Ocean Theme - Cool blue color scheme
 * 
 * A calming, professional theme with ocean-inspired colors
 * Perfect for coastal or marine wildlife monitoring
 */

import { createTheme } from '@mui/material/styles';

const oceanTheme = createTheme({
  palette: {
    mode: 'dark',
    
    primary: {
      main: '#006994',      // Deep ocean blue
      light: '#0086b3',     // Light sea blue
      dark: '#004d6b',      // Navy blue
      contrastText: '#ffffff',
    },
    
    secondary: {
      main: '#0097a7',      // Teal
      light: '#00acc1',     // Light teal
      dark: '#00838f',      // Dark cyan
      contrastText: '#ffffff',
    },
    
    success: {
      main: '#00897b',      // Sea green
      light: '#26a69a',
      dark: '#00695c',
    },
    
    warning: {
      main: '#ffa726',      // Coral orange
      light: '#ffb74d',
      dark: '#f57c00',
    },
    
    error: {
      main: '#ef5350',      // Coral red
      light: '#f44336',
      dark: '#c62828',
    },
    
    info: {
      main: '#29b6f6',      // Sky blue
      light: '#4fc3f7',
      dark: '#0288d1',
    },
    
    background: {
      default: '#0a1929',   // Deep ocean
      paper: '#0d2d42',     // Ocean depth
    },
    
    text: {
      primary: '#e3f2fd',
      secondary: '#90caf9',
    },
  },
  
  typography: {
    fontFamily: '"Inter", "Roboto", sans-serif',
    
    h1: {
      fontWeight: 700,
      letterSpacing: '-0.03em',
    },
    
    button: {
      fontWeight: 500,
      textTransform: 'uppercase',
      letterSpacing: '0.5px',
    },
  },
  
  shape: {
    borderRadius: 12,
  },
  
  components: {
    MuiCard: {
      styleOverrides: {
        root: {
          backgroundImage: 'linear-gradient(to bottom right, rgba(0, 105, 148, 0.1), rgba(0, 151, 167, 0.05))',
          backdropFilter: 'blur(10px)',
          border: '1px solid rgba(255, 255, 255, 0.1)',
        },
      },
    },
    
    MuiButton: {
      styleOverrides: {
        root: {
          borderRadius: 20,
        },
        contained: {
          background: 'linear-gradient(45deg, #006994 30%, #0097a7 90%)',
          boxShadow: '0 3px 5px 2px rgba(0, 105, 148, .3)',
        },
      },
    },
    
    MuiAppBar: {
      styleOverrides: {
        root: {
          background: 'linear-gradient(90deg, #0a1929 0%, #0d2d42 100%)',
          backdropFilter: 'blur(10px)',
        },
      },
    },
  },
});

export default oceanTheme;

/**
 * Usage:
 * 
 * import { ThemeProvider } from '@mui/material/styles';
 * import oceanTheme from './examples/themes/ocean-theme';
 * 
 * function App() {
 *   return (
 *     <ThemeProvider theme={oceanTheme}>
 *       <YourApp />
 *     </ThemeProvider>
 *   );
 * }
 */
