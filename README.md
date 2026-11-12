# Crimson Desert Animation Player

A Crimson Desert animation system in development by Raebeees. It provides an in-game library for selecting and playing custom animations, including dances with music. This repository is a project overview; it does not contain code, game assets, or an installable release.

## How it works

Open **F4 → Animations** and select an animation from the menu. The runtime connects the selection to a prepared game animation route and starts the associated audio when available. Moving cancels playback.

The system is designed to play selected animations on demand rather than waiting for the game to trigger a particular idle.

## Animation workflow

Source animations are imported into Blender and retargeted to the Crimson Desert character rig. Bone movement, orientation, hands, feet, and deformation are reviewed before conversion for game playback. The character’s original body mesh and skin weights are preserved during the transfers.

The menu and runtime were developed alongside the existing Tidebreaker Flight and Cyberware systems, with checks intended to keep animation playback separate from flight behavior.

## Current status

The animation menu and several dances are installed for testing. Menu, asset, and animation-route checks have passed, while individual dances still need in-game review for deformation, transitions, audio timing, and flight compatibility. Additional animations and facial animation support are in development.
