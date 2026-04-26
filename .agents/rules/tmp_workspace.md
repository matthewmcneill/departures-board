# Temporary Workspace Files

When you need to create temporary files, test scripts, or intermediate output during your work, you MUST create and use a `tmp/` directory at the root of the current repository. 

1. Ensure the `tmp/` directory exists in the current repo.
2. Ensure `tmp/` is added to the repository's `.gitignore` file to prevent accidental commits of temporary data.
3. Place all temporary working files and tools inside this `tmp/` directory rather than leaving them in the repository root or scattering them across the project.
