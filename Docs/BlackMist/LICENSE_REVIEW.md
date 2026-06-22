# License Review

Review date: 2026-06-22

This is an engineering license review, not legal advice.

## Summary

- MIT is suitable for the original plugin source, shader source, and documentation in this repository.
- No Unreal Engine source files, Engine binaries, Starter Content source files, or third-party library sources are included in the repository.
- The plugin requires Unreal Engine to build and run. Unreal Engine remains licensed separately by Epic Games and is not sublicensed by this repository.
- The shader file includes `/Engine/Private/Common.ush` and `/Engine/Private/ScreenPass.ush` by virtual path. Those Engine files are compile-time dependencies from the user's installed Engine and are not copied into this repository.
- Generated output such as `Binaries`, `Intermediate`, `Saved`, packaged plugin archives, and Engine-derived binaries must stay out of Git. `.gitignore` is configured for that.

## Screenshots

The comparison images under `Docs/BlackMist/Images/` are rendered screenshots. Epic's current Unreal Engine EULA treats rendered video files or images as Non-Engine Products when they do not include Engine Code, do not require Engine Code to run, and do not include Starter Content in source format.

Separate asset licenses may still apply to any scene content shown in the screenshots. Keep these images public only if the rendered scene assets are owned by the repository owner or are otherwise redistributable for this use.

## Public GitHub Suitability

The repository is suitable for a public MIT GitHub release under these assumptions:

- The repository owner accepts the MIT release of this plugin code and documentation.
- The comparison screenshots were captured by the repository owner from content they are allowed to publish.
- Future commits do not add Unreal Engine source, generated Engine binaries, Marketplace/Fab assets, Starter Content source files, or code under incompatible licenses.
