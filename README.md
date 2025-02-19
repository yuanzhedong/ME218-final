# PIC0

Planner

Crate related control


# PIC1

Navigator

Wheel Motor related control

Crate detection sensor


# Git Workflow: Checkout a Branch, Make a PR, and Resolve Conflicts

## 1. Checkout a New Branch from `main`
```sh
git checkout main         # Switch to main branch
git pull origin main      # Get the latest changes
git checkout -b feature-branch  # Create and switch to a new branch
```

## 2. Make Changes and Commit
```sh
# Edit files...

git add .                 # Stage changes
git commit -m "Add new feature"  # Commit changes
```

## 3. Push the Branch to GitHub
```sh
git push -u origin feature-branch
```

## 4. Create a Pull Request (PR)
1. Go to **GitHub → Repository → Pull Requests**.
2. Click **New Pull Request** and select `feature-branch` to merge into `main`.
3. Submit the PR.

## 5. Resolve Merge Conflicts (If Any)
If there are conflicts, merge `main` into your feature branch:
```sh
git checkout main
git pull origin main      # Ensure main is up to date
git checkout feature-branch
git merge main            # Merge main into the feature branch
```
Manually fix conflicts in the marked files:
```
<<<<<<< HEAD
# Code from main branch
=======
# Code from feature-branch
>>>>>>> feature-branch
```
After resolving conflicts:
```sh
git add .
git commit -m "Resolve merge conflicts"
git push origin feature-branch
```

## 6. Merge PR into `main`
1. Once reviewed and conflicts are resolved, **merge the PR** on GitHub.
2. Finally, update your local `main`:
```sh
git checkout main
git pull origin main
```

🚀 **Done!**
