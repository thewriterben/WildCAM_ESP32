# Collaboration Features - Quick Setup Guide

This guide will help you set up and start using the collaboration features in WildCAM ESP32.

## Prerequisites

- PostgreSQL database running
- Python 3.8+ installed
- Node.js 16+ and npm installed
- Existing WildCAM backend and frontend

## Step-by-Step Setup

### 1. Database Migration

Run the SQL migration to add collaboration tables:

```bash
# Connect to your PostgreSQL database
psql -U wildlife_user -d wildlife_db

# Or if using environment variable
psql $DATABASE_URL

# Run the migration script
\i backend/migrations/add_collaboration_tables.sql

# Verify tables were created
\dt

# You should see:
# - detection_comments
# - bookmarks
# - chat_messages

# Exit psql
\q
```

### 2. Backend Setup

No additional Python packages needed - all dependencies are already in `requirements.txt`.

If you need to reinstall:

```bash
cd backend
pip install -r requirements.txt
```

### 3. Frontend Setup

No additional npm packages needed - all dependencies are already in `package.json`.

If you need to reinstall:

```bash
cd frontend/dashboard
npm install
```

### 4. Environment Variables

Ensure your `.env` file has:

```bash
# Database
DATABASE_URL=postgresql://wildlife_user:password@localhost:5432/wildlife_db

# JWT Secret
JWT_SECRET_KEY=your-secret-key-change-in-production

# Optional: Redis for future scaling
REDIS_URL=redis://localhost:6379
```

### 5. Start the Application

**Terminal 1 - Backend:**
```bash
cd backend
python app.py
```

**Terminal 2 - Frontend:**
```bash
cd frontend/dashboard
npm start
```

The application should open at `http://localhost:3000`

## First-Time Usage

### 1. Create User Accounts

Create test users with different roles:

```bash
curl -X POST http://localhost:5000/api/auth/register \
  -H "Content-Type: application/json" \
  -d '{
    "username": "admin_user",
    "email": "admin@example.com",
    "password": "SecurePass123",
    "role": "admin",
    "first_name": "Admin",
    "last_name": "User"
  }'

curl -X POST http://localhost:5000/api/auth/register \
  -H "Content-Type: application/json" \
  -d '{
    "username": "researcher1",
    "email": "researcher@example.com",
    "password": "SecurePass123",
    "role": "researcher",
    "first_name": "Research",
    "last_name": "Scientist"
  }'
```

### 2. Login and Test

1. Open `http://localhost:3000` in your browser
2. Login with one of the users you created
3. You should see:
   - User presence indicator in the top-right (showing 1 online)
   - "Team Chat" option in the sidebar menu

### 3. Test Multi-User Collaboration

**Open Multiple Browser Windows:**

1. Window 1: Login as admin_user
2. Window 2: Login as researcher1 (use incognito/private browsing)

**Test Real-time Chat:**

1. Both users navigate to "Team Chat"
2. Send a message from Window 1
3. Message should appear instantly in Window 2
4. Both users should see "2 online" in the presence indicator

**Test Comments on Detections:**

1. Navigate to a wildlife detection
2. Add a comment from Window 1
3. Comment should appear in Window 2 instantly

**Test Shared Bookmarks:**

1. Create a shared bookmark from Window 1
2. Bookmark should appear in Window 2's bookmark list

## Troubleshooting

### WebSocket Not Connecting

**Problem:** Chat messages not appearing in real-time

**Solution:**
1. Check browser console for errors (F12)
2. Verify backend is running on port 5000
3. Check WebSocket connection status in browser:
   ```javascript
   // In browser console
   localStorage.getItem('access_token')  // Should show your JWT
   ```
4. Restart both backend and frontend

### JWT Token Expired

**Problem:** "Unauthorized" errors

**Solution:**
1. Logout and login again
2. Token expires after 24 hours by default
3. Check token in browser console:
   ```javascript
   localStorage.clear()  // Clear tokens
   // Then login again
   ```

### Database Connection Failed

**Problem:** "Could not connect to database"

**Solution:**
1. Verify PostgreSQL is running:
   ```bash
   sudo systemctl status postgresql
   ```
2. Check database credentials in `.env`
3. Test connection:
   ```bash
   psql $DATABASE_URL -c "SELECT 1"
   ```

### Migration Already Applied

**Problem:** "Table already exists" error

**Solution:**
Migration can be run multiple times safely. The SQL uses `CREATE TABLE IF NOT EXISTS`.

