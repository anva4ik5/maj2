# Deployment Guide

## Railway Deployment (Backend)

### Files to push to Railway:
```
backend/
├── package.json
├── server.js
├── .env.example
└── README.md
```

### Steps:

1. **Create Railway project:**
   - Go to railway.app
   - Create new project
   - Connect GitHub repository

2. **Set environment variables in Railway:**
   ```
   TELEGRAM_BOT_TOKEN=your_bot_token
   ADMIN_TELEGRAM_ID=your_admin_id
   DATABASE_URL=postgresql://...
   NODE_ENV=production
   ```

3. **Deploy:**
   - Railway will automatically deploy from the backend/ folder
   - It will install dependencies from package.json
   - Start server with `node server.js`

## Cheat Client (C++)

### Files to build locally:
```
src/
CMakeLists.txt
main.cpp
config.ini
```

### Build steps:
```bash
mkdir build
cd build
cmake ..
cmake --build .
```

### Required libraries:
- Visual Studio Build Tools
- CURL
- nlohmann/json
