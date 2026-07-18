# MetaDrop 0.1.3

MetaDrop 0.1.3 aligns the repository and release layout with Vacuo. The README now uses only the centered application icon, documentation follows the same EN/RU/UK/DE structure, and native package artifacts use one consistent release set. Metadata engines and verification behavior are unchanged.

## Included

| Asset | Purpose |
|---|---|
| `metadrop_0.1.3_amd64.deb` | Debian / Ubuntu package |
| `metadrop-0.1.3-1.x86_64.rpm` | Fedora / RHEL package |
| `PKGBUILD` | Arch-family package recipe |
| `MetaDrop-0.1.3-x86_64.tar.gz` | Installable Linux tree |
| `MetaDrop-0.1.3-source.tar.gz` | Release source |
| `MetaDrop-0.1.3.spdx` | SPDX 2.3 software bill of materials |
| `SHA256SUMS` | Integrity manifest for every asset |

## Changes

- Removed the wide README banner; the application icon is now centered without a surrounding hero background.
- Standardized project documentation and removed the unused code-of-conduct file.
- Standardized release assets with Vacuo and removed AppImage from new releases.
- Added a dedicated privacy document, security model, support page and CodeQL workflow.

## Privacy boundary

- Source files are never overwritten; MetaDrop creates and verifies a separate copy.
- Unsupported formats are never reported as clean.
- Visible content, filenames, filesystem records, comments, tracked changes, hidden sheets/slides and macros remain outside the verified metadata scope.
- Processing stays local; the application contains no telemetry or upload client.

Verify downloads with `sha256sum --ignore-missing --check SHA256SUMS`.
