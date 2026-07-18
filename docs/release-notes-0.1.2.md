# MetaDrop 0.1.2

MetaDrop 0.1.2 refreshes the application identity and makes installation, source builds and removal straightforward in English, Russian, Ukrainian and German. Metadata engines and the inspect → sanitize → verify boundary are unchanged from 0.1.1.

## Included

| Asset | Purpose |
|---|---|
| `MetaDrop-0.1.2-x86_64.AppImage` | Portable x86-64 Linux build |
| `metadrop_0.1.2_amd64.deb` | Debian / Ubuntu package |
| `metadrop-0.1.2-1.x86_64.rpm` | Fedora / RHEL package |
| `PKGBUILD` | Arch-family package recipe |
| `MetaDrop-0.1.2-source.tar.gz` | Release source |
| `SHA256SUMS` | Integrity manifest for every asset |

## Changes

- New high-contrast application icon and compact repository header.
- English default README plus complete RU, UK and DE documentation.
- Exact AppImage, DEB, RPM, Arch, source-build and uninstall commands.
- RPM and generated Arch packaging added to the automated release.
- Build-provenance attestations for published artifacts.

## Privacy boundary

- Source files are never overwritten; MetaDrop creates a separate copy and verifies it before saving.
- Unsupported formats are never reported as clean.
- Visible content, filenames, filesystem records, comments, tracked changes, hidden sheets/slides and macros remain outside the verified metadata scope.
- Processing stays local; the application contains no telemetry or file-upload client.

Verify downloads with `sha256sum --ignore-missing --check SHA256SUMS`.