To check if tables exist:
```bash
psql $DATABASE_URL -c "\dt" | grep -E "detection_comments|bookmarks|chat_messages"
```

## Testing the Features

### Test Checklist

- [ ] Can register new users
- [ ] Can login successfully
- [ ] See user presence indicator (should show 1 online)
- [ ] Can access Team Chat page
- [ ] Can send chat messages
- [ ] Chat messages appear in real-time
- [ ] Open second browser window and login
- [ ] See 2 users online in presence indicator
- [ ] Messages from one window appear in other
- [ ] Can add comments to wildlife detections
- [ ] Comments sync in real-time
- [ ] Can create bookmarks
- [ ] Can share bookmarks with team
- [ ] Shared bookmarks visible to all users

## API Testing with curl

### Get Active Users
```bash
curl http://localhost:5000/api/collaboration/presence \
  -H "Authorization: Bearer YOUR_ACCESS_TOKEN"
```

### Send Chat Message
```bash
curl -X POST http://localhost:5000/api/collaboration/chat \
  -H "Content-Type: application/json" \
  -H "Authorization: Bearer YOUR_ACCESS_TOKEN" \
  -d '{"message": "Hello team!"}'
```

### Add Comment to Detection
```bash
curl -X POST http://localhost:5000/api/collaboration/comments \
  -H "Content-Type: application/json" \
  -H "Authorization: Bearer YOUR_ACCESS_TOKEN" \
  -d '{"detection_id": 1, "comment": "Great sighting!"}'
```

### Create Shared Bookmark
```bash
curl -X POST http://localhost:5000/api/collaboration/bookmarks \
  -H "Content-Type: application/json" \
  -H "Authorization: Bearer YOUR_ACCESS_TOKEN" \
  -d '{
    "detection_id": 1, 
    "title": "Rare Species",
    "description": "First wolf sighting in 10 years",
    "shared": true
  }'
```

## Performance Tips

### For Better Performance:

1. **Add Database Indexes** (already included in migration):
   - Comments indexed by detection_id
   - Bookmarks indexed by shared flag
   - Chat messages indexed by timestamp

2. **Limit Message History**:
   - Default: Last 50 messages
   - Adjust in `TeamChat.js` if needed

3. **Presence Refresh Rate**:
   - Default: Every 30 seconds
   - Adjust in `UserPresence.js` if needed

4. **WebSocket Connection**:
   - Automatically reconnects on disconnect
   - Max 5 reconnection attempts

## Security Best Practices

1. **Change Default JWT Secret:**
   ```bash
   # In .env
   JWT_SECRET_KEY=$(openssl rand -base64 32)
   ```

2. **Use HTTPS in Production:**
   - Configure SSL/TLS certificates
   - Update WebSocket URL to `wss://`

3. **Set Strong Password Requirements:**
   - Minimum 8 characters
   - At least 1 uppercase, 1 lowercase, 1 number
   - Already enforced in auth.py

4. **Regular Token Rotation:**
   - Access tokens expire after 24 hours
   - Refresh tokens expire after 30 days
   - Users automatically logged out after expiration

## Next Steps

1. ✅ Setup complete - collaboration features working
2. Test with your team members
3. Customize UI components to match your theme
4. Add more WebSocket events as needed
5. Monitor performance and scale as needed

## Need Help?

- Check `COLLABORATION_FEATURES.md` for feature documentation
- Check `COLLABORATION_ARCHITECTURE.md` for technical details
- Run tests: `python tests/test_collaboration_models.py`
- Check logs in browser console (F12)
- Check backend logs in terminal

## Common Use Cases

### Research Team Collaboration

1. Researcher spots interesting detection
2. Creates shared bookmark with title "Gray Wolf - Rare Sighting"
3. Adds comment with field notes
4. Sends chat message to team: "Check bookmark - wolf at Camera 5!"
5. Team members instantly see notification
6. Discuss in real-time via chat
7. Add additional comments and observations

### Field Work Coordination

1. Field worker updates camera location
2. Sends chat: "Camera 3 battery replaced, relocated 100m north"
3. Admin acknowledges via chat
4. Updates reflected in camera management
5. Team stays synchronized on field activities

### Species Identification

1. Uncertain detection appears
2. Researcher adds comment: "Bear or large dog? Need expert opinion"
3. Expert reviews and comments: "Confirmed - Black bear, juvenile"
4. Original researcher updates verification
5. Creates shared bookmark for reference
6. Full discussion thread preserved for future reference

Enjoy collaborating with your team on wildlife monitoring! 🦌
