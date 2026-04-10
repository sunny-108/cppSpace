# Git and GitHub Setup Guide

## Initial Setup Steps

### 1. Initialize Git Repository
```bash
cd /Users/sunny/workspace/cppSpace
git init
```

### 2. Create a .gitignore file
```bash
cat > .gitignore << 'EOF'
# Compiled files
*.o
*.out
a.out
*.exe

# Build directories
build/
bin/

# IDE files
.vscode/
.idea/
*.swp

# macOS
.DS_Store
EOF
```

### 3. Add and Commit Files
```bash
git add .
git commit -m "Initial commit: C++ learning workspace"
```

### 4. Create a GitHub Repository
- Go to https://github.com/new
- Create a new repository (e.g., "cppSpace" or "cpp-learning")
- **Don't** initialize with README, .gitignore, or license

### 5. Link and Push to GitHub
```bash
git remote add origin https://github.com/YOUR_USERNAME/YOUR_REPO_NAME.git
git branch -M main
git push -u origin main
```

Replace `YOUR_USERNAME` and `YOUR_REPO_NAME` with your actual GitHub username and repository name.

## Common Git Commands

### Check Repository Status
```bash
git status
```

### Add Changes
```bash
# Add specific file
git add filename.cpp

# Add all changes
git add .

# Add all changes in a directory
git add directory/
```

### Commit Changes
```bash
git commit -m "Your commit message"
```

### Push Changes
```bash
# Push to main branch
git push origin main

# Push all branches
git push --all origin
```

### Pull Changes
```bash
git pull origin main
```

### View Commit History
```bash
git log

# Compact view
git log --oneline

# Graph view
git log --graph --oneline --all
```

### Create and Switch Branches
```bash
# Create new branch
git branch branch-name

# Switch to branch
git checkout branch-name

# Create and switch in one command
git checkout -b branch-name

# Modern way (Git 2.23+)
git switch -c branch-name
```

### View Remote Information
```bash
git remote -v
```

## Tips

1. **Commit Often**: Make small, focused commits with clear messages
2. **Pull Before Push**: Always pull latest changes before pushing
3. **Use Branches**: Create feature branches for new work
4. **Write Good Commit Messages**: Be descriptive about what changed and why

## Repository Information
- Repository: https://github.com/sunny-108/cppSpace
- Owner: sunny-108
- Branch: main
