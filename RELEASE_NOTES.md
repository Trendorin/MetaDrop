# MetaDrop 0.1.5

MetaDrop 0.1.5 adds a directly installable Arch Linux package and strengthens validation of every release format. Application behavior and the metadata-sanitization boundary are unchanged.

## Included

| Asset | Target |
|---|---|
| `metadrop_0.1.5_amd64.deb` | Ubuntu 24.04 |
| `metadrop-0.1.5-1.x86_64.rpm` | Fedora 44 |
| `metadrop-0.1.5-1-x86_64.pkg.tar.zst` | Arch Linux |
| `PKGBUILD` | Arch source rebuild |
| `MetaDrop-0.1.5-x86_64.tar.gz` | Installable Linux tree |
| `MetaDrop-0.1.5-source.tar.gz` | Release source |
| `MetaDrop-0.1.5.spdx` | SPDX 2.3 SBOM |
| `SHA256SUMS` | Integrity manifest |

## Validation

- DEB: built and installed on Ubuntu 24.04; package metadata and all dynamic links checked.
- RPM: built and installed on Fedora 44; Exiv2/qpdf SONAMEs and all dynamic links checked.
- Arch: clean-built as an unprivileged user, installed with `pacman -U`, and verified with `pacman -Qkk`.
- TGZ, source, `PKGBUILD`, SPDX data, checksums and build provenance are checked before publication.

Install on Arch Linux:

```bash
sudo pacman -U ./metadrop-0.1.5-1-x86_64.pkg.tar.zst
```

Verify downloads with `sha256sum --ignore-missing --check SHA256SUMS`.
