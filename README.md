# Encouragement Pal

A Geode mod for Geometry Dash. A little pal pops up in the corner with sarcastic "encouragement" when you hit milestones, set a new best, or beat a level.

## Building

### Easiest: GitHub Actions (no local setup)
1. Make a new GitHub repo and upload everything in this folder (including `.github/`).
2. Go to the **Actions** tab. The "Build Geode Mod" workflow runs automatically on push.
3. When it finishes, download **Build Output**. It contains `kaiden.encouragement-pal.geode`.

### Locally
1. Install the [Geode SDK and CLI](https://docs.geode-sdk.org/getting-started/) (sets `GEODE_SDK`).
2. In this folder, run `geode build`.

## Installing
Copy the `.geode` file into your Geode mods folder (Geode menu → "Open Mods Folder"), then restart Geometry Dash.

## Customizing
- **Lines**: edit the lists at the top of `src/main.cpp`.
- **Pal's look**: replace `resources/pal.png` (square PNG, ~128×128).
- **Milestones**: change `MILESTONES` in `src/main.cpp`.
