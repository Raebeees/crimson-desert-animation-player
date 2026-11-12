# Crimson Desert Animation Player

A Crimson Desert animation system in development by Raebeees. It provides an in-game library for selecting custom animations, including dances with music. This repository documents the project and its design; no download is currently provided.

## Description

Open **F4 → Animations** to select an animation. The client manages the menu, playback and audio; a shared runtime host checks actor identity and readiness and connects the selection to a prepared game animation route. Movement cancels playback.

The client and host communicate through a versioned interface so their responsibilities remain separate. Request tokens and lifecycle checks bound playback to the correct actor and request. Private animation routes support on-demand selection while avoiding global replacement of normal idle behavior. The host includes Tidebreaker integration because the features share runtime infrastructure. Build and asset checks guard version-specific integration.

## Animation workflow

Source animations are imported into Blender and retargeted to the Crimson Desert character rig. Bone movement, orientation, hands, feet and deformation are reviewed before conversion. The original body mesh and skin weights are preserved during transfers.

## Testing and status

An eight-entry lifecycle simulation passed. Offline tests are not gameplay acceptance. Several dances are installed for testing; deformation, transitions, audio timing and flight compatibility still need in-game review. Facial animation remains in development.

The current implementation depends on build-specific components and separately prepared animation and audio assets; it is not a general installable release.
