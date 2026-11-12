# Crimson Desert Animation Player

A Crimson Desert animation system in development by Raebeees. It provides an in-game library for selecting custom animations, including dances with music. This is a source snapshot for review, not an installable release.

## Source

Download and extract [animation-player-source.zip](animation-player-source.zip). It contains the dance client, shared host source and headers, tests, reference build scripts, MinHook with its license, and a source hash manifest. Game assets, audio, compiled plugins and private logs are not included.

## How it works and why

Open **F4 → Animations** to select an animation. The client manages the menu, playback and audio; a shared runtime host checks actor identity and readiness and connects the selection to a prepared game animation route. Movement cancels playback.

The client and host communicate through a versioned interface so their responsibilities remain separate. Request tokens and lifecycle checks bound playback to the correct actor and request. Private animation routes support on-demand selection while avoiding global replacement of normal idle behavior. The host includes Tidebreaker integration because the features share runtime infrastructure. Build and asset checks guard version-specific integration.

## Animation workflow

Source animations are imported into Blender and retargeted to the Crimson Desert character rig. Bone movement, orientation, hands, feet and deformation are reviewed before conversion. The original body mesh and skin weights are preserved during transfers.

## Tests and build limits

The eight-entry lifecycle simulation passed on this snapshot. The handoff test compiled but could not load the omitted DanceModule.asi; it was not a successful standalone test. Other included tests were not rerun for this publication. Offline tests are not gameplay acceptance.

The historical Windows x64 MSVC scripts use sanitized placeholder paths and require external adapter files and MinHook object files. They have not been verified as a reproducible public build. Runtime also requires matching game and mod versions and separately prepared animation and audio assets. Do not remove version or readiness checks to force loading.

Several dances are installed for testing; deformation, transitions, audio timing and flight compatibility still need in-game review. Facial animation remains in development.

## Licensing

The original project code license is not specified in this snapshot. MinHook retains its license in the archive. Third-party game assets and mods remain subject to their respective terms.
