/**
 * RBACExample Component
 * 
 * Demonstrates how to use the RBAC hook in React components
 * to conditionally render UI based on user permissions.
 */

import React from 'react';
import {
  Box,
  Paper,
  Typography,
  Button,
  Chip,
  Alert,
  CircularProgress,
  Grid,
  Card,
  CardContent,
  CardActions,
  Divider
} from '@mui/material';
import {
  Lock,
  LockOpen,
  Person,
  AdminPanelSettings,
  Visibility,
  Edit,
  Delete,
  Add
} from '@mui/icons-material';

import useRBAC, { Permissions, Roles, RoleLevels } from '../hooks/useRBAC';

/**
 * Example component showing RBAC integration
 */
const RBACExample = () => {
  const {
    rbacInfo,
    loading,
    error,
    hasPermission,
    hasAnyPermission,
    hasRole,
    hasMinimumRoleLevel,
    isAdmin,
    role,
    roleLevel,
    permissions
  } = useRBAC();

  if (loading) {
    return (
      <Box sx={{ display: 'flex', justifyContent: 'center', p: 4 }}>
        <CircularProgress />
      </Box>
    );
  }

  if (error) {
    return (
      <Alert severity="error" sx={{ m: 2 }}>
        Error loading RBAC information: {error}
      </Alert>
    );
  }

  return (
    <Box sx={{ p: 3 }}>
      <Typography variant="h4" gutterBottom>
        RBAC Integration Example
      </Typography>

      {/* User Role Display */}
      <Paper sx={{ p: 3, mb: 3 }}>
        <Box sx={{ display: 'flex', alignItems: 'center', gap: 2, mb: 2 }}>
          {isAdmin ? (
            <AdminPanelSettings color="error" fontSize="large" />
          ) : (
            <Person color="primary" fontSize="large" />
          )}
          <div>
            <Typography variant="h5">
              Current Role: {role}
            </Typography>
            <Typography variant="body2" color="text.secondary">
              Role Level: {roleLevel} / 5
            </Typography>
          </div>
          {isAdmin && (
            <Chip label="Administrator" color="error" />
          )}
        </Box>

        <Divider sx={{ my: 2 }} />

        <Typography variant="subtitle2" gutterBottom>
          Your Permissions ({permissions.length}):
        </Typography>
        <Box sx={{ display: 'flex', flexWrap: 'wrap', gap: 1, mt: 1 }}>
          {permissions.slice(0, 10).map((perm) => (
            <Chip
              key={perm}
              label={perm.replace(/_/g, ' ')}
              size="small"
              variant="outlined"
            />
          ))}
          {permissions.length > 10 && (
            <Chip
              label={`+${permissions.length - 10} more`}
              size="small"
              variant="outlined"
              color="primary"
            />
          )}
        </Box>
      </Paper>

      {/* Permission-based UI Examples */}
      <Typography variant="h6" gutterBottom>
        Permission-Based UI Components
      </Typography>

      <Grid container spacing={3}>
        {/* View Detection Card */}
        <Grid item xs={12} md={6} lg={4}>
          <Card>
            <CardContent>
              <Box sx={{ display: 'flex', alignItems: 'center', gap: 1, mb: 2 }}>
                <Visibility color="primary" />
                <Typography variant="h6">View Detection</Typography>
              </Box>
              <Typography variant="body2" color="text.secondary">
                Basic viewing permission available to all users.
              </Typography>
            </CardContent>
            <CardActions>
              {hasPermission(Permissions.VIEW_DETECTIONS) ? (
                <Button
                  size="small"
                  startIcon={<LockOpen />}
                  color="success"
                >
                  Available
                </Button>
              ) : (
                <Button
                  size="small"
                  startIcon={<Lock />}
                  disabled
                >
                  Locked
                </Button>
              )}
            </CardActions>
          </Card>
        </Grid>

        {/* Create Annotation Card */}
        <Grid item xs={12} md={6} lg={4}>
          <Card>
            <CardContent>
              <Box sx={{ display: 'flex', alignItems: 'center', gap: 1, mb: 2 }}>
                <Add color="primary" />
                <Typography variant="h6">Create Annotation</Typography>
              </Box>
              <Typography variant="body2" color="text.secondary">
                Available to Observers and above.
              </Typography>
            </CardContent>
            <CardActions>
              {hasPermission(Permissions.CREATE_ANNOTATIONS) ? (
                <Button
                  size="small"
                  startIcon={<LockOpen />}
                  color="success"
                  onClick={() => alert('Creating annotation...')}
                >
                  Create
                </Button>
              ) : (
                <Button
                  size="small"
                  startIcon={<Lock />}
                  disabled
                >
                  Requires Observer Role
                </Button>
              )}
            </CardActions>
          </Card>
        </Grid>

        {/* Edit Annotation Card */}
        <Grid item xs={12} md={6} lg={4}>
          <Card>
            <CardContent>
              <Box sx={{ display: 'flex', alignItems: 'center', gap: 1, mb: 2 }}>
                <Edit color="primary" />
                <Typography variant="h6">Edit Annotation</Typography>
              </Box>
              <Typography variant="body2" color="text.secondary">
                Available to Field Workers and above.
              </Typography>
            </CardContent>
            <CardActions>
              {hasPermission(Permissions.EDIT_ANNOTATIONS) ? (
                <Button
                  size="small"
                  startIcon={<LockOpen />}
                  color="success"
                  onClick={() => alert('Editing annotation...')}
                >
                  Edit
                </Button>
              ) : (
                <Button
                  size="small"
                  startIcon={<Lock />}
                  disabled
                >
                  Requires Field Worker Role
                </Button>
              )}
            </CardActions>
          </Card>
        </Grid>

        {/* Delete Annotation Card */}
        <Grid item xs={12} md={6} lg={4}>
          <Card>
            <CardContent>
              <Box sx={{ display: 'flex', alignItems: 'center', gap: 1, mb: 2 }}>
                <Delete color="error" />
                <Typography variant="h6">Delete Annotation</Typography>
              </Box>
              <Typography variant="body2" color="text.secondary">
                Available to Researchers and above.
              </Typography>
            </CardContent>
            <CardActions>
              {hasPermission(Permissions.DELETE_ANNOTATIONS) ? (
                <Button
                  size="small"
                  startIcon={<LockOpen />}
                  color="error"
                  onClick={() => {
                    if (window.confirm('Delete annotation?')) {
                      alert('Deleting annotation...');
                    }
                  }}
                >
                  Delete
                </Button>
              ) : (
                <Button
                  size="small"
                  startIcon={<Lock />}
                  disabled
                >
                  Requires Researcher Role
                </Button>
              )}
            </CardActions>
          </Card>
        </Grid>

        {/* Export Data Card */}
        <Grid item xs={12} md={6} lg={4}>
          <Card>
            <CardContent>
              <Box sx={{ display: 'flex', alignItems: 'center', gap: 1, mb: 2 }}>
                <AdminPanelSettings color="primary" />
                <Typography variant="h6">Export Data</Typography>
              </Box>
              <Typography variant="body2" color="text.secondary">
                Available to Researchers and above.
              </Typography>
            </CardContent>
            <CardActions>
              {hasPermission(Permissions.EXPORT_DATA) ? (
                <Button
                  size="small"
                  startIcon={<LockOpen />}
                  color="primary"
                  onClick={() => alert('Exporting data...')}
                >
                  Export
                </Button>
              ) : (
                <Button
                  size="small"
                  startIcon={<Lock />}
                  disabled
                >
                  Requires Researcher Role
                </Button>
              )}
            </CardActions>
          </Card>
        </Grid>

        {/* Admin Panel Card */}
        <Grid item xs={12} md={6} lg={4}>
          <Card>
            <CardContent>
              <Box sx={{ display: 'flex', alignItems: 'center', gap: 1, mb: 2 }}>
                <AdminPanelSettings color="error" />
                <Typography variant="h6">Admin Panel</Typography>
              </Box>
              <Typography variant="body2" color="text.secondary">
                Only available to Administrators.
              </Typography>
            </CardContent>
            <CardActions>
              {isAdmin ? (
                <Button
                  size="small"
                  startIcon={<LockOpen />}
                  color="error"
                  onClick={() => alert('Opening admin panel...')}
                >
                  Access
                </Button>
              ) : (
                <Button
                  size="small"
                  startIcon={<Lock />}
                  disabled
                >
                  Admin Only
                </Button>
              )}
            </CardActions>
          </Card>
        </Grid>
      </Grid>

      {/* Role-based Alert */}
      <Box sx={{ mt: 3 }}>
        {roleLevel < RoleLevels.RESEARCHER ? (
          <Alert severity="info">
            You are currently a {role}. Contact an administrator to request
            additional permissions.
          </Alert>
        ) : (
          <Alert severity="success">
            You have {role} level access with {permissions.length} permissions.
          </Alert>
        )}
      </Box>

      {/* Code Example */}
      <Paper sx={{ p: 3, mt: 3, bgcolor: 'grey.50' }}>
        <Typography variant="h6" gutterBottom>
          Usage Example
        </Typography>
        <Typography variant="body2" component="pre" sx={{ fontFamily: 'monospace', fontSize: '0.875rem' }}>
{`import useRBAC, { Permissions } from '../hooks/useRBAC';

function MyComponent() {
  const { hasPermission, isAdmin, role } = useRBAC();

  return (
    <div>
      <h1>Welcome, {role}</h1>
      
      {hasPermission(Permissions.CREATE_ANNOTATIONS) && (
        <Button onClick={createAnnotation}>
          Create Annotation
        </Button>
      )}
      
      {isAdmin && (
        <AdminPanel />
      )}
    </div>
  );
}`}
        </Typography>
      </Paper>
    </Box>
  );
};

export default RBACExample;
